#include "widget.h"
#include "ui_widget.h"

const QString RedLabel   = "min-width: 30px; min-height: 30px;max-width:30px; max-height: 30px;border-radius: 15px;  border:1px solid black;background:red";
const QString GreenLabel = "min-width: 30px; min-height: 30px;max-width:30px; max-height: 30px;border-radius: 15px;  border:1px solid black;background:green";

const QString RedButton   = "background:red";
const QString GreenButton = "background:green";
const QString OrginButton = "";

const QHostAddress AlarmAddress = QHostAddress("230.10.10.14");
const int          AlarmPort    = 20010;
const QHostAddress DCUAddress   = QHostAddress("230.10.10.12");
const int          DCUPort      = 20010;

Widget::Widget(QHostAddress IP, QWidget *parent) : QWidget(parent), ui(new Ui::Widget), ip(IP)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/images/ATV.ico"));

    for (auto i = 0; i < 16; i++)
    {
        AlarmLBList.append(findChild<QLabel *>(QString("AlarmLB1_%1").arg(i + 1)));
        AlarmLBList.at(i)->setStyleSheet(RedLabel);

        AlarmPBList.append(findChild<QPushButton *>(QString("AlarmPB1_%1").arg(i + 1)));

        AlarmStatus.insert(i, AlarmIDLE);
        connect(AlarmPBList.at(i), &QPushButton::clicked, this, &Widget::AlarmProcess);
    }

    SUdpSocket = new QUdpSocket(this);
    qDebug() << "SUdpSocket = " << SUdpSocket->bind(ip, 20010, QUdpSocket::ShareAddress);
    SUdpSocket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 0);

    RCmdUdpSocket = new QUdpSocket(this);
    RCmdUdpSocket->bind(AlarmAddress, AlarmPort, QUdpSocket::ShareAddress);
    if (RCmdUdpSocket->joinMulticastGroup(AlarmAddress))
    {
        connect(RCmdUdpSocket, SIGNAL(readyRead()), this, SLOT(RecvData()));
    }

    //    id = startTimer(1000);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == id)
    {
        CheckAlarm();
    }
}

void Widget::AlarmProcess()
{
    int index  = sender()->objectName().split('_').at(1).toInt() - 1;
    int locate = ((index / 4) + 1) * 16 + (index % 4) + 1;  //报警位置

    if (AlarmStatus[index] == AlarmIDLE)
    {
        AlarmStatus[index] = AlarmCall;
        AlarmPBList.at(index)->setStyleSheet(RedButton);
    }
}

void Widget::RevData()
{
    int        size;
    QByteArray Data;
    if (RCmdUdpSocket->hasPendingDatagrams())
    {
        size = RCmdUdpSocket->pendingDatagramSize();
        Data.resize(size);

        RCmdUdpSocket->readDatagram(Data.data(), size);
    }
}

void Widget::CheckAlarm(void)
{
    for (int i = 0; i < 16; i++)
    {
        if (AlarmStatus[i] == AlarmCall)
        {
            static char cnt = 0;

            char Data[30] = {0};

            Data[0]  = 0x7E;
            Data[1]  = cnt;
            Data[3]  = ((i / 4) + 1) * 16 + (i % 4) + 1;
            Data[14] = 0x03;  //紧急对讲请求

            Data[28] = 0x00;
            Data[29] = 0x0D;

            cnt++;

            SUdpSocket->writeDatagram(Data, 30, DCUAddress, DCUPort);
        }
    }
}
