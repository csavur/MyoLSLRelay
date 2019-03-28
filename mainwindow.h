#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "datacollector.h"
#include "myo/myo.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButtonConnect_clicked();
    void on_pushButtonStartStreaming_clicked();
    void on_pushButtonStopStreaming_clicked();

    void updateButtons();

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::MainWindow *ui;

    DataCollector *dataCollector;
    myo::Hub *hub;

    bool isRunning;
};


#endif // MAINWINDOW_H
