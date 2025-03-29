#include "mainwindow.hpp"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    QWidget *centralWidget = new QWidget(this);
    tabWidget = new QTabWidget(this);

    connectionWidget = new ConnectionWidget(this);
    ledControlWidget = new LEDControlWidget(nullptr, this);
    robotControlWidget = new RobotControlWidget(nullptr, this);
    magWidget = new MagnetometerWidget(nullptr, this);

    tabWidget->addTab(connectionWidget, "Connection");
    tabWidget->addTab(robotControlWidget, "Robot Control");
    tabWidget->addTab(ledControlWidget, "LED Control");
    tabWidget->addTab(magWidget, "Magnetometer");

    setCentralWidget(tabWidget);

    tabWidget->setCurrentWidget(connectionWidget);

    connect(connectionWidget, &ConnectionWidget::connected, this, &MainWindow::onConnected);
    connect(connectionWidget, &ConnectionWidget::disconnected, this, &MainWindow::onDisconnected);

    tabWidget->setTabEnabled(tabWidget->indexOf(ledControlWidget), true);
    tabWidget->setTabEnabled(tabWidget->indexOf(robotControlWidget), true);
    tabWidget->setTabEnabled(tabWidget->indexOf(magWidget), true);


    setWindowTitle("SkyRocket");
    resize(400, 300);
}

MainWindow::~MainWindow(){

    if (connectionWidget) {
        qDebug() << "Deleting ConnectionWidget";
        delete connectionWidget;
        connectionWidget = nullptr;
    }

}

void MainWindow::onConnected(QLowEnergyService *service)
{
    ledControlWidget->setService(service);
    robotControlWidget->setService(service);
    magWidget->setService(service);

    tabWidget->setTabEnabled(tabWidget->indexOf(ledControlWidget), true);
    tabWidget->setTabEnabled(tabWidget->indexOf(robotControlWidget), true);
    tabWidget->setTabEnabled(tabWidget->indexOf(magWidget), true);

    tabWidget->setTabEnabled(tabWidget->indexOf(connectionWidget), false);

    tabWidget->setCurrentWidget(robotControlWidget);
}

void MainWindow::onDisconnected()
{
    ledControlWidget->setService(nullptr);
    robotControlWidget->setService(nullptr);

    tabWidget->setTabEnabled(tabWidget->indexOf(ledControlWidget), false);
    tabWidget->setTabEnabled(tabWidget->indexOf(robotControlWidget), false);
    tabWidget->setTabEnabled(tabWidget->indexOf(magWidget), false);

    tabWidget->setTabEnabled(tabWidget->indexOf(connectionWidget), true);
    
    tabWidget->setCurrentWidget(connectionWidget);
}
