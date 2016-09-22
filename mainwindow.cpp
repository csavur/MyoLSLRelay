#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <vector>
#include <QScrollBar>
#include <QTime>
#include <QMessageBox>
#include <QFile>

#define M_PI  (3.1418)

using namespace std;
using namespace myo;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    poseEnum = -1;

    isRunning = false;
    ui->widgetControlPanel->setEnabled(false);


    connect(&tcpServer, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
    haveAconnection = false;
    tcpServer.listen(QHostAddress::AnyIPv4, 2000);

    connect(ui->actionAbout, SIGNAL(triggered(bool)), this, SLOT(slotOpenAbout()));
    connect(ui->actionAboutQt, SIGNAL(triggered(bool)), qApp, SLOT(aboutQt()));
}

MainWindow::~MainWindow()
{
    qDebug() << "~MainWindow()";
    delete hub;
    delete ui;
}

/// This callback will be called when new data is available
/// from myo arm band, the frequency of the myo arm band is 200 Hz
void MainWindow::onEMGCallbackFunction(void *p, int i)
{
    // Get back into the class by treating p as the "this" pointer.
    ((MainWindow *)p) -> callbackFunctionEMG(i);
}

void MainWindow::callbackFunctionEMG(int i)
{
    Q_UNUSED(i);

    int nSamples = ui->spinBoxBufferSize->value();
    //cout << "Inside callback :" << i << endl;

    qApp->processEvents();
    QString d;
    for (size_t i = 0; i < dataCollector.emgSamples.size(); i++) {
        d.append(QString::number(static_cast<int>(dataCollector.emgSamples[i])) + QString(", "));
    }

    if (buffer.size() == nSamples) {
        // send matlab
        QString txt  = buffer.join("\n");
        txt.append("\n");

        if (ui->checkBoxResult->isChecked()) {
            ui->textEditEMGResult->append(txt);
        }

        sendData();

        buffer.clear();
        ui->textEditEMGResult->append(tr("%1 samples Data sent to matlab... Time stamp : %2 ms").arg(nSamples).arg(timeStamp.elapsed()));
        timeStamp.restart();
    } else {
        buffer.append(d);
    }

    ui->textEditEMGResult->verticalScrollBar()->setValue(ui->textEditEMGResult->verticalScrollBar()->maximum());
}

void MainWindow::onGYROCallbackFunction(void *p, int i)
{
    ((MainWindow *)p)->callbackFunctionGYRO(i);
}

void MainWindow::callbackFunctionGYRO(int i)
{
    Q_UNUSED(i);

    const myo::Quaternion<float>& quat = dataCollector.getRotation();

    using std::atan2;
    using std::asin;
    using std::sqrt;
    using std::max;
    using std::min;

    // Calculate Euler angles (roll, pitch, and yaw) from the unit quaternion.
    float roll = atan2(2.0f * (quat.w() * quat.x() + quat.y() * quat.z()), 1.0f - 2.0f * (quat.x() * quat.x() + quat.y() * quat.y()));
    float pitch = asin(max(-1.0f, min(1.0f, 2.0f * (quat.w() * quat.y() - quat.z() * quat.x()))));
    float yaw = atan2(2.0f * (quat.w() * quat.z() + quat.x() * quat.y()), 1.0f - 2.0f * (quat.y() * quat.y() + quat.z() * quat.z()));
    // Convert the floating point angles in radians to a scale from 0 to 18.
    roll_w = static_cast<int>((roll + (float)M_PI)/(M_PI * 2.0f) * 18);
    pitch_w = static_cast<int>((pitch + (float)M_PI/2.0f)/M_PI * 18);
    yaw_w = static_cast<int>((yaw + (float)M_PI)/(M_PI * 2.0f) * 18);

    QString gyro;
    gyro.append("roll :" + QString::number(roll_w) + " pithc :" + QString::number(pitch_w) + " yaw :" + QString::number(yaw_w));
    ui->textEditGYROResult->append(gyro);
}

void MainWindow::onPoseCallbackFunction(void *p, int i)
{
    ((MainWindow *)p)->callbackFunctionPose(i);
}

void MainWindow::callbackFunctionPose(int i)
{
    Q_UNUSED(i);
    QString pose;

    switch (dataCollector.getPose().type()) {
    case myo::Pose::rest:
        pose.append("rest");
        break;
    case myo::Pose::fist:
        pose.append("fist");
        break;
    case myo::Pose::waveIn:
        pose.append("wave in");
        break;
    case myo::Pose::waveOut:
        pose.append("wave out");
        break;
    case myo::Pose::fingersSpread:
        pose.append("finger Spread");
        break;
    case myo::Pose::doubleTap:
        pose.append("Double tab");
        break;
    default:
        pose.append("Unknown gesture");
        break;
    }
    ui->textEditPose->append(pose);

    // This last pose will be sended to Matlab
    poseEnum = (int)dataCollector.getPose().type();
}

void MainWindow::readEMG()
{
    dataCollector.resetCounter();
    dataCollector.registerEMGCallback(MainWindow::onEMGCallbackFunction, this);
    dataCollector.registerGYROCallback(MainWindow::onGYROCallbackFunction, this);
    dataCollector.registerPoseCallback(MainWindow::onPoseCallbackFunction, this);
    while (isRunning) {
        hub->runOnce(1);  // 1000 is per sec
        // NOTE when EMG data is available controller will triger the callback function
    }
    qDebug() << "Stopped" ;
}

void MainWindow::on_pushButtonStartStreaming_clicked()
{
    isRunning = true;
    readEMG();
}

