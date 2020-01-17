#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QByteArray>
#include <QDebug>
#include <QSerialPort>        //提供访问串口的功能
#include <QSerialPortInfo>    //提供系统中存在的串口的信息

#pragma execution_character_set("utf-8")

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public:
//    void mousePressEvent(QMouseEvent *event);
//    void mouseMoveEvent(QMouseEvent *event);

    void initWindow();

    QSerialPort serial;

    QPoint m_startPos;
    QPoint m_startPosOrg;

private slots:
    void readyRead();
    void on_btnOpen_clicked();

    void on_btnSaveRecv_clicked();

    void on_btnClearRecv_clicked();

    void on_ckHex_stateChanged(int arg1);

    void on_ckTime_stateChanged(int arg1);

    void on_btnSend_clicked();

    void on_btnClearSend_clicked();

    void on_chSendHex_stateChanged(int arg1);

private:
    Ui::MainWindow *ui;

};
#endif // MAINWINDOW_H
