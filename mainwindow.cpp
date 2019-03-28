#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QMessageBox>

using namespace std;
using namespace myo;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    hub = nullptr;
    isRunning = false;
    ui->widgetControlPanel->setEnabled(false);
    dataCollector = new DataCollector(this);

    connect(dataCollector, &DataCollector::connectionLost, [this] () {
        isRunning = false;
        updateButtons();
        ui->widgetControlPanel->setEnabled(false);
        ui->pushButtonConnect->setEnabled(true);
        QMessageBox::warning(this, tr("Warning"), tr("Connection Lost..."));
    });

    connect(ui->actionAbout, &QAction::triggered, [this] () {
        QMessageBox::about(this, QString("About"), tr("This Application is written by Celal Savur \n"
                                                      "For any feedback or suggestions please contact to cs1323@rit.edu \n"
                                                      "Thank you for using this application..."));
    });

    connect(ui->actionAboutQt, SIGNAL(triggered(bool)), qApp, SLOT(aboutQt()));
}

MainWindow::~MainWindow()
{
    qDebug() << "~MainWindow()";
    if(hub)
        delete hub;
    delete ui;
}

void MainWindow::on_pushButtonStartStreaming_clicked()
{
    isRunning = true;
    updateButtons();
    while (isRunning) {
        hub->runOnce(1);
        qApp->processEvents();
    }
    qDebug() << "Stopped" ;
    updateButtons();
}

void MainWindow::on_pushButtonStopStreaming_clicked()
{
    isRunning = false;
    updateButtons();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    isRunning = false;
    QMainWindow::closeEvent(event);
}

void MainWindow::on_pushButtonConnect_clicked()
{
    bool connectionStatus = false;
    try {
        hub = new myo::Hub("edu.rit.mabl");
        hub->setLockingPolicy(hub->lockingPolicyNone);
        std::cout << "Attempting to find a Myo..." << std::endl;
        qApp->processEvents();
        myo::Myo* myo = hub->waitForMyo(10000);
        if (!myo) {
            throw std::runtime_error("Unable to find a Myo!");
        }
        connectionStatus = true;
        myo->setStreamEmg(myo::Myo::streamEmgEnabled);
        hub->addListener(dataCollector);
    } catch (const std::exception& e) {
        std::cout << "Error #" << e.what() << std::endl;
        return;
    }

    // if connection is ok
    if(connectionStatus) {
        ui->widgetControlPanel->setEnabled(true);
        ui->pushButtonConnect->setEnabled(false);
    }

    updateButtons();
}

void MainWindow::updateButtons()
{
    ui->pushButtonStartStreaming->setEnabled(!isRunning);
    ui->pushButtonStopStreaming->setEnabled(isRunning);
}
