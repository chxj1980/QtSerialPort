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
    initMainWindow();
    initWindow();
    initStatusBar();
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == m_stsExit) // 程序退出
    {
        //判断事件
        if(event->type() == QEvent::MouseButtonPress)
        {
            m_stsDebugInfo->setText("press exit label");
            // TODO：直接退出还是发信号？
            emit sig_exit();
            return true; // 事件处理完毕
        }
        else
        {
            return false;
        }
    }
    else
    {
        return QWidget::eventFilter(watched, event);
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    //当鼠标左键点击时.
    if (event->button() == Qt::LeftButton)
    {
        //记录鼠标的世界坐标
        m_startPos = event->globalPos();
        //记录窗体的世界坐标
        m_windowPos = this->frameGeometry().topLeft();
        m_pressMouse = 1;
    }
    else if(event->button() == Qt::RightButton)
    {

    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton)
    {
        if (m_pressMouse)
        {
        //移动中的鼠标位置相对于初始位置的相对位置
        QPoint relativePos = event->globalPos() - m_startPos;
        //然后移动窗体即可
        this->move(m_windowPos + relativePos );

        }
    }
    else if(event->button() == Qt::RightButton)
    {

    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_pressMouse = 0;
}

// 对主窗口的初始化
void MainWindow::initMainWindow()
{
    setWindowTitle(tr("QtSerialPort"));
    setMinimumSize(480, 320);

    Qt::WindowFlags winFlags  = Qt::Dialog;
    winFlags = winFlags | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint;

    setWindowFlags(winFlags);
}

void MainWindow::initWindow()
{
    m_pressMouse = 0;
    m_recvHex = 0;
    m_sendHex = 0;
    m_sendTimer = 0;
    m_sendTimerId = 0;
    /////////////////////////////////////////////////////////////
    QStringList list;
    list.clear();
    list << "2400" << "4800" << "9600" << "14400" << \
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
    ui->btnOpen->setIconSize(ui->btnOpen->rect().size());
    ui->btnOpen->setIcon(QIcon(":images/notopened.ico"));
    //ui->ckRecvHex->setCheckState(Qt::Checked);
    ui->txtInterval->setText("1000");
    ui->txtRecv->setPlaceholderText("Receive data here");
    ui->txtSend->setPlaceholderText("Send data here");
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


void MainWindow::initStatusBar()
{
    // 状态栏分别为：
    // 临时信息（可不用）做一空的label占位
    // 提示信息（可多个）
    // 系统时间
    // 版本信息（或版权声明）

    //ui->statusbar->setStyleSheet(QString("QStatusBar::item{border: 0px}")); // 不显示边框
    ui->statusbar->setSizeGripEnabled(false);//去掉状态栏右下角的三角


    m_stsEmpty = new QLabel();
    m_stsDebugInfo = new QLabel();
    m_stsSysTime = new QLabel();
    m_stsCopyright = new QLabel();
    m_stsExit = new QLabel();

    m_stsEmpty->setMinimumWidth(100);
    ui->statusbar->addWidget(m_stsEmpty);
    m_stsDebugInfo->setMinimumWidth(this->width()/3);
    ui->statusbar->addWidget(m_stsDebugInfo);

    //ui->statusbar->showMessage(tr("临时信息!"),2000);//显示临时信息2000ms 前面的正常信息被覆盖 当去掉后一项时，会一直显示

//    QDateTime dateTime(QDateTime::currentDateTime());
//    QString timeStr = dateTime.toString("yyyy-MM-dd HH:mm:ss.zzz");
//    m_stsSysTime->setText(timeStr);
//    ui->statusbar->addPermanentWidget(m_stsSysTime);

    // 版权信息
    m_stsCopyright->setFrameStyle(QFrame::NoFrame);
    m_stsCopyright->setText(tr("  <a href=\"https://www.latelee.org\">技术主页</a>  "));
    m_stsCopyright->setOpenExternalLinks(true);
    ui->statusbar->addPermanentWidget(m_stsCopyright);

    // 退出图标
    m_stsExit->installEventFilter(this); // 安装事件过滤，以便获取其单击事件
    m_stsExit->setToolTip("Exit App");
    // 贴图
    QPixmap exitIcon(":/images/exit.png");
    m_stsExit->setMinimumWidth(32);
    m_stsExit->setPixmap(exitIcon);
    ui->statusbar->addPermanentWidget(m_stsExit);

    connect(this, &MainWindow::sig_exit, qApp, &QApplication::quit); // 直接关联到全局的退出槽
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

void MainWindow::sendHexData(QString& tips, uint8_t* ibuf, uint8_t ilen)
{
    QByteArray sendData;

    for (int i = 0; i < ilen; i++)
    {
        sendData[i] = ibuf[i];
    }

    QString tmp = sendData.toHex(' ').toLower();
    serial.write(sendData);
    //ui->txtSend->appendPlainText(tips + tmp);
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
    
    // TODO
    // 似乎直接用QByteArray无法直接取真正的值
    // 这里先转为数组，再判断，需要优化
    uint8_t *data = (uint8_t*)buffer.data();
    uint8_t buf[255] = {0};
    for (int i = 0; i < buffer.size(); i++)
    {
        buf[i] = data[i];
        //qDebug("%x ", buf[i]);
    }
    
    // 根据值判断做逻辑处理，可做成函数
    if (buf[0] == 0xa5 && buf[1] == 0x5a)
    {
    
    }
    
}

void MainWindow::on_btnOpen_clicked()
{
//    qDebug() << ui->cbDatabit->currentText().toInt();
//    qDebug() << ui->cbStopbit->currentText().toInt();
//    qDebug() << ui->cbParity->currentIndex();

    if(ui->btnOpen->text()==QString("打开串口"))
    {
        on_cbPortName_currentTextChanged(ui->cbPortName->currentText());
        on_cbBaudrate_currentTextChanged(ui->cbBaudrate->currentText()); // tocheck...
        //设置数据位
        on_cbDatabit_currentTextChanged(ui->cbDatabit->currentText());
//        switch(ui->cbDatabit->currentText().toInt())
//        {
//        case 8: serial.setDataBits(QSerialPort::Data8); break;
//        case 7: serial.setDataBits(QSerialPort::Data7); break;
//        case 6: serial.setDataBits(QSerialPort::Data6); break;
//        case 5: serial.setDataBits(QSerialPort::Data5); break;
//        default: break;
//        }
        //设置停止位
        on_cbStopbit_currentIndexChanged(ui->cbStopbit->currentIndex());
//        switch(ui->cbStopbit->currentIndex())
//        {
//        case 0: serial.setStopBits(QSerialPort::OneStop); break;
//        case 1: serial.setStopBits(QSerialPort::OneAndHalfStop); break;
//        case 2: serial.setStopBits(QSerialPort::TwoStop); break;
//        default: break;
//        }
        //设置奇偶校验
        switch(ui->cbParity->currentIndex())
        {
        case 0: serial.setParity(QSerialPort::NoParity); break;
        case 1: serial.setParity(QSerialPort::OddParity); break;
        case 2: serial.setParity(QSerialPort::EvenParity); break;
        default: break;
        }

        //设置流控制
        on_cbFlow_currentIndexChanged(ui->cbFlow->currentIndex());
//        switch(ui->cbFlow->currentIndex())
//        {
//        case 0: serial.setFlowControl(QSerialPort::NoFlowControl); break;
//        case 1: serial.setFlowControl(QSerialPort::HardwareControl); break;
//        case 2: serial.setFlowControl(QSerialPort::SoftwareControl); break;
//        default: break;
//        }
        if(!serial.open(QIODevice::ReadWrite) && !serial.isOpen())
        {
            QMessageBox::about(NULL, tr("info"), tr("open port failed."));
            return;
        }

        ui->btnOpen->setText(tr("关闭串口"));
        ui->btnOpen->setIcon(QIcon(":images/opened.ico"));
    }
    else
    {
        serial.close();

        ui->btnOpen->setText(tr("打开串口"));
        ui->btnOpen->setIcon(QIcon(":images/notopened.ico"));
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

void MainWindow::on_cbPortName_currentTextChanged(const QString &arg1)
{
    serial.setPortName(arg1);
}

void MainWindow::on_cbBaudrate_currentTextChanged(const QString &arg1)
{
    serial.setBaudRate(arg1.toInt());
}

void MainWindow::on_cbDatabit_currentTextChanged(const QString &arg1)
{
    switch(arg1.toInt())
    {
    case 8: serial.setDataBits(QSerialPort::Data8); break;
    case 7: serial.setDataBits(QSerialPort::Data7); break;
    case 6: serial.setDataBits(QSerialPort::Data6); break;
    case 5: serial.setDataBits(QSerialPort::Data5); break;
    default: break;
    }
}

void MainWindow::on_cbStopbit_currentIndexChanged(int index)
{
    //qDebug()<< index;
    //设置停止位
    switch(index)
    {
    case 0: serial.setStopBits(QSerialPort::OneStop); break;
    case 1: serial.setStopBits(QSerialPort::OneAndHalfStop); break;
    case 2: serial.setStopBits(QSerialPort::TwoStop); break;
    default: break;
    }
}

void MainWindow::on_cbParity_currentIndexChanged(int index)
{
    //设置奇偶校验
    switch(index)
    {
    case 0: serial.setParity(QSerialPort::NoParity); break;
    case 1: serial.setParity(QSerialPort::OddParity); break;
    case 2: serial.setParity(QSerialPort::EvenParity); break;
    default: break;
    }
}

void MainWindow::on_cbFlow_currentIndexChanged(int index)
{
    //设置流控制
    switch(index)
    {
    case 0: serial.setFlowControl(QSerialPort::NoFlowControl); break;
    case 1: serial.setFlowControl(QSerialPort::HardwareControl); break;
    case 2: serial.setFlowControl(QSerialPort::SoftwareControl); break;
    default: break;
    }
}
