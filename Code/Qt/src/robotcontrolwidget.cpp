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

    int16_t value = 0;
    QByteArray data;
    data.append(static_cast<char>(2));  // HEADING command ID


    switch (event->key()) {
    case Qt::Key_Up:
        data.append(static_cast<char>(value >> 8));  // High byte
        data.append(static_cast<char>(value & 0xFF));  // Low byte
        sendCommand(data);
        break;

    case Qt::Key_Down:
        value = 180;
        data.append(static_cast<char>(value >> 8));  // High byte
        data.append(static_cast<char>(value & 0xFF));  // Low byte
        sendCommand(data);
        break;

    case Qt::Key_Left:
        value = 270;
        data.append(static_cast<char>(value >> 8));  // High byte
        data.append(static_cast<char>(value & 0xFF));  // Low byte
        sendCommand(data);
        break;

    case Qt::Key_Right:
        value = 90;
        data.append(static_cast<char>(value >> 8));  // High byte
        data.append(static_cast<char>(value & 0xFF));  // Low byte
        sendCommand(data);
        break;

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
    case Qt::Key_A: // Turn Left
        turnLeft = true;
        break;
    case Qt::Key_E: // Turn Right
        turnRight = true;
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
    case Qt::Key_A: // Turn Left
        turnLeft = false;
        break;
    case Qt::Key_E: // Turn Right
        turnRight = false;
        break;
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

void RobotControlWidget::sendSpeed() {
    QByteArray data;
    data.append(static_cast<char>(4));  // SPEED command ID
    data.append(static_cast<char>(speed >> 8));  // High byte
    data.append(static_cast<char>(speed & 0xFF));  // Low byte
    sendCommand(data);
}

void RobotControlWidget::processCommand()
{
    if (!forward && !backward && !left && !right && !turnLeft && !turnRight) {
        QByteArray data;
        data.append(static_cast<char>(6));  // STOP command ID
        sendCommand(data);
        return;
    }

    int16_t angle = 365;
    int8_t turnRate = 0;

    if (forward) {
        if (left) angle = 315;
        else if (right) angle = 45;
        else angle = 0;
    }
    else if (backward) {
        if (left) angle = 225;
        else if (right) angle = 135;
        else angle = 180;
    }
    else if (right) angle = 90;
    else if (left) angle = 270;

    // Set turn rate based on A/E keys
    if (turnLeft && !turnRight) turnRate = -1;  // Negative for left turn
    else if (turnRight && !turnLeft) turnRate = 1;  // Positive for right turn
    // If both are pressed, they cancel out (turnRate remains 0)

    QByteArray data;
    data.append(static_cast<char>(5));  // MOVING command ID
    data.append(static_cast<char>(angle >> 8));  // Angle high byte
    data.append(static_cast<char>(angle & 0xFF));  // Angle low byte
    data.append(static_cast<char>(turnRate));  // Turn rate byte
    sendCommand(data);
}
void RobotControlWidget::sendCommand(const QByteArray &data)
{
    if (!service || isEmergencyStopped) return;

    qDebug() << "Sending command:" << data.toHex();

    QLowEnergyCharacteristic characteristic = service->characteristic(characteristicUuid);
    if (characteristic.isValid()) {
        service->writeCharacteristic(characteristic, data);
    }
}

void RobotControlWidget::emergencyStop()
{
    if (!service) return;

    QByteArray data;
    data.append(static_cast<char>(7));  // EMERGENCY_STOP command ID
    QLowEnergyCharacteristic characteristic = service->characteristic(characteristicUuid);
    if (characteristic.isValid()) {
        service->writeCharacteristic(characteristic, data);
        isEmergencyStopped = true;
        emergencyStopButton->setVisible(false);
        activateButton->setVisible(true);
    }
}

void RobotControlWidget::activateRobot()
{
    if (!service) return;

    QByteArray data;
    data.append(static_cast<char>(8));  // ACTIVATE command ID
    QLowEnergyCharacteristic characteristic = service->characteristic(characteristicUuid);
    if (characteristic.isValid()) {
        service->writeCharacteristic(characteristic, data);
        isEmergencyStopped = false;
        emergencyStopButton->setVisible(true);
        activateButton->setVisible(false);
    }
}
