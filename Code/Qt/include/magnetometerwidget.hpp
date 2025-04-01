#ifndef MAGNETOMETERWIDGET_HPP
#define MAGNETOMETERWIDGET_HPP

#include <QWidget>
#include <QLowEnergyService>

class QDoubleSpinBox;
class QPushButton;
class QSlider;
class QLabel;

class MagnetometerWidget : public QWidget {
    Q_OBJECT
public:
    explicit MagnetometerWidget(QLowEnergyService *service, QWidget *parent = nullptr);
    void setService(QLowEnergyService *service);

private slots:
    void onApplyPIDClicked();
    void onCalibrateClicked();
    void onYawClicked();
    void updateHeadingLabel(int value);
    void sendHeadingValue();

private:
    QLowEnergyService *service;       
    QBluetoothUuid characteristicUuid;
    QDoubleSpinBox *kpSpinBox;             
    QDoubleSpinBox *kiSpinBox;               
    QDoubleSpinBox *kdSpinBox;              
    QPushButton *applyButton;               
    QPushButton *calibrateButton;
    QPushButton *yawButton;
    QSlider *headingSlider;               
    QLabel *headingValueLabel;               
    QLabel *headingLabel;
};

#endif
