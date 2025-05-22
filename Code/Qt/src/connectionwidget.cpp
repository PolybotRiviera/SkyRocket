#include "connectionwidget.hpp"
#include <QVBoxLayout>
#include <QApplication>
#include <QDebug>

ConnectionWidget::ConnectionWidget(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    scanButton = new QPushButton("Scan for Devices", this);
    deviceList = new QListWidget(this);
    statusLabel = new QLabel("Ready", this);
    layout->addWidget(scanButton);
    layout->addWidget(deviceList);
    layout->addWidget(statusLabel);

    discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    controller = nullptr;
    service = nullptr;
    serviceUuid = QBluetoothUuid("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
    characteristicUuid = QBluetoothUuid("beb5483e-36e1-4688-b7f5-ea07361b26a8");

    connect(scanButton, &QPushButton::clicked, this, &ConnectionWidget::startScan);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &ConnectionWidget::deviceDiscovered);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &ConnectionWidget::scanFinished);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred, this, &ConnectionWidget::scanError);
    connect(deviceList, &QListWidget::itemDoubleClicked, this, &ConnectionWidget::connectToDevice);
}

ConnectionWidget::~ConnectionWidget()
{
    qDebug() << "ConnectionWidget destructor called";

    stopOperations();

    if (controller) {
        qDebug() << "Disconnecting and deleting QLowEnergyController";
        controller->disconnectFromDevice();
        QCoreApplication::processEvents();
        delete controller;
        controller = nullptr;
    }
    if (service) {
        qDebug() << "Deleting QLowEnergyService";
        delete service;
        service = nullptr;
    }
    if (discoveryAgent) {
        qDebug() << "Deleting QBluetoothDeviceDiscoveryAgent";
        delete discoveryAgent;
        discoveryAgent = nullptr;
    }

    qDebug() << "ConnectionWidget destruction complete";
}

void ConnectionWidget::stopOperations()
{
    qDebug() << "Stopping ongoing operations in ConnectionWidget";

    if (discoveryAgent && discoveryAgent->isActive()) {
        qDebug() << "Stopping device discovery";
        discoveryAgent->stop();
    }

    if (controller && controller->state() != QLowEnergyController::UnconnectedState) {
        qDebug() << "Disconnecting from device";
        controller->disconnectFromDevice();
        QCoreApplication::processEvents();
    }
}

void ConnectionWidget::startScan()
{
    if (connecting){
        return;
    }
    scanning = true;
    deviceList->clear();
    scanButton->setEnabled(false);
    statusLabel->setText("Scanning for devices...");

    discoveryAgent->setLowEnergyDiscoveryTimeout(1000);
    //qDebug() << "Scanning timeout set to" << discoveryAgent->lowEnergyDiscoveryTimeout() << "ms";

    discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

void ConnectionWidget::deviceDiscovered(const QBluetoothDeviceInfo &device)
{   if(device.name().contains("SkyRocket")){
        QString label = QString("%1 (%2) RSSI: %3")
        .arg(device.name())
            .arg(device.address().toString())
            .arg(device.rssi());
        deviceList->addItem(label);
    }
}

void ConnectionWidget::scanFinished()
{
    scanning = false;
    scanButton->setEnabled(true);
    if (deviceList->count() == 0) {
        deviceList->clear();
        deviceList->addItem("No devices found");
        statusLabel->setText("No devices found");
    } else {
        statusLabel->setText("Scan complete. Double-click a device to connect.");
    }
}

void ConnectionWidget::scanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    QString errorMsg;
    switch (error) {
    case QBluetoothDeviceDiscoveryAgent::NoError:
        return;
    case QBluetoothDeviceDiscoveryAgent::PoweredOffError:
        errorMsg = "Bluetooth is powered off";
        break;
    case QBluetoothDeviceDiscoveryAgent::InputOutputError:
        errorMsg = "I/O Error occurred";
        break;
    default:
        errorMsg = "Unknown error: " + QString::number(error);
    }
    QMessageBox::warning(this, "Scan Error", errorMsg);
    scanButton->setEnabled(true);
    deviceList->clear();
    deviceList->addItem("Scan failed: " + errorMsg);
    statusLabel->setText("Scan failed: " + errorMsg);
}

void ConnectionWidget::connectToDevice(QListWidgetItem *item)
{
    if (scanning || connecting){
        return;
    }

    if (controller) {
        controller->disconnectFromDevice();
        delete controller;
        controller = nullptr;
    }

    QString itemText = item->text();
    QString address = itemText.mid(itemText.indexOf("(") + 1, 17);

    QList<QBluetoothDeviceInfo> devices = discoveryAgent->discoveredDevices();
    for (const QBluetoothDeviceInfo &device : devices) {
        if (device.address().toString().contains(address)) {
            currentDevice = device;
            break;
        }
    }

    controller = QLowEnergyController::createCentral(currentDevice, this);
    connect(controller, &QLowEnergyController::connected, this, &ConnectionWidget::deviceConnected);
    connect(controller, &QLowEnergyController::disconnected, this, &ConnectionWidget::deviceDisconnected);
    connect(controller, &QLowEnergyController::errorOccurred, this, &ConnectionWidget::errorOccurred);
    connect(controller, &QLowEnergyController::serviceDiscovered, this, &ConnectionWidget::serviceDiscovered);
    connect(controller, &QLowEnergyController::discoveryFinished, this, &ConnectionWidget::serviceScanDone);

    statusLabel->setText("Connecting to " + currentDevice.name() + "...");
    controller->connectToDevice();
}

void ConnectionWidget::deviceConnected()
{
    connecting = false;
    statusLabel->setText("Connected to " + currentDevice.name());
    //QMessageBox::information(this, "Connected", "Successfully connected to " + currentDevice.name());
    controller->discoverServices();
}

void ConnectionWidget::serviceDiscovered(const QBluetoothUuid &gatt)
{
    if (gatt == serviceUuid) {
        service = controller->createServiceObject(serviceUuid, this);
        if (service) {
            service->discoverDetails();
        }
    }
}

void ConnectionWidget::serviceScanDone()
{
    if (!service) {
        statusLabel->setText("Service not found");
        QMessageBox::warning(this, "Error", "BLE service not found! Check UUIDs or ESP32 advertising.");
        resetToScanUI();
        return;
    }
    emit connected(service);
}

void ConnectionWidget::deviceDisconnected()
{
    handleDisconnection();
}

void ConnectionWidget::errorOccurred(QLowEnergyController::Error error)
{
    if (error == QLowEnergyController::RemoteHostClosedError) {
        handleDisconnection();
    } else {
        statusLabel->setText("Connection failed: " + QString::number(error));
        qDebug() << "bello" << error;
        qDebug() << "bello";

        QMessageBox::critical(this, "Error", "Connection error: " + QString::number(error));
        if (service) {
            delete service;
            service = nullptr;
        }
        resetToScanUI();
        startScan();
    }
}

void ConnectionWidget::handleDisconnection()
{
    statusLabel->setText("Disconnected");
    //QMessageBox::warning(this, "Disconnected", "Device disconnected");
    if (service) {
        delete service;
        service = nullptr;
    }
    emit disconnected();
    resetToScanUI();
    startScan();
}

void ConnectionWidget::resetToScanUI()
{
    deviceList->clear();
    deviceList->addItem("Double-click a device to connect");
    statusLabel->setText("Ready");
    scanButton->setEnabled(true);
    QApplication::processEvents();
    update();
}
