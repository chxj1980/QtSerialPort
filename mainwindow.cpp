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
    initStatusBar();
    initWindow();
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_stsExit) // 程序退出
    {
        //判断事件
        if(event->type() == QEvent::MouseButtonPress)
        {
            // TODO：直接退出还是发信号？
            emit sig_exit();
            return true; // 事件处理完毕
        }
        else
        {
            return false;
        }
    }
    else if (watched == m_stsResetCnt)
    {
        if(event->type() == QEvent::MouseButtonPress)
        {
            m_stsRx->setText("RX: 0");
            m_stsTx->setText("TX: 0");
            m_rxCnt = m_txCnt = 0;
            return true;
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

    m_showTimestamp = 0;
    m_sendNewline = 0;

    m_rxCnt = 0;
    m_txCnt = 0;

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
        if (-1 == ui->cbPortName->findText(info.portName()))
            ui->cbPortName->addItem(info.portName());
    }
    serial.close(); // try close
    serial.setReadBufferSize(8192);
    serial.setSettingsRestoredOnClose(false);

    //连接信号和槽
    QObject::connect(&serial, &QSerialPort::readyRead, this, &MainWindow::readyRead);

    QObject::connect(this, &MainWindow::sig_deviceChanged, this, &MainWindow::on_deviceChanged);
    // 注册各类设备，可根据要求删减
#if 01
    static const GUID GUID_DEVINTERFACE_LIST[] =
    {
    // GUID_DEVINTERFACE_USB_DEVICE
    { 0xA5DCBF10, 0x6530, 0x11D2, { 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } },
    // GUID_DEVINTERFACE_DISK
    { 0x53f56307, 0xb6bf, 0x11d0, { 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b } },
    // GUID_DEVINTERFACE_HID,
    { 0x4D1E55B2, 0xF16F, 0x11CF, { 0x88, 0xCB, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 } },
    // GUID_NDIS_LAN_CLASS
    { 0xad498944, 0x762f, 0x11d0, { 0x8d, 0xcb, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c } }
    //// GUID_DEVINTERFACE_COMPORT
    //{ 0x86e0d1e0, 0x8089, 0x11d0, { 0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73 } },
    //// GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR
    //{ 0x4D36E978, 0xE325, 0x11CE, { 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } },
    //// GUID_DEVINTERFACE_PARALLEL
    //{ 0x97F76EF0, 0xF883, 0x11D0, { 0xAF, 0x1F, 0x00, 0x00, 0xF8, 0x00, 0x84, 0x5C } },
    //// GUID_DEVINTERFACE_PARCLASS
    //{ 0x811FC6A5, 0xF728, 0x11D0, { 0xA5, 0x37, 0x00, 0x00, 0xF8, 0x75, 0x3E, 0xD1 } }
    };

    //注册插拔事件
    HDEVNOTIFY hDevNotify;
    DEV_BROADCAST_DEVICEINTERFACE NotifacationFiler;
    ZeroMemory(&NotifacationFiler,sizeof(DEV_BROADCAST_DEVICEINTERFACE));
    NotifacationFiler.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotifacationFiler.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

    for(int i=0;i<sizeof(GUID_DEVINTERFACE_LIST)/sizeof(GUID);i++)
    {
        NotifacationFiler.dbcc_classguid = GUID_DEVINTERFACE_LIST[i];//GetCurrentUSBGUID();//m_usb->GetDriverGUID();

        hDevNotify = RegisterDeviceNotification((HANDLE)this->winId(),&NotifacationFiler,DEVICE_NOTIFY_WINDOW_HANDLE);
        if(!hDevNotify)
        {
            DWORD Err = GetLastError();
            //qDebug() << "注册失败" <<endl;
        }
        //else
    }
#endif
    on_btnOpen_clicked(); // todo
}


void MainWindow::initStatusBar()
{
    // 状态栏分别为：
    // 提示信息（可多个）
    // RX、TX
    // 版本信息（或版权声明）
    // 退出图标
    ui->statusbar->setMinimumHeight(22);
    //ui->statusbar->setStyleSheet(QString("QStatusBar::item{border: 0px}")); // 不显示边框
    ui->statusbar->setSizeGripEnabled(false);//去掉状态栏右下角的三角

    m_stsDebugInfo = new QLabel();
    m_stsRx = new QLabel();
    m_stsTx = new QLabel();
    m_stsResetCnt = new QLabel();
    m_stsCopyright = new QLabel();
    m_stsExit = new QLabel();

    m_stsDebugInfo->setMinimumWidth(this->width()/2);
    ui->statusbar->addWidget(m_stsDebugInfo);

    m_stsRx->setMinimumWidth(64);
    ui->statusbar->addPermanentWidget(m_stsRx);
    m_stsRx->setText("RX: 0");

    m_stsTx->setMinimumWidth(64);
    ui->statusbar->addPermanentWidget(m_stsTx);
    m_stsTx->setText("TX: 0");

    m_stsResetCnt->installEventFilter(this);
    m_stsResetCnt->setFrameStyle(QFrame::Plain);
    m_stsResetCnt->setText("复位计数");
    m_stsResetCnt->setMinimumWidth(100);
    ui->statusbar->addPermanentWidget(m_stsResetCnt);

    printDebugInfo("欢迎使用串口调试助手");
    // 版权信息
    m_stsCopyright->setFrameStyle(QFrame::NoFrame);
    m_stsCopyright->setText(tr("  <a href=\"https://www.latelee.org\">技术主页</a>  "));
    m_stsCopyright->setOpenExternalLinks(true);
    ui->statusbar->addPermanentWidget(m_stsCopyright);

    // 退出图标
    m_stsExit->installEventFilter(this); // 安装事件过滤，以便获取其单击事件
    m_stsExit->setToolTip("Exit App");
    m_stsExit->setMinimumWidth(32);
    // 贴图
    QPixmap exitIcon(":/images/exit.png");
    m_stsExit->setPixmap(exitIcon);
    ui->statusbar->addPermanentWidget(m_stsExit);

    connect(this, &MainWindow::sig_exit, qApp, &QApplication::quit); // 直接关联到全局的退出槽
}

