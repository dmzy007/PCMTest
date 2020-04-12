#include "widget.h"
#include "ui_widget.h"

const QString RedLabel   = "min-width: 30px; min-height: 30px;max-width:30px; max-height: 30px;border-radius: 15px;  border:1px solid black;background:red";
const QString GreenLabel = "min-width: 30px; min-height: 30px;max-width:30px; max-height: 30px;border-radius: 15px;  border:1px solid black;background:green";

const QString RedButton   = "background:red";
const QString GreenButton = "background:green";
const QString OrginButton = "";

const QHostAddress AlarmAddress      = QHostAddress("230.10.10.14");
const int          AlarmPort         = 20010;
const QHostAddress AlarmAudioAddress = QHostAddress("230.10.10.26");
const int          AlarmAudioPort    = 20060;
const QHostAddress DCUAddress        = QHostAddress("230.10.10.12");
const int          DCUPort           = 20010;

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

    RAudioUdpSocket = new QUdpSocket(this);
    RAudioUdpSocket->bind(AlarmAudioAddress, AlarmAudioPort, QUdpSocket::ShareAddress);
    if (RAudioUdpSocket->joinMulticastGroup(AlarmAudioAddress))
    {
        connect(RAudioUdpSocket, SIGNAL(readyRead()), this, SLOT(RecvAudioData()));
    }

    SoundCardInit();
    id = startTimer(1000);
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
    else if (AlarmStatus[index] == AlarmCall)
    {
        AlarmStatus[index] = AlarmHang;
        AlarmPBList.at(index)->setStyleSheet(OrginButton);
    }
    else if (AlarmStatus[index] == AlarmCom)
    {
        AlarmStatus[index] = AlarmHang;
        AlarmPBList.at(index)->setStyleSheet(OrginButton);

        AudioOutput->stop();
        delete AudioOutputDevice;
        AudioOutputDevice = nullptr;
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

        if (Data.at(14) == 0x02)
        {
            int index          = (Data.at(16) / 16 - 1) * 4 + (Data.at(16) % 16 - 1);
            AlarmStatus[index] = AlarmCom;
            AudioOutputDevice  = AudioOutput->start();
            AlarmPBList.at(index)->setStyleSheet(GreenButton);
        }
    }
}

void Widget::RevAudioData()
{
    int        size;
    QByteArray Data;
    if (RCmdUdpSocket->hasPendingDatagrams())
    {
        size = RCmdUdpSocket->pendingDatagramSize();
        Data.resize(size);

        RCmdUdpSocket->readDatagram(Data.data(), size);

        if (AudioOutputDevice != nullptr)
        {
            AudioOutputDevice->write(Data);
        }
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
        else if (AlarmStatus[i] == AlarmHang)
        {
            AlarmStatus[i]  = AlarmIDLE;
            static char cnt = 0;

            char Data[30] = {0};

            Data[0]  = 0x7E;
            Data[1]  = cnt;
            Data[3]  = ((i / 4) + 1) * 16 + (i % 4) + 1;
            Data[14] = 0x04;  //紧急对讲挂起

            Data[28] = 0x00;
            Data[29] = 0x0D;

            cnt++;

            SUdpSocket->writeDatagram(Data, 30, DCUAddress, DCUPort);
        }
    }
}

void Widget::SoundCardInit(void)
{
    AudioOutputDeviceList = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    AudioInputDeviceList  = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);

    for (int i = 0; i < AudioOutputDeviceList.size(); i++)
    {
        qDebug() << "OutPutDevice Name = " << AudioOutputDeviceList.at(i).deviceName();
    }
    for (int i = 0; i < AudioInputDeviceList.size(); i++)
    {
        qDebug() << "InPutDevice Name = " << AudioInputDeviceList.at(i).deviceName();
    }

    /*******************设置音频播放参数*****************/
    const int SampleRate   = 16000;
    const int ChannelCount = 1;
    const int SampleSize   = 16;

    QAudioFormat format;
    format.setSampleRate(SampleRate);
    format.setChannelCount(ChannelCount);
    format.setSampleSize(SampleSize);
    format.setCodec("audio/pcm");
    format.setSampleType(QAudioFormat::SignedInt);
    format.setByteOrder(QAudioFormat::LittleEndian);

    AudioOutput = new QAudioOutput(AudioOutputDeviceList.at(0), format, this);

    //    AudioOutputDevice = AudioOutput->start();
    //    Testid            = startTimer(16);
    //    qDebug() << "Size = " << AudioOutputDevice->write(Data);
    //    qDebug() << "Stop";
    //    AudioOutput->stop();

    //    AudioInput       = new QAudioInput(curDevice, format, this);  //打开设备
    //    AudioInputDevice = AudioInput->start();
    //    connect(AudioInputDevice, SIGNAL(readyRead()), SLOT(readMore()));
}
