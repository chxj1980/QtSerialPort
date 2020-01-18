#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QByteArray>
#include <QDebug>
#include <QSerialPort>        //提供访问串口的功能
#include <QSerialPortInfo>    //提供系统中存在的串口的信息
#include <QTimer>
#include <QMessageBox>
#include <QDateTime>

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

    void initMainWindow();
    void initWindow();
    void mousePressEvent(QMouseEvent *event); // 鼠标按下
    void mouseMoveEvent(QMouseEvent *event); // 鼠标移动
    void mouseReleaseEvent(QMouseEvent *event); // 鼠标释放

    void timerEvent(QTimerEvent *event);

    void sendData();

    QSerialPort serial;

    int m_pressMouse;
    QPoint m_startPos;
    QPoint m_windowPos;

    int m_recvHex;
    int m_sendHex;
    int m_sendTimer;

    int m_showTimestamp;

    int m_sendTimerId;

private slots:
    void readyRead();
    void on_btnOpen_clicked();

    void on_btnSaveRecv_clicked();

    void on_btnClearRecv_clicked();

    void on_ckRecvHex_stateChanged(int arg1);

    void on_ckTimestamp_stateChanged(int arg1);

    void on_btnSend_clicked();

    void on_btnClearSend_clicked();

    void on_chSendHex_stateChanged(int arg1);

    void on_ckSendTimer_stateChanged(int arg1);

private:
    Ui::MainWindow *ui;

};
#endif // MAINWINDOW_H
