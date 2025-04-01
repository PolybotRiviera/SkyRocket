#include "ledcontrolwidget.hpp"
#include <QVBoxLayout>

LEDControlWidget::LEDControlWidget(QLowEnergyService *service, QWidget *parent)
    : QWidget(parent), service(service)
{
    characteristicUuid = QBluetoothUuid("beb5483e-36e1-4688-b7f5-ea07361b26a8");

    QVBoxLayout *layout = new QVBoxLayout(this);

    colorPreview = new QLabel(this);
    colorPreview->setFixedSize(120, 50);
    colorPreview->setStyleSheet("background-color: black; border: 1px solid gray;");
    layout->addWidget(colorPreview);

    colorButton = new QPushButton("Change LED Color", this);
    layout->addWidget(colorButton);

    toggleButton = new QPushButton("Turn LED On", this);
    toggleButton->setMaximumWidth(120);
    isLedOn = false;
    layout->addWidget(toggleButton);

    brightnessSlider = new QSlider(Qt::Horizontal, this);
    brightnessSlider->setRange(0, 100);
    brightnessSlider->setValue(50);
    brightnessSlider->setMaximumWidth(120);
    layout->addWidget(brightnessSlider);


    layout->setAlignment(Qt::AlignCenter);

    connect(toggleButton, &QPushButton::clicked, this, &LEDControlWidget::toggleLed);
    connect(colorButton, &QPushButton::clicked, this, &LEDControlWidget::changeColor);
    connect(brightnessSlider, &QSlider::valueChanged, this, [this](int value) {
        brightness = value;
    });

    connect(brightnessSlider, &QSlider::sliderReleased, this, &LEDControlWidget::setBrightness);
}

void LEDControlWidget::setService(QLowEnergyService *newService)
{
    service = newService;
}

void LEDControlWidget::changeColor()
{
    if (!service) return;

    QColor newColor = QColorDialog::getColor(_color, this, "Select LED Color");
    if (newColor.isValid()) {
        _color = newColor;
        QLowEnergyCharacteristic characteristic = service->characteristic(characteristicUuid);
        if (characteristic.isValid()) {
            QByteArray data;
            data.append(static_cast<char>(1));  // COLOR_RGB command ID
            data.append(static_cast<char>(_color.red()));
            data.append(static_cast<char>(_color.green()));
            data.append(static_cast<char>(_color.blue()));
            service->writeCharacteristic(characteristic, data);
            qDebug() << "Sending COLOR_RGB:" << data.toHex();
            updateColorPreview(_color);
        } else {
            qDebug() << "LEDControlWidget: Invalid characteristic for writing";
        }
    }
}
void LEDControlWidget::toggleLed()
{
    if (!service) return;

    isLedOn = !isLedOn;
    toggleButton->setText(isLedOn ? "Turn LED Off" : "Turn LED On");
    QLowEnergyCharacteristic characteristic = service->characteristic(characteristicUuid);
    if (characteristic.isValid()) {
        QByteArray data;
        data.append(static_cast<char>(1));  // COLOR_RGB command ID
        if (isLedOn) {
            data.append(static_cast<char>(_color.red()));
            data.append(static_cast<char>(_color.green()));
            data.append(static_cast<char>(_color.blue()));
            updateColorPreview(_color);
        } else {
            data.append(static_cast<char>(0));  // R=0
            data.append(static_cast<char>(0));  // G=0
            data.append(static_cast<char>(0));  // B=0
            updateColorPreview(Qt::black);
        }
        service->writeCharacteristic(characteristic, data);
        qDebug() << "Sending COLOR_RGB (toggle):" << data.toHex();
    }
}
void LEDControlWidget::setBrightness()
{
    if (!service) return;

    QLowEnergyCharacteristic characteristic = service->characteristic(characteristicUuid);
    if (characteristic.isValid()) {
        QByteArray data;
        data.append(static_cast<char>(0));  // BRIGHTNESS command ID
        data.append(static_cast<char>(brightness));  // Brightness value (0-100)
        service->writeCharacteristic(characteristic, data);
        qDebug() << "Sending BRIGHTNESS:" << data.toHex();
        updateColorPreview(_color);
    }
}

void LEDControlWidget::updateColorPreview(const QColor &color)
{
    colorPreview->setStyleSheet(QString("background-color: rgb(%1,%2,%3); border: 1px solid gray;")
                                    .arg(color.red()).arg(color.green()).arg(color.blue()));
}
