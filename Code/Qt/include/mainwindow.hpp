#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QTabWidget>
#include "connectionwidget.hpp"
#include "ledcontrolwidget.hpp"
#include "robotcontrolwidget.hpp"
#include "magnetometerwidget.hpp"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void onConnected(QLowEnergyService *service);
    void onDisconnected();
private:
    ConnectionWidget *connectionWidget;
    QTabWidget *tabWidget;
    LEDControlWidget *ledControlWidget;
    RobotControlWidget *robotControlWidget;
    MagnetometerWidget *magWidget;

};

#endif
