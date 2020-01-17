#include "mainwindow.h"
#include "ui_mainwindow.h"


void stringTohexString(QString& str, QString& hexStr)
{

}

int hexStringToString(QString& hexStr, QString& str)
{
    int ret = 0;
    bool ok;
    QByteArray retByte;
    hexStr = hexStr.trimmed();
    hexStr = hexStr.simplified();
    QStringList sl = hexStr.split(" ");

    foreach (QString s, sl)
    {
        if(!s.isEmpty())
        {
            char c = (s.toInt(&ok,16))&0xFF;
            if (ok)
            {
                retByte.append(c);
            }
            else
            {
                ret = -1;
            }
        }
    }

    str = retByte;

    return ret;
}

int hexStringToHexArray(QString& hexStr, QByteArray& arr)
{
    int ret = 0;
    bool ok;
    hexStr = hexStr.trimmed();
    hexStr = hexStr.simplified();
    QStringList sl = hexStr.split(" ");

    foreach (QString s, sl)
    {
        if(!s.isEmpty())
        {
            char c = (s.toInt(&ok,16))&0xFF;
            if (ok)
            {
                arr.append(c);
            }
            else
            {
                ret = -1;
            }
        }
    }

    return ret;
}

int hexArrayToString(QByteArray& hexArr, QString& str)
{
    int ret = 0;
    str = hexArr.toHex(' ').toLower();
    return ret;
}

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
    setWindowTitle(tr("QtSerialPort"));
    setMinimumSize(480, 320);

    Qt::WindowFlags winFlags  = Qt::Dialog;
    winFlags = winFlags | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint;

    setWindowFlags(winFlags);

    /////////////////////////////////////////////////////////////
    m_recvHex = 0;
    m_sendHex = 0;
    m_sendTimer = 0;
    m_sendTimerId = 0;
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
    //ui->btnOpenStatus->setIcon(QIcon(":/images/notopened.ico"));
    //ui->ckRecvHex->setCheckState(Qt::Checked);
    ui->txtInterval->setText("1000");
    ui->txtRecv->setPlaceholderText("Receive data here");
    ui->txtSend->setPlainText("hello world");

    //////////////////////////////////////////////////////////////
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        ui->cbPortName->addItem(info.portName());
    }
    serial.close(); // try close
    serial.setReadBufferSize(8192);
    serial.setSettingsRestoredOnClose(false);

    //连接信号和槽
    QObject::connect(&serial, &QSerialPort::readyRead, this, &MainWindow::readyRead);

    on_btnOpen_clicked(); // todo
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    //qDebug() << "Timer ID:" << event->timerId();
    sendData();
}

void MainWindow::sendData()
{
    QString sendStr = ui->txtSend->toPlainText().toLatin1().toLower();
    QByteArray sendData;
    QString showStr;

    if (m_sendHex == 1)
    {
        hexStringToHexArray(sendStr, sendData);
    }
    else if (m_sendHex == 0)
    {
        sendData = sendStr.toLatin1();
    }
    //qDebug() << sendData;
    serial.write(sendData);
}

void MainWindow::readyRead()
{
    QByteArray buffer = serial.readAll();
    QString info;
    QString tmpStr;
    QString timeStr = "";
    if (m_showTimestamp)
    {
        QDateTime dateTime(QDateTime::currentDateTime());
        timeStr = "[" + dateTime.toString("yyyy-MM-dd HH:mm::ss.zzz") + "] ";
    }

    if (m_recvHex == 1)
    {
        info = buffer.toHex(' ').data();
//        for (int i = 0; i < buffer.size(); i++)
//        {
//            tmpStr.sprintf("%02x ", buffer.at(i));
//            qDebug() << buffer.at(i);
//            info.append(tmpStr);
//        }
    }
    else
    {
        info = QString(buffer);
    }

    ui->txtRecv->appendPlainText(timeStr + info);
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
        //ui->lbPortStatus->setIcon(QIcon(":/images/opened.ico"));
        ui->btnOpen->setText(tr("关闭串口"));
    }
    else
    {
        serial.close();
        //ui->lbPortStatus->setIcon(QIcon(":/images/notopened.ico"));
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

void MainWindow::on_ckRecvHex_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
    {
        m_recvHex = 1;
    }
    else if (arg1 == Qt::Unchecked)
    {
        m_recvHex = 0;
    }
}

void MainWindow::on_ckTimestamp_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
    {
        m_showTimestamp = 1;
    }
    else if (arg1 == Qt::Unchecked)
    {
        m_showTimestamp = 0;
    }
}

void MainWindow::on_btnSend_clicked()
{
    if (m_sendTimer)
    {
        if(ui->btnSend->text()==QString("发送"))
        {
            if (m_sendTimerId == 0)
                m_sendTimerId = startTimer(ui->txtInterval->text().toInt());
            ui->btnSend->setText(tr("停止发送"));
        }
        else
        {
            if (m_sendTimerId)
            {
                killTimer(m_sendTimerId);
                m_sendTimerId = 0;
            }
            ui->btnSend->setText(tr("发送"));
        }
    }
    else
    {
        sendData();
    }
}

void MainWindow::on_btnClearSend_clicked()
{
    ui->txtSend->clear();
}

void MainWindow::on_chSendHex_stateChanged(int arg1)
{
    QString sendStr = ui->txtSend->toPlainText().toLatin1().toLower();
    QByteArray sendData = sendStr.toLatin1();//ui->txtSend->toPlainText().toLatin1();
    QString tmpStr;
    QString showStr;
    //qDebug() << sendStr;
    //qDebug() << sendData;
    if (arg1 == Qt::Checked)
    {
        m_sendHex = 1;
        showStr = sendData.toHex(' ').data();
    }
    else if (arg1 == Qt::Unchecked)
    {
        m_sendHex = 0;
        hexStringToString(sendStr, showStr); // 用string来转
    }
    ui->txtSend->setPlainText(showStr);
}

void MainWindow::on_ckSendTimer_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
    {
        m_sendTimer = 1;
    }
    else if (arg1 == Qt::Unchecked)
    {
        m_sendTimer = 0;
        if (m_sendTimerId)
        {
            killTimer(m_sendTimerId);
            m_sendTimerId = 0;
        }
    }
}
