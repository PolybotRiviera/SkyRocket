#ifndef CONNECTIONWIDGET_HPP
#define CONNECTIONWIDGET_HPP

#include <QWidget>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QLowEnergyController>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>

class ConnectionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ConnectionWidget(QWidget *parent = nullptr);
    ~ConnectionWidget();
signals:
    void connected(QLowEnergyService *service);
    void disconnected();
private slots:
    void startScan();
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
    void scanFinished();
    void scanError(QBluetoothDeviceDiscoveryAgent::Error error);
    void connectToDevice(QListWidgetItem *item);
    void deviceConnected();
    void serviceDiscovered(const QBluetoothUuid &gatt);
    void serviceScanDone();
    void deviceDisconnected();
    void errorOccurred(QLowEnergyController::Error error);
private:
    void handleDisconnection();
    void resetToScanUI();
    void stopOperations();
    bool scanning = false;
    bool connecting = false;
    QPushButton *scanButton;
    QListWidget *deviceList;
    QLabel *statusLabel;
    QBluetoothDeviceDiscoveryAgent *discoveryAgent;
    QLowEnergyController *controller;
    QBluetoothDeviceInfo currentDevice;
    QLowEnergyService *service;
    QBluetoothUuid serviceUuid;
    QBluetoothUuid characteristicUuid;
};

#endif
