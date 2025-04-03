#ifndef BEACONCONTROLWIDGET_HPP
#define BEACONCONTROLWIDGET_HPP

#include <QWidget>
#include <QLowEnergyService>

class QDoubleSpinBox;
class QPushButton;
class QLineEdit;
class QLabel;

class BeaconControlWidget : public QWidget {
    Q_OBJECT
public:
    explicit BeaconControlWidget(QLowEnergyService *service, QWidget *parent = nullptr);
    void setService(QLowEnergyService *service);

private slots:
    void onApplyPIDClicked();
    void onCalibrateClicked();
    void onGoToToggleClicked();

private:
    QLowEnergyService *service;
    QBluetoothUuid characteristicUuid;
    QDoubleSpinBox *kpSpinBox;
    QDoubleSpinBox *kiSpinBox;
    QDoubleSpinBox *kdSpinBox;
    QLineEdit *targetInput;  // For x,y coordinates
    QPushButton *applyButton;
    QPushButton *calibrateButton;
    QPushButton *goToButton;
    QLabel *targetLabel;
};

#endif
