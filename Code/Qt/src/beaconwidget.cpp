#include "BeaconWidget.hpp"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QDebug>

BeaconWidget::BeaconWidget(QLowEnergyService *service, QWidget *parent)
    : QWidget(parent), service(service)
{
    characteristicUuid = QBluetoothUuid("beb5483e-36e1-4688-b7f5-ea07361b26a8");

    kpSpinBox = new QDoubleSpinBox(this);
    kpSpinBox->setRange(0.0, 100.0);
    kpSpinBox->setSingleStep(0.1);
    kpSpinBox->setDecimals(3);
    kpSpinBox->setValue(1.0);
    kpSpinBox->setMaximumWidth(120);

    kiSpinBox = new QDoubleSpinBox(this);
    kiSpinBox->setRange(0.0, 100.0);
    kiSpinBox->setSingleStep(0.1);
    kiSpinBox->setDecimals(3);
    kiSpinBox->setValue(0.0);
    kiSpinBox->setMaximumWidth(120);

    kdSpinBox = new QDoubleSpinBox(this);
    kdSpinBox->setRange(0.0, 100.0);
    kdSpinBox->setSingleStep(0.1);
    kdSpinBox->setDecimals(3);
    kdSpinBox->setValue(0.0);
    kdSpinBox->setMaximumWidth(120);

    applyButton = new QPushButton("Apply PID", this);
    applyButton->setMaximumWidth(120);

    calibrateButton = new QPushButton("Calibrate", this);
    calibrateButton->setMaximumWidth(120);

    yawButton = new QPushButton("Toggle Heading", this);
    yawButton->setMaximumWidth(120);

    headingSlider = new QSlider(Qt::Horizontal, this);
    headingSlider->setRange(0, 360);
    headingSlider->setValue(0);
    headingSlider->setMaximumWidth(100);

    headingValueLabel = new QLabel("0", this);
    headingValueLabel->setMaximumWidth(20);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addStretch();

    QLabel *headingLabel = new QLabel("Heading");
    headingLabel->setMaximumWidth(120);
    headingLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(headingLabel);
    QHBoxLayout *headingLayout = new QHBoxLayout;
    headingLayout->addWidget(headingSlider);
    headingLayout->addWidget(headingValueLabel);
    mainLayout->addLayout(headingLayout);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->addStretch();

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow("Kp", kpSpinBox);
    formLayout->addRow("Ki", kiSpinBox);
    formLayout->addRow("Kd", kdSpinBox);
    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();

    mainLayout->addWidget(applyButton);
    mainLayout->addWidget(calibrateButton);
    mainLayout->addWidget(yawButton);
    mainLayout->addStretch();

    connect(applyButton, &QPushButton::clicked, this, &BeaconWidget::onApplyPIDClicked);
    connect(calibrateButton, &QPushButton::clicked, this, &BeaconWidget::onCalibrateClicked);
    connect(yawButton, &QPushButton::clicked, this, &BeaconWidget::onYawClicked);

    connect(headingSlider, &QSlider::valueChanged, this, &BeaconWidget::updateHeadingLabel);
    connect(headingSlider, &QSlider::sliderReleased, this, &BeaconWidget::sendHeadingValue);
}

void BeaconWidget::setService(QLowEnergyService *newService)
{
    service = newService;
}

void BeaconWidget::onYawClicked()
{
    if (!service) {
        qDebug() << "BeaconWidget: No BLE service available";
        return;
    }

    QLowEnergyCharacteristic characteristic = service->characteristic(characteristicUuid);
    if (!characteristic.isValid()) {
        return;
    }

    QByteArray data;
    data.append(static_cast<char>(10));  // YAW_COMPENSATED_TOGGLE command ID
    service->writeCharacteristic(characteristic, data);
    qDebug() << "Sending YAW_COMPENSATED_TOGGLE:" << data.toHex();
}

void BeaconWidget::onApplyPIDClicked()
{
    if (!service) {
        qDebug() << "BeaconWidget: No BLE service available";
        return;
    }

    QLowEnergyCharacteristic characteristic = service->characteristic(characteristicUuid);
    if (!characteristic.isValid()) {
        qDebug() << "BeaconWidget: Invalid PID characteristic";
        return;
    }

    // Convert double values to int16_t (multiply by 1000 to preserve 3 decimals)
    int16_t kp = static_cast<int16_t>(kpSpinBox->value() * 1000);
    int16_t ki = static_cast<int16_t>(kiSpinBox->value() * 1000);
    int16_t kd = static_cast<int16_t>(kdSpinBox->value() * 1000);

    QByteArray data;
    data.append(static_cast<char>(3));  // MAG_PID command ID
    data.append(static_cast<char>(kp >> 8));  // kp high byte
    data.append(static_cast<char>(kp & 0xFF));  // kp low byte
    data.append(static_cast<char>(ki >> 8));  // ki high byte
    data.append(static_cast<char>(ki & 0xFF));  // ki low byte
    data.append(static_cast<char>(kd >> 8));  // kd high byte
    data.append(static_cast<char>(kd & 0xFF));  // kd low byte

    service->writeCharacteristic(characteristic, data);
    qDebug() << "Sending MAG_PID:" << data.toHex();
}

void BeaconWidget::onCalibrateClicked()
{
    if (!service) {
        qDebug() << "BeaconWidget: No BLE service available";
        return;
    }

    QLowEnergyCharacteristic characteristic = service->characteristic(characteristicUuid);
    if (!characteristic.isValid()) {
        return;
    }

    QByteArray data;
    data.append(static_cast<char>(9));  // MAG_CALIBRATION command ID
    service->writeCharacteristic(characteristic, data);
    qDebug() << "Sending MAG_CALIBRATION:" << data.toHex();
}

void BeaconWidget::updateHeadingLabel(int value)
{
    headingValueLabel->setText(QString::number(value));
}

void BeaconWidget::sendHeadingValue()
{
    if (!service) {
        qDebug() << "BeaconWidget: No BLE service available";
        return;
    }

    QLowEnergyCharacteristic characteristic = service->characteristic(characteristicUuid);
    if (!characteristic.isValid()) {
        qDebug() << "BeaconWidget: Invalid heading characteristic";
        return;
    }

    int16_t value = static_cast<int16_t>(headingSlider->value());
    QByteArray data;
    data.append(static_cast<char>(2));  // HEADING command ID
    data.append(static_cast<char>(value >> 8));  // High byte
    data.append(static_cast<char>(value & 0xFF));  // Low byte

    service->writeCharacteristic(characteristic, data);
    qDebug() << "Sending HEADING:" << data.toHex();
}
