#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initWindow();
}

MainWindow::~MainWindow()
{
    delete ui;
}


//void MainWindow::mousePressEvent(QMouseEvent *event)
//{
//    m_startPos = event->globalPos();
//    m_startPosOrg = event->pos();
//}

//void MainWindow::mouseMoveEvent(QMouseEvent *event)
//{
//    if(event->buttons() & Qt::LeftButton)
//    {
//        QPoint movePos = event->globalPos() - m_startPos;
//        if( movePos.manhattanLength() > 5)
//            this->move(event->globalPos() - m_startPosOrg);
//        return;
//    }
//}

void MainWindow::initWindow()
{
    setWindowTitle(tr("QSerialPort"));
    setMinimumSize(480, 320);

    Qt::WindowFlags winFlags  = Qt::Dialog;
    winFlags = winFlags | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint;

    setWindowFlags(winFlags);

    /////////////////////////////////////////////////////////////
    QStringList list;
    list.clear();
    list << "1200" << "2400" << "4800" << "9600" << "14400" << \
         "19200" << "38400" << "43000" << "57600" << "76800" << \
         "115200" << "230400" << "256000" << "460800" << "921600";
    ui->cbBaudrate->addItems(list);
    ui->cbBaudrate->setCurrentText(tr("115200"));

    list.clear();
    list << "5" << "6" << "7" << "8";
    ui->cbDatabit->addItems(list);
    ui->cbDatabit->setCurrentText(tr("8"));

    list.clear();
    list << "1" << "1.5" << "2";
    ui->cbStopbit->addItems(list);
    ui->cbStopbit->setCurrentText(tr("1"));

    list.clear();
    list << "none" << "odd" << "even";
    ui->cbParity->addItems(list);
    ui->cbParity->setCurrentText(tr("none"));

    list.clear();
    list << "off" << "hardware" << "software";
    ui->cbFlow->addItems(list);
    ui->cbFlow->setCurrentText(tr("off"));

    ui->btnOpen->setText(tr("打开串口"));

    //////////////////////////////////////////////////////////////
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        ui->cbPortName->addItem(info.portName());
    }
    serial.close(); // ??
    serial.setReadBufferSize(8192);
    serial.setSettingsRestoredOnClose(false);

    //连接信号和槽
    QObject::connect(&serial, &QSerialPort::readyRead, this, &MainWindow::readyRead);
}

void MainWindow::readyRead()
{
    //qDebug() << "readyRead...";
    QByteArray buffer = serial.readAll();
    QString info = QString(buffer);
    ui->txtRecv->appendPlainText(info);
}

void MainWindow::on_btnOpen_clicked()
{
//    qDebug() << ui->cbDatabit->currentText().toInt();
//    qDebug() << ui->cbStopbit->currentText().toInt();
//    qDebug() << ui->cbParity->currentIndex();

    if(ui->btnOpen->text()==QString("打开串口"))
    {
        serial.setPortName(ui->cbPortName->currentText());
        serial.setBaudRate(ui->cbBaudrate->currentText().toInt()); // tocheck...
        //设置数据位
        switch(ui->cbDatabit->currentText().toInt())
        {
        case 8: serial.setDataBits(QSerialPort::Data8); break;
        case 7: serial.setDataBits(QSerialPort::Data7); break;
        case 6: serial.setDataBits(QSerialPort::Data7); break;
        case 5: serial.setDataBits(QSerialPort::Data7); break;
        default: break;
        }
        //设置停止位
        switch(ui->cbStopbit->currentIndex())
        {
        case 0: serial.setStopBits(QSerialPort::OneStop); break;
        case 1: serial.setStopBits(QSerialPort::OneAndHalfStop); break;
        case 2: serial.setStopBits(QSerialPort::TwoStop); break;
        default: break;
        }
        //设置奇偶校验
        switch(ui->cbParity->currentIndex())
        {
        case 0: serial.setParity(QSerialPort::NoParity); break;
        case 1: serial.setParity(QSerialPort::OddParity); break;
        case 2: serial.setParity(QSerialPort::EvenParity); break;
        default: break;
        }

        //设置流控制
        switch(ui->cbFlow->currentIndex())
        {
        case 0: serial.setFlowControl(QSerialPort::NoFlowControl); break;
        case 1: serial.setFlowControl(QSerialPort::HardwareControl); break;
        case 2: serial.setFlowControl(QSerialPort::SoftwareControl); break;
        default: break;
        }
        if( !serial.open(QIODevice::ReadWrite) && !serial.isOpen() )
        {
            QMessageBox::about(NULL, tr("info"), tr("open port failed."));
            return;
        }
        ui->btnOpen->setText(tr("关闭串口"));
    }
    else
    {
        serial.close();
        ui->btnOpen->setText(tr("打开串口"));
    }
}

void MainWindow::on_btnSaveRecv_clicked()
{

}

void MainWindow::on_btnClearRecv_clicked()
{
    ui->txtRecv->clear();
}

void MainWindow::on_ckHex_stateChanged(int arg1)
{

}

void MainWindow::on_ckTime_stateChanged(int arg1)
{

}

void MainWindow::on_btnSend_clicked()
{
    QByteArray data = ui->txtSend->toPlainText().toUtf8();
    //qDebug() << data;
    serial.write(data);
}

void MainWindow::on_btnClearSend_clicked()
{
    ui->txtSend->clear();
}

void MainWindow::on_chSendHex_stateChanged(int arg1)
{

}