void MainWindow::on_pushButtonStopStreaming_clicked()
{
    isRunning = false;
}

void MainWindow::on_pushButtonClear_clicked()
{
    ui->textEditEMGResult->clear();
    ui->textEditGYROResult->clear();
    ui->textEditPose->clear();
}

void MainWindow::acceptConnection()
{
    tcpServerConnection = tcpServer.nextPendingConnection();

    connect(tcpServerConnection, SIGNAL(disconnected()), this, SLOT(on_pushButtonStopStreaming_clicked()));
    qDebug() << "Got a new Connection" ;

    haveAconnection = true;

    if(ui->checkBoxAutoStart->isChecked()) {
        isRunning = true;
        readEMG();
    }
}

QString MainWindow::addPoseGyroInfo()
{
    QString poseString;

    poseString.append(QString::number(poseEnum) + QString(", "));
    // roll_w, pitch_w, yaw_w;
    poseString.append(QString::number(roll_w) + QString(", "));
    poseString.append(QString::number(pitch_w) + QString(", "));
    poseString.append(QString::number(yaw_w) + QString(", "));

    for (int i = 0; i < 4; i++) {
        poseString.append( "-1" + QString(", "));
    }

    poseEnum = -1;

    return poseString;
}

void MainWindow::sendData()
{
    if(haveAconnection) {
        if(tcpServerConnection) {
            QByteArray byteArray;

            if(poseEnum != -1) {
                QString poseString = addPoseGyroInfo();
                buffer.prepend(poseString);
                qDebug() << "pose sent...";
            }

            //
            QString txt  = buffer.join(" ");
            byteArray.append(txt);

            byteArray.append("\n");
            tcpServerConnection->write(byteArray);
            tcpServerConnection->flush();

            qDebug() << "size buffer" <<byteArray.size();
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    isRunning = false;
    QMainWindow::closeEvent(event);
}

void MainWindow::on_pushButtonConnect_clicked()
{
    bool connectionStatus = false;
    // try to connect myo
    // We catch any exceptions that might occur below -- see the catch statement for more details.
    try {

        // First, we create a Hub with our application identifier. Be sure not to use the com.example namespace when
        // publishing your application. The Hub provides access to one or more Myos.
        hub = new myo::Hub("com.example.emg-data-sample");

        //std::cout << "Attempting to find a Myo..." << std::endl;
        ui->textEditEMGResult->setText("Attempting to find a Myo...");

        qApp->processEvents();
        // Next, we attempt to find a Myo to use. If a Myo is already paired in Myo Connect, this will return that Myo
        // immediately.
        // waitForMyo() takes a timeout value in milliseconds. In this case we will try to find a Myo for 10 seconds, and
        // if that fails, the function will return a null pointer.
        myo::Myo* myo = hub->waitForMyo(10000);

        // If waitForMyo() returned a null pointer, we failed to find a Myo, so exit with an error message.
        if (!myo) {
            throw std::runtime_error("Unable to find a Myo!");
        }

        // We've found a Myo.
        ui->textEditEMGResult->append("Connected to a Myo armband!");

        // Next we enable EMG streaming on the found Myo.
        myo->setStreamEmg(myo::Myo::streamEmgEnabled);

        // Hub::addListener() takes the address of any object whose class inherits from DeviceListener, and will cause
        hub->addListener(&dataCollector);

        connectionStatus = true;
        // If a standard exception occurred, we print out its message and exit.
    } catch (const std::exception& e) {
        ui->textEditEMGResult->append(QString("Error : ") + QString::fromStdString(e.what()));
        return;
    }

    // if connection is ok
    if(connectionStatus) {
        ui->widgetControlPanel->setEnabled(true);
        //ui->pushButtonConnect->setEnabled(false);
    }
}

void MainWindow::on_pushButtonTest_clicked()
{
    buffer.clear();

    buffer.append("-1, 0, -1, -17, -5, -1, -2, 1,");

    sendData();
}

void MainWindow::on_checkBoxAutoStart_clicked(bool checked)
{
    ui->pushButtonTest->setEnabled(!checked);
}

void MainWindow::on_pushButtonDataSet_clicked()
{
    QFile file("C:/Users/cs1323/Documents/GitHub/workspace/ASL_code/csvlist.csv");

    file.open(QIODevice::ReadOnly);
    QTextStream stream(&file);
    while (!stream.atEnd()){
        QString s = stream.readLine(); // reads line from file
        chData.append(s.split(",")); // appends first column to list, ',' is separator
    }
    file.close();

    connect(&timer, SIGNAL(timeout()), this,SLOT(timerTimeOut()));

    timer.start(250);

    poseEnum = 5;
}

void MainWindow::timerTimeOut()
{
    // Each time will send the 50x8 data to matlab
    static int counter = 0;
    qDebug() << counter;
    if(counter == 160) {
        timer.stop();
        counter = 0;
    }

    if(haveAconnection) {
        for (int i = (0 + counter*50); i < (counter*50 + 50) ; ++i) {
            QString row;
            for (int j = 0; j < 8; ++j) {
                row.append(chData.at(j).at(i) + ", ");
            }
            buffer.append(row);
            row.clear();
        }
    }
    sendData();
    counter++;
    buffer.clear();
}

void MainWindow::slotOpenAbout()
{
    QMessageBox::about(this, QString("About"), tr("This Application is written by Celal SAVUR \n"
                                                  "For any feedback or suggestions please contact to cs1323@rit.edu \n"
                                                  "Thank you for using this application..."));
}
