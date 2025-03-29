#ifndef LEDCONTROLWIDGET_HPP
#define LEDCONTROLWIDGET_HPP

#include <QWidget>
#include <QLowEnergyService>
#include <QPushButton>
#include <QColorDialog>
#include <qlabel.h>
#include <qslider.h>

class LEDControlWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LEDControlWidget(QLowEnergyService *service, QWidget *parent = nullptr);
    void setService(QLowEnergyService *service);
private slots:
    void changeColor();
    void toggleLed();
    void updateColorPreview(const QColor &color);
    void setBrightness();
private:
    QLowEnergyService *service;
    QPushButton *colorButton;
    QPushButton *toggleButton;
    QSlider *brightnessSlider;
    QLabel *colorPreview;
    QBluetoothUuid characteristicUuid;
    QColor _color = Qt::white;
    bool isLedOn;
    int brightness = 50;
};

#endif