void MainWindow::printDebugInfo(const char* str)
{
    QString tmp = str;
    m_stsDebugInfo->setText(tmp);
}

void MainWindow::showMessage(const char* str)
{
    QString tmp = str;
    ui->statusbar->showMessage(tmp, 500);
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    //qDebug() << "Timer ID:" << event->timerId();
    sendData();
}

bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    MSG* msg = reinterpret_cast<MSG*>(message);
    int msgType = msg->message;
    if(msgType==WM_DEVICECHANGE) // 设备插入事件
    {
        //qDebug() << "Event DEVICECHANGE Happend" << endl;
        PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)msg->lParam;
        switch (msg->wParam) {
        case DBT_DEVICEARRIVAL:
            if(lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)
            {
                PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
                if(lpdbv->dbcv_flags == 0) //插入u盘
                {
                }
            }
            if(lpdb->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
            {
                //PDEV_BROADCAST_DEVICEINTERFACE pDevInf  = (PDEV_BROADCAST_DEVICEINTERFACE)lpdb;
                //QString strname = QString::fromWCharArray(pDevInf->dbcc_name,pDevInf->dbcc_size);
                //qDebug() << "arrive" + strname;
                showMessage("USB device arrive");
                emit sig_deviceChanged(1);
            }
            break;
        case DBT_DEVICEREMOVECOMPLETE:  // 设备移除事件
            if(lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)
            {
                PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
                if(lpdbv->dbcv_flags == 0)
                {
                }
            }
            if(lpdb->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
            {
                //PDEV_BROADCAST_DEVICEINTERFACE pDevInf  = (PDEV_BROADCAST_DEVICEINTERFACE)lpdb;
                //QString strname = QString::fromWCharArray(pDevInf->dbcc_name,pDevInf->dbcc_size);
                showMessage("USB device removed");
                emit sig_deviceChanged(0);
            }
            break;
        }
    }
    return false;
}

void MainWindow::sendData()
{
    if (!serial.isOpen())
    {
        showMessage("serial port not opened.");
        return;
    }
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
    if (m_sendNewline == 1)
    {
        sendData.append(0x0d);
        sendData.append(0x0a);
    }
    //qDebug() << sendData;
    m_txCnt += sendData.size();
    m_stsTx->setText("TX: " + QString::number(m_txCnt));
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

    m_rxCnt += buffer.size();
    m_stsRx->setText("RX: " + QString::number(m_rxCnt));

    if (m_showTimestamp)
    {
        QDateTime dateTime(QDateTime::currentDateTime());
        timeStr = "[" + dateTime.toString("yyyy-MM-dd HH:mm:ss.zzz") + "] ";
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
    if (buf[0] == 0xaa && buf[1] == 0x55)
    {

    }
}

void MainWindow::on_deviceChanged(int flag)
{
    if (flag == 1)
    {
        foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
        {
            if (-1 == ui->cbPortName->findText(info.portName()))
                ui->cbPortName->addItem(info.portName());
        }
    }
    else
    {
        serial.close();
        ui->btnOpen->setText(tr("打开串口"));
        ui->btnOpen->setIcon(QIcon(":images/notopened.ico"));
    }
}

void MainWindow::on_btnOpen_clicked()
{
    if(ui->btnOpen->text()==QString("打开串口"))
    {
        // 串口设备
        on_cbPortName_currentTextChanged(ui->cbPortName->currentText());
        // 波特率（注：QtSerial支持的枚举不多，但设置了其它值也行）
        on_cbBaudrate_currentTextChanged(ui->cbBaudrate->currentText());
        //设置数据位
        on_cbDatabit_currentTextChanged(ui->cbDatabit->currentText());
        //设置停止位
        on_cbStopbit_currentIndexChanged(ui->cbStopbit->currentIndex());
        //设置奇偶校验
        on_cbParity_currentIndexChanged(ui->cbParity->currentIndex());
        //设置流控制
        on_cbFlow_currentIndexChanged(ui->cbFlow->currentIndex());

        // 打开串口
        if(!serial.open(QIODevice::ReadWrite) && !serial.isOpen())
        {
            //QMessageBox::about(NULL, tr("info"), tr("open port failed."));
            showMessage("open port failed.\n");
            return;
        }

        showMessage("port opened.");
        ui->btnOpen->setText(tr("关闭串口"));
        ui->btnOpen->setIcon(QIcon(":images/opened.ico"));
    }
    else
    {
        serial.close();

        showMessage("port closed.");
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
            ui->btnSend->setText("发送");
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

void MainWindow::on_chSendNewline_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
    {
        m_sendNewline = 1;
    }
    else if (arg1 == Qt::Unchecked)
    {
        m_sendNewline = 0;
    }
}
