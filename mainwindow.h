#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "datacollector.h"
#include "myo/myo.hpp"

#include <QTime>
#include <QTimer>

#include <QTcpServer>
#include <QTcpSocket>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static void onEMGCallbackFunction(void *p, int i);
    void callbackFunctionEMG(int i);

    static void onGYROCallbackFunction(void *p, int deviceID);
    void callbackFunctionGYRO(int deviceID);

    static void onPoseCallbackFunction(void *p, int i);
    void callbackFunctionPose(int i);

private slots:
    void on_pushButtonConnect_clicked();
    void on_pushButtonStartStreaming_clicked();
    void on_pushButtonStopStreaming_clicked();
    void on_pushButtonClear_clicked();
    void acceptConnection();

    QString addPoseGyroInfo();

    void registerCallback();
    void sendData();

    void timerTimeOut();
    void slotOpenAbout();


protected:

    void closeEvent(QCloseEvent *event);

private:
    Ui::MainWindow *ui;

    DataCollector dataCollector;
    myo::Hub *hub;

    bool isRunning;
    QStringList buffer;


    QTcpServer tcpServer;
    QTcpSocket *tcpServerConnection;

    bool haveAconnection;

    QTime timeStamp;

    // These values are set by onOrientationData() and onPose() above.
    int roll_w, pitch_w, yaw_w;

    int poseEnum;

    QList<QStringList> chData;

    QTimer timer;

};


#endif // MAINWINDOW_H
