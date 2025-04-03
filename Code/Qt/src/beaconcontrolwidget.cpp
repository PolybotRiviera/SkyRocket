#include "BeaconControlWidget.hpp"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QDebug>

BeaconControlWidget::BeaconControlWidget(QLowEnergyService *service, QWidget *parent)
    : QWidget(parent), service(service)
{
    characteristicUuid = QBluetoothUuid("beb5483e-36e1-4688-b7f5-ea07361b26a8");

    kpSpinBox = new QDoubleSpinBox(this);
    kpSpinBox->setRange(0.0, 100.0);
    kpSpinBox->setSingleStep(0.1);
    kpSpinBox->setDecimals(3);
    kpSpinBox->setValue(1.0);
    kpSpinBox->setMaximumWidth(100);
    kpSpinBox->setAlignment(Qt::AlignCenter);

    kiSpinBox = new QDoubleSpinBox(this);
    kiSpinBox->setRange(0.0, 100.0);
    kiSpinBox->setSingleStep(0.1);
    kiSpinBox->setDecimals(3);
    kiSpinBox->setValue(0.0);
    kiSpinBox->setMaximumWidth(100);
    kiSpinBox->setAlignment(Qt::AlignCenter);

    kdSpinBox = new QDoubleSpinBox(this);
    kdSpinBox->setRange(0.0, 100.0);
    kdSpinBox->setSingleStep(0.1);
    kdSpinBox->setDecimals(3);
    kdSpinBox->setValue(0.0);
    kdSpinBox->setMaximumWidth(100);
    kdSpinBox->setAlignment(Qt::AlignCenter);

    targetInput = new QLineEdit(this);
    targetInput->setPlaceholderText("x,y (e.g., 1000,1000)");
    targetInput->setMaximumWidth(120);
    targetInput->setAlignment(Qt::AlignCenter);

    applyButton = new QPushButton("Apply PID", this);
    applyButton->setMaximumWidth(120);

    calibrateButton = new QPushButton("Calibrate", this);
    calibrateButton->setMaximumWidth(120);

    goToButton = new QPushButton("Toggle GoTo", this);
    goToButton->setMaximumWidth(120);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addStretch();

    QLabel *headingLabel = new QLabel("Target X,Y");
    headingLabel->setMaximumWidth(120);
    headingLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(headingLabel);
    mainLayout->addWidget(targetInput);


    QFormLayout *formLayout = new QFormLayout;
    formLayout->setLabelAlignment(Qt::AlignCenter);
    formLayout->setFormAlignment(Qt::AlignCenter);
    formLayout->addRow("kp", kpSpinBox);
    formLayout->addRow("ki", kiSpinBox);
    formLayout->addRow("kd", kdSpinBox);

    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();

    // Center the buttons
    mainLayout->addWidget(applyButton);
    mainLayout->addWidget(calibrateButton);
    mainLayout->addWidget(goToButton);
    mainLayout->addStretch();

    // Set the main layout alignment
    mainLayout->setAlignment(Qt::AlignCenter);
    connect(applyButton, &QPushButton::clicked, this, &BeaconControlWidget::onApplyPIDClicked);
    connect(calibrateButton, &QPushButton::clicked, this, &BeaconControlWidget::onCalibrateClicked);
    connect(goToButton, &QPushButton::clicked, this, &BeaconControlWidget::onGoToToggleClicked);
}

void BeaconControlWidget::setService(QLowEnergyService *newService)
{
    service = newService;
}

void BeaconControlWidget::onApplyPIDClicked()
{
    if (!service) {
        qDebug() << "BeaconControlWidget: No BLE service available";
        return;
    }

    QLowEnergyCharacteristic characteristic = service->characteristic(characteristicUuid);
    if (!characteristic.isValid()) {
        qDebug() << "BeaconControlWidget: Invalid PID characteristic";
        return;
    }

    int16_t kp = static_cast<int16_t>(kpSpinBox->value() * 1000);
    int16_t ki = static_cast<int16_t>(kiSpinBox->value() * 1000);
    int16_t kd = static_cast<int16_t>(kdSpinBox->value() * 1000);

    QByteArray data;
    data.append(static_cast<char>(13));  // BEACON_PID command ID
    data.append(static_cast<char>(kp >> 8));
    data.append(static_cast<char>(kp & 0xFF));
    data.append(static_cast<char>(ki >> 8));
    data.append(static_cast<char>(ki & 0xFF));
    data.append(static_cast<char>(kd >> 8));
    data.append(static_cast<char>(kd & 0xFF));

    service->writeCharacteristic(characteristic, data);
    qDebug() << "Sending BEACON_PID:" << data.toHex();
}

void BeaconControlWidget::onCalibrateClicked()
{
    if (!service) {
        qDebug() << "BeaconControlWidget: No BLE service available";
        return;
    }

    QLowEnergyCharacteristic characteristic = service->characteristic(characteristicUuid);
    if (!characteristic.isValid()) {
        return;
    }

    QByteArray data;
    data.append(static_cast<char>(11));  // CALIBRATE_BEACON command ID
    service->writeCharacteristic(characteristic, data);
    qDebug() << "Sending CALIBRATE_BEACON:" << data.toHex();
}

void BeaconControlWidget::onGoToToggleClicked()
{
    if (!service) {
        qDebug() << "BeaconControlWidget: No BLE service available";
        return;
    }

    QLowEnergyCharacteristic characteristic = service->characteristic(characteristicUuid);
    if (!characteristic.isValid()) {
        qDebug() << "BeaconControlWidget: Invalid GoTo characteristic";
        return;
    }

    QString targetText = targetInput->text();
    QStringList coords = targetText.split(",");
    if (coords.size() != 2) {
        qDebug() << "BeaconControlWidget: Invalid target format. Use 'x,y'";
        return;
    }

    bool xOk, yOk;
    int16_t x = coords[0].toInt(&xOk);
    int16_t y = coords[1].toInt(&yOk);

    if (!xOk || !yOk) {
        qDebug() << "BeaconControlWidget: Invalid coordinate values";
        return;
    }

    QByteArray data;
    data.append(static_cast<char>(12));  // GOTO command ID
    data.append(static_cast<char>(x >> 8));
    data.append(static_cast<char>(x & 0xFF));
    data.append(static_cast<char>(y >> 8));
    data.append(static_cast<char>(y & 0xFF));

    service->writeCharacteristic(characteristic, data);
    qDebug() << "Sending GOTO:" << data.toHex();
}
