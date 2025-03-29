#include "robotcontrolwidget.hpp"
#include <QKeyEvent>

RobotControlWidget::RobotControlWidget(QLowEnergyService *service, QWidget *parent)
    : QWidget(parent), service(service), speed(50),
    isEmergencyStopped(false)
{
    characteristicUuid = QBluetoothUuid("beb5483e-36e1-4688-b7f5-ea07361b26a8");

    // Main layout for the widget
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);

    // Title
    QLabel *title = new QLabel("Control with ZQSD", this);
    title->setAlignment(Qt::AlignCenter);
    title->setMaximumWidth(120);
    mainLayout->addStretch();
    mainLayout->addWidget(title);
    mainLayout->addStretch();

    // Speed slider
    speedSlider = new QSlider(Qt::Horizontal, this);
    speedSlider->setRange(0, 100);
    speedSlider->setValue(50); // Initial speed
    speedSlider->setMaximumWidth(100);
    speedValueLabel = new QLabel(QString("%1").arg(speed), this);
    speedValueLabel->setAlignment(Qt::AlignCenter);
    speedValueLabel->setMaximumWidth(20);

    // Connect the slider's valueChanged signal to update the speed and label
    connect(speedSlider, &QSlider::valueChanged, this, [this](int value) {
        speed = value;
        speedValueLabel->setText(QString("%1").arg(speed));
    });

    QLabel *speedLabel = new QLabel("Speed", this);
    speedLabel->setMaximumWidth(120);
    speedLabel->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(speedLabel);

    QHBoxLayout *speedLayout = new QHBoxLayout;
    speedLayout->addWidget(speedSlider);
    speedLayout->addWidget(speedValueLabel);
    mainLayout->addLayout(speedLayout);
    mainLayout->addStretch();


    // Buttons
    emergencyStopButton = new QPushButton("Emergency Stop", this);
    emergencyStopButton->setStyleSheet("background-color: red; color: white; font-weight: bold;");
    emergencyStopButton->setMaximumWidth(120);
    activateButton = new QPushButton("Activate", this);
    activateButton->setStyleSheet("background-color: green; color: white; font-weight: bold;");
    activateButton->setMaximumWidth(120);
    activateButton->setVisible(false);

    // Center each button horizontally
    mainLayout->addWidget(emergencyStopButton);
    mainLayout->addWidget(activateButton);

    mainLayout->addStretch();

    // Connect signals
    connect(emergencyStopButton, &QPushButton::clicked, this, &RobotControlWidget::emergencyStop);
    connect(activateButton, &QPushButton::clicked, this, &RobotControlWidget::activateRobot);

    connect(speedSlider, &QSlider::sliderReleased, this, &RobotControlWidget::sendSpeed);

    // Ensure the widget can receive key events
    setFocusPolicy(Qt::StrongFocus);
}

void RobotControlWidget::setService(QLowEnergyService *newService)
{
    service = newService;
}

void RobotControlWidget::keyPressEvent(QKeyEvent *event)
{
    if (!service || isEmergencyStopped || !isEnabled()) {
        event->ignore();
        return;
    }

    if (event->isAutoRepeat()) {
        event->ignore();
        return;
    }

    switch (event->key()) {
    case Qt::Key_Z: // Forward
        forward = true;
        break;
    case Qt::Key_S: // Backward
        backward = true;
        break;
    case Qt::Key_Q: // Left
        left = true;
        break;
    case Qt::Key_D: // Right
        right = true;
        break;
    case Qt::Key_K:
        emergencyStop();
        break;
    default:
        event->ignore();
        return;
    }
    event->accept();
    processCommand();
}

void RobotControlWidget::keyReleaseEvent(QKeyEvent *event)
{
    if (!service || isEmergencyStopped || !isEnabled()) {
        event->ignore();
        return;
    }

    if (event->isAutoRepeat()) {
        event->ignore();
        return;
    }

    switch (event->key()) {
    case Qt::Key_Z: // Forward
        forward = false;
        break;
    case Qt::Key_S: // Backward
        backward = false;
        break;
    case Qt::Key_Q: // Left
        left = false;
        break;
    case Qt::Key_D: // Right
        right = false;
        break;
    case Qt::Key_K:
        emergencyStop();
        break;
    default:
        event->ignore();
        return;
    }
    event->accept();
    processCommand();
}

void RobotControlWidget::sendSpeed(){
    sendCommand(QString("Speed %1").arg(speed));
}

void RobotControlWidget::processCommand(){

    if(!forward && !backward && !left && !right){
        sendCommand("stop");
        return;
    }

    if(forward){
        if(left){sendCommand("Moving 315");}
        else if(right){sendCommand("Moving 45");}
        else{sendCommand("Moving 0");}
    }

    else if(backward){
        if(left){sendCommand("Moving 225");}
        else if(right){sendCommand("Moving 135");}
        else{sendCommand("Moving 180");}
    }

    else if(right){sendCommand("Moving 90");}

    else if(left){sendCommand("Moving 270");}
}

void RobotControlWidget::sendCommand(const QString &command)
{
    if (!service || isEmergencyStopped) return;

    qDebug() << command;

    QLowEnergyCharacteristic characteristic = service->characteristic(characteristicUuid);
    if (characteristic.isValid()) {
        service->writeCharacteristic(characteristic, command.toUtf8());
    }
}

void RobotControlWidget::emergencyStop()
{
    if (!service) return;

    QLowEnergyCharacteristic characteristic = service->characteristic(characteristicUuid);
    if (characteristic.isValid()) {
        service->writeCharacteristic(characteristic, QByteArray("emergency_stop"));
        isEmergencyStopped = true;
        emergencyStopButton->setVisible(false);
        activateButton->setVisible(true);
    }
}

void RobotControlWidget::activateRobot()
{
    if (!service) return;

    QLowEnergyCharacteristic characteristic = service->characteristic(characteristicUuid);
    if (characteristic.isValid()) {
        service->writeCharacteristic(characteristic, QByteArray("activate"));
        isEmergencyStopped = false;
        emergencyStopButton->setVisible(true);
        activateButton->setVisible(false);
    }
}
