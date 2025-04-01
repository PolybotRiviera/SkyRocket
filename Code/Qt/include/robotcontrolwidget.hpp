#ifndef ROBOTCONTROLWIDGET_HPP
#define ROBOTCONTROLWIDGET_HPP

#include <QWidget>
#include <QLowEnergyService>
#include <QSlider>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

class RobotControlWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RobotControlWidget(QLowEnergyService *service, QWidget *parent = nullptr);
    void setService(QLowEnergyService *service);
protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
private slots:
    void sendCommand(const QByteArray &command);
    void processCommand();
    void emergencyStop();
    void activateRobot();
    void sendSpeed();
private:
    QLowEnergyService *service;
    QSlider *speedSlider;
    QLabel *speedLabel;
    QLabel *speedValueLabel;
    QPushButton *emergencyStopButton;
    QPushButton *activateButton;
    QBluetoothUuid characteristicUuid;
    int speed;
    bool isEmergencyStopped;
    bool forward = false, backward = false, left = false, right = false, turnLeft = false, turnRight = false;



};

#endif
