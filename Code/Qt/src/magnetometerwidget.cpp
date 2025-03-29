#include "MagnetometerWidget.hpp"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QDebug>

MagnetometerWidget::MagnetometerWidget(QLowEnergyService *service, QWidget *parent)
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
    mainLayout->addStretch();

    connect(applyButton, &QPushButton::clicked, this, &MagnetometerWidget::onApplyPIDClicked);
    connect(calibrateButton, &QPushButton::clicked, this, &MagnetometerWidget::onCalibrateClicked);

    connect(headingSlider, &QSlider::valueChanged, this, &MagnetometerWidget::updateHeadingLabel);
    connect(headingSlider, &QSlider::sliderReleased, this, &MagnetometerWidget::sendHeadingValue);
}

void MagnetometerWidget::setService(QLowEnergyService *newService)
{
    service = newService;
}

void MagnetometerWidget::onApplyPIDClicked()
{
    if (!service) {
        qDebug() << "MagnetometerWidget: No BLE service available";
        return;
    }

    double Kp = kpSpinBox->value();
    double Ki = kiSpinBox->value();
    double Kd = kdSpinBox->value();

    QString pidCommand = "magPID " + QString("%1 %2 %3").arg(Kp).arg(Ki).arg(Kd);

    QLowEnergyCharacteristic characteristic = service->characteristic(characteristicUuid);

    if (!characteristic.isValid()) {
        qDebug() << "MagnetometerWidget: Invalid PID characteristic";
        return;
    }

    service->writeCharacteristic(characteristic, pidCommand.toUtf8());
}

void MagnetometerWidget::onCalibrateClicked(){
    if (!service) {
        qDebug() << "MagnetometerWidget: No BLE service available";
        return;
    }

    QLowEnergyCharacteristic characteristic = service->characteristic(characteristicUuid);

    if (!characteristic.isValid()) {
        return;
    }

    service->writeCharacteristic(characteristic, QString("magCalibration").toUtf8());
}

void MagnetometerWidget::updateHeadingLabel(int value)
{
    headingValueLabel->setText(QString::number(value));
}

void MagnetometerWidget::sendHeadingValue()
{
    int value = headingSlider->value();

    if (!service) {
        qDebug() << "MagnetometerWidget: No BLE service available";
        return;
    }

    QString headingCommand = "Heading " + QString::number(value);

    QLowEnergyCharacteristic characteristic = service->characteristic(characteristicUuid);
    if (characteristic.isValid()) {
        service->writeCharacteristic(characteristic, headingCommand.toUtf8());
    } else {
        qDebug() << "MagnetometerWidget: Invalid heading characteristic";
    }
}
