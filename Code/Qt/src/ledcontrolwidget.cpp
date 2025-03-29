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
    brightnessSlider->setValue(50); // Initial speed
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

    _color = QColorDialog::getColor(Qt::white, this, "Select LED Color");
    if (_color.isValid()) {
        QString command = "ColorRGB " + QString("%1 %2 %3").arg(_color.red()).arg(_color.green()).arg(_color.blue());
        QLowEnergyCharacteristic characteristic = service->characteristic(characteristicUuid);
        if (characteristic.isValid()) {
            service->writeCharacteristic(characteristic, command.toUtf8());
        } else {
            qDebug() << "LEDControlWidget: Invalid characteristic for writing";
        }
        updateColorPreview(_color);
    }
}

void LEDControlWidget::toggleLed()
{
    if (!service) return;
    isLedOn = !isLedOn;
    toggleButton->setText(isLedOn ? "Turn LED Off" : "Turn LED On");
    QLowEnergyCharacteristic characteristic = service->characteristic(characteristicUuid);
    if (characteristic.isValid()) {
        QString command = "ColorRGB " + QString("%1 %2 %3").arg(_color.red()).arg(_color.green()).arg(_color.blue());
        if (isLedOn){
            service->writeCharacteristic(characteristic, command.toUtf8());
            updateColorPreview(_color);
        }
        else{
            service->writeCharacteristic(characteristic, "ColorRGB 0 0 0");
            updateColorPreview(Qt::black);
        }

    }
}
void LEDControlWidget::setBrightness()
{
    if (!service) return;
    QLowEnergyCharacteristic characteristic = service->characteristic(characteristicUuid);
    if (characteristic.isValid()) {
        QString command = "Brightness " + QString("%1").arg(brightness);
        service->writeCharacteristic(characteristic, command.toUtf8());
        updateColorPreview(_color);
    }
}

void LEDControlWidget::updateColorPreview(const QColor &color)
{
    colorPreview->setStyleSheet(QString("background-color: rgb(%1,%2,%3); border: 1px solid gray;")
                                    .arg(color.red()).arg(color.green()).arg(color.blue()));
}
