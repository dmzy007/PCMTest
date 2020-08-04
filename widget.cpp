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
const QHostAddress DCU1AudioAddress  = QHostAddress("230.10.10.24");
const QHostAddress DCU2AudioAddress  = QHostAddress("230.10.10.25");
const int          DCUAudioPort      = 20060;

Widget::Widget(QHostAddress IP, QWidget *parent) : QWidget(parent), ui(new Ui::Widget), ip(IP)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/images/ATV.ico"));

    for (auto i = 0; i < 16; i++)
    {
        AlarmLBList.append(findChild<QLabel *>(QString("AlarmLB1_%1").arg(i + 1)));
        AlarmLBList.at(i)->setStyleSheet(RedLabel);

        AlarmPBList.append(findChild<QPushButton *>(QString("AlarmPB1_%1").arg(i + 1)));

        AlarmStatus.insert(i, IDLE);
        connect(AlarmPBList.at(i), &QPushButton::clicked, this, &Widget::AlarmProcess);
    }

    SoundCardInit();
    id = startTimer(1000);

    qDebug() << ip;
    SUdpSocket = new QUdpSocket(this);
    SUdpSocket->bind(ip, 20000, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    SUdpSocket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 1);

    RAlarmUdpSocket = new QUdpSocket(this);
    RAlarmUdpSocket->bind(QHostAddress::AnyIPv4, AlarmPort, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    if (RAlarmUdpSocket->joinMulticastGroup(AlarmAddress))
    {
        qDebug() << "RAlarmUdpSocket    绑定成功";
        connect(RAlarmUdpSocket, SIGNAL(readyRead()), this, SLOT(RevAlarmData()));
    }

    RAlarmAudioUdpSocket = new QUdpSocket(this);
    RAlarmAudioUdpSocket->bind(QHostAddress::AnyIPv4, AlarmAudioPort, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    if (RAlarmAudioUdpSocket->joinMulticastGroup(AlarmAudioAddress))
    {
        qDebug() << "RAlarmAudioUdpSocket   绑定成功";
        connect(RAlarmAudioUdpSocket, SIGNAL(readyRead()), this, SLOT(RevAlarmAudioData()));
    }

    DCUUdpSocket = new QUdpSocket(this);
    DCUUdpSocket->bind(ip, DCUPort, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    DCUUdpSocket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 0);
    if (DCUUdpSocket->joinMulticastGroup(DCUAddress))
    {
        qDebug() << "DCUUdpSocket   绑定成功";
        connect(DCUUdpSocket, SIGNAL(readyRead()), this, SLOT(RevDCUData()));
    }

    DCUAudioUdpSocket = new QUdpSocket(this);
    DCUAudioUdpSocket->bind(QHostAddress::AnyIPv4, DCUAudioPort, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    DCUAudioUdpSocket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 0);

    if (ui->comboBox->currentIndex() == 0)
    {
        if (DCUAudioUdpSocket->joinMulticastGroup(DCU1AudioAddress))
        {
            qDebug() << "DCU1AudioAddress = " << DCU1AudioAddress;
            connect(DCUAudioUdpSocket, SIGNAL(readyRead()), this, SLOT(RevDCUAudioData()));
        }
    }
    else if (ui->comboBox->currentIndex() == 1)
    {
        if (DCUAudioUdpSocket->joinMulticastGroup(DCU2AudioAddress))
        {
            qDebug() << "DCU2AudioAddress = " << DCU2AudioAddress;
            connect(DCUAudioUdpSocket, SIGNAL(readyRead()), this, SLOT(RevDCUAudioData()));
        }
    }

    connect(ui->InterComPB, SIGNAL(clicked()), this, SLOT(InterComProcess()));
    connect(ui->comboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(SetLocal(QString)));

    DriverStatus.second = IDLE;
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
    int index = sender()->objectName().split('_').at(1).toInt() - 1;
    //    int locate = ((index / 4) + 1) * 16 + (index % 4) + 1;  //报警位置

    if (AlarmStatus[index] == IDLE)
    {
        AlarmStatus[index] = Call;
        AlarmPBList.at(index)->setStyleSheet(RedButton);
    }
    else if (AlarmStatus[index] == Call)
    {
        AlarmStatus[index] = Hang;
        AlarmPBList.at(index)->setStyleSheet(OrginButton);
    }
    else if (AlarmStatus[index] == Com)
    {
        AlarmStatus[index] = Hang;
        AlarmPBList.at(index)->setStyleSheet(OrginButton);

        AudioInput->stop();
        AudioOutput->stop();
    }
}

void Widget::RevAlarmData()
{
    int        size;
    QByteArray Data;

    if (RAlarmUdpSocket->hasPendingDatagrams())
    {
        size = RAlarmUdpSocket->pendingDatagramSize();
        Data.resize(size);

        RAlarmUdpSocket->readDatagram(Data.data(), size);

        if (Data.at(0) == 0x7B)
        {
            if (Data.at(14) == 0x02)
            {
                int index = ((unsigned char)Data.at(16) / 16 - 1) * 2 + ((unsigned char)Data.at(16) % 16 - 1);

                AlarmStatus[index] = Com;
                AlarmIndex         = index;
                AlarmPBList.at(index)->setStyleSheet(GreenButton);

                AudioInputDevice  = AudioInput->start();
                AudioOutputDevice = AudioOutput->start();
                connect(AudioInputDevice, SIGNAL(readyRead()), this, SLOT(SendAudioData()));
            }
            else if (Data.at(14) == 0x04)
            {
                int index = ((unsigned char)Data.at(16) / 16 - 1) * 2 + ((unsigned char)Data.at(16) % 16 - 1);

                AlarmStatus[index] = IDLE;
                AlarmPBList.at(index)->setStyleSheet(OrginButton);

                AudioInput->stop();
                AudioOutput->stop();
            }
        }
    }
}

void Widget::RevDCUData()
{
    int        size;
    QByteArray Data;

    if (DCUUdpSocket->hasPendingDatagrams())
    {
        size = DCUUdpSocket->pendingDatagramSize();
        Data.resize(size);

        DCUUdpSocket->readDatagram(Data.data(), size);

        if (Data.at(0) == 'S' && Data.at(1) == 'Y' && Data.at(2) == 'S' && Data.at(3) == 'S' && Data.at(4) == 'E' && Data.at(5) == 'T')
        {
            if (Data.at(10) == 0x07 && (Data.at(9) == 0x03))  //收到对讲命令
            {
                if (Data.at(11) == 0x01)  //呼叫
                {
                    DriverStatus.second = Call;
                    ui->InterComPB->setStyleSheet(RedButton);
                    qDebug() << "对端司机对讲呼叫";
                }
                else if (Data.at(11) == 0x02)
                {
                    DriverStatus.second = Com;
                    ui->InterComPB->setStyleSheet(GreenButton);

                    AudioInputDevice  = AudioInput->start();
                    AudioOutputDevice = AudioOutput->start();
                    connect(AudioInputDevice, SIGNAL(readyRead()), this, SLOT(SendAudioData()));

                    qDebug() << "对端司机对讲接通";
                }
                else if (Data.at(11) == 0x03)
                {
                    DriverStatus.first  = 0;
                    DriverStatus.second = IDLE;
                    ui->InterComPB->setStyleSheet(OrginButton);

                    AudioInput->stop();
                    AudioOutput->stop();

                    qDebug() << "对端司机对讲挂断";
                }
            }
        }
    }
}

void Widget::RevAlarmAudioData()
{
    int        size;
    QByteArray Data;
    if (RAlarmAudioUdpSocket->hasPendingDatagrams())
    {
        size = RAlarmAudioUdpSocket->pendingDatagramSize();
        Data.resize(size);

        RAlarmAudioUdpSocket->readDatagram(Data.data(), size);

        if (AlarmStatus[AlarmIndex] == Com)
        {
            AudioOutputDevice->write(Data);
        }
    }
}

void Widget::RevDCUAudioData()
{
    if (DCUAudioUdpSocket->hasPendingDatagrams())
    {
        int        size;
        QByteArray Data;

        size = DCUAudioUdpSocket->pendingDatagramSize();
        Data.resize(size);

        DCUAudioUdpSocket->readDatagram(Data.data(), size);
        if (DriverStatus.second == Com)
        {
            AudioOutputDevice->write(Data);
        }
    }
}

void Widget::CheckAlarm(void)
{
    for (int i = 0; i < 16; i++)
    {
        if (AlarmStatus[i] == Call)
        {
            static char cnt = 0;

            char Data[30] = {0};

            Data[0]  = 0x7C;
            Data[1]  = cnt;
            Data[3]  = ((i / 4) + 1) * 16 + (i % 4) + 1;
            Data[3]  = ((i / 2) + 1) * 16 + (i % 2) + 1;
            Data[4]  = 0x01;
            Data[14] = 0x03;  //紧急对讲请求

            Data[28] = 0x00;
            Data[29] = 0x0D;

            cnt++;

            SUdpSocket->writeDatagram(Data, 30, DCUAddress, DCUPort);

            Data[0]  = 0x7C;
            Data[1]  = cnt;
            Data[3]  = ((i / 4) + 1) * 16 + (i % 4) + 1;
            Data[3]  = ((i / 2) + 1) * 16 + (i % 2) + 1;
            Data[4]  = 0x06;
            Data[14] = 0x03;  //紧急对讲请求

            Data[28] = 0x00;
            Data[29] = 0x0D;

            cnt++;

            SUdpSocket->writeDatagram(Data, 30, DCUAddress, DCUPort);
        }
        else if (AlarmStatus[i] == Hang)
        {
            AlarmStatus[i]  = IDLE;
            static char cnt = 0;

            char Data[30] = {0};

            Data[0]  = 0x7C;
            Data[1]  = cnt;
            Data[3]  = ((i / 4) + 1) * 16 + (i % 4) + 1;
            Data[3]  = ((i / 2) + 1) * 16 + (i % 2) + 1;
            Data[4]  = 0x01;
            Data[14] = 0x04;  //紧急对讲挂起

            Data[28] = 0x00;
            Data[29] = 0x0D;

            cnt++;

            SUdpSocket->writeDatagram(Data, 30, DCUAddress, DCUPort);

            Data[0]  = 0x7C;
            Data[1]  = cnt;
            Data[3]  = ((i / 4) + 1) * 16 + (i % 4) + 1;
            Data[3]  = ((i / 2) + 1) * 16 + (i % 2) + 1;
            Data[4]  = 0x01;
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

    Format.setSampleRate(SampleRate);
    Format.setChannelCount(ChannelCount);
    Format.setSampleSize(SampleSize);
    Format.setCodec("audio/pcm");
    Format.setSampleType(QAudioFormat::SignedInt);
    Format.setByteOrder(QAudioFormat::LittleEndian);

    AudioOutput = new QAudioOutput(QAudioDeviceInfo::defaultOutputDevice(), Format, this);
    AudioInput  = new QAudioInput(QAudioDeviceInfo::defaultInputDevice(), Format, this);
}

void Widget::SetLocal(QString str)
{
    qDebug() << str;
    DCUAudioUdpSocket->leaveMulticastGroup(DCU1AudioAddress);
    DCUAudioUdpSocket->leaveMulticastGroup(DCU2AudioAddress);
    disconnect(DCUAudioUdpSocket, SIGNAL(readyRead()), this, SLOT(RevDCUAudioData()));

    if (ui->comboBox->currentIndex() == 0)
    {
        if (DCUAudioUdpSocket->joinMulticastGroup(DCU1AudioAddress))
        {
            connect(DCUAudioUdpSocket, SIGNAL(readyRead()), this, SLOT(RevDCUAudioData()));
        }
    }
    else if (ui->comboBox->currentIndex() == 1)
    {
        if (DCUAudioUdpSocket->joinMulticastGroup(DCU2AudioAddress))
        {
            connect(DCUAudioUdpSocket, SIGNAL(readyRead()), this, SLOT(RevDCUAudioData()));
        }
    }
}

void Widget::InterComProcess()
{
    qDebug() << "DriverStatus.first = " << DriverStatus.first;
    qDebug() << "DriverStatus.second = " << DriverStatus.second;

    if ((DriverStatus.first == 0) && (DriverStatus.second == IDLE))
    {
        DriverStatus.first  = 1;
        DriverStatus.second = Call;
        ui->InterComPB->setStyleSheet(RedButton);

        char Data[30] = {0};

        Data[0]  = 'S';
        Data[1]  = 'Y';
        Data[2]  = 'S';
        Data[3]  = 'S';
        Data[4]  = 'E';
        Data[5]  = 'T';
        Data[6]  = ui->comboBox->currentText().left(1).toInt();
        Data[9]  = 0x03;  //控制命令
        Data[10] = 0x07;  //对讲命令
        Data[11] = 0x01;  //呼叫

        Data[28] = 0x00;
        Data[29] = 0x0D;

        SUdpSocket->writeDatagram(Data, 30, DCUAddress, DCUPort);
        qDebug() << "本端司机对讲呼叫";
    }
    else if ((DriverStatus.first == 0) && (DriverStatus.second == Call))
    {
        DriverStatus.first  = 1;
        DriverStatus.second = Com;
        ui->InterComPB->setStyleSheet(GreenButton);

        AudioInputDevice  = AudioInput->start();
        AudioOutputDevice = AudioOutput->start();
        connect(AudioInputDevice, SIGNAL(readyRead()), this, SLOT(SendAudioData()));

        char Data[30] = {0};

        Data[0]  = 'S';
        Data[1]  = 'Y';
        Data[2]  = 'S';
        Data[3]  = 'S';
        Data[4]  = 'E';
        Data[5]  = 'T';
        Data[6]  = ui->comboBox->currentText().left(1).toInt();
        Data[9]  = 0x03;  //控制命令
        Data[10] = 0x07;  //对讲命令
        Data[11] = 0x02;  //接通

        Data[28] = 0x00;
        Data[29] = 0x0D;

        SUdpSocket->writeDatagram(Data, 30, DCUAddress, DCUPort);
        qDebug() << "本端司机对讲接通";
    }
    else if ((DriverStatus.first == 1) && (DriverStatus.second == Call))
    {
        DriverStatus.first  = 0;
        DriverStatus.second = IDLE;
        ui->InterComPB->setStyleSheet(OrginButton);

        char Data[30] = {0};

        Data[0]  = 'S';
        Data[1]  = 'Y';
        Data[2]  = 'S';
        Data[3]  = 'S';
        Data[4]  = 'E';
        Data[5]  = 'T';
        Data[6]  = ui->comboBox->currentText().left(1).toInt();
        Data[9]  = 0x03;  //控制命令
        Data[10] = 0x07;  //对讲命令
        Data[11] = 0x03;  //挂断

        Data[28] = 0x00;
        Data[29] = 0x0D;

        SUdpSocket->writeDatagram(Data, 30, DCUAddress, DCUPort);
        qDebug() << "本端司机对讲挂断";
    }
    else if ((DriverStatus.first == 1) && (DriverStatus.second == Com))
    {
        DriverStatus.first  = 0;
        DriverStatus.second = IDLE;
        ui->InterComPB->setStyleSheet(OrginButton);

        AudioOutput->stop();
        AudioInput->stop();

        char Data[30] = {0};

        Data[0]  = 'S';
        Data[1]  = 'Y';
        Data[2]  = 'S';
        Data[3]  = 'S';
        Data[4]  = 'E';
        Data[5]  = 'T';
        Data[6]  = ui->comboBox->currentText().left(1).toInt();
        Data[9]  = 0x03;  //控制命令
        Data[10] = 0x07;  //对讲命令
        Data[11] = 0x03;  //挂断

        Data[28] = 0x00;
        Data[29] = 0x0D;

        SUdpSocket->writeDatagram(Data, 30, DCUAddress, DCUPort);
        qDebug() << "本端司机对讲挂断";
    }
}

void Widget::SendAudioData()
{
    if (DriverStatus.second == Com)
    {
        AudioData.append(AudioInputDevice->readAll());

        while (AudioData.size() > 1024)
        {
            QByteArray datagram = AudioData.mid(0, 1024);
            AudioData.remove(0, 1024);

            SUdpSocket->writeDatagram(datagram, DCU1AudioAddress, DCUAudioPort);
        }
    }

    if (AlarmStatus[AlarmIndex] == Com)
    {
        AudioData.append(AudioInputDevice->readAll());

        while (AudioData.size() > 1024)
        {
            QByteArray datagram = AudioData.mid(0, 1024);
            AudioData.remove(0, 1024);

            SUdpSocket->writeDatagram(datagram, DCU1AudioAddress, DCUAudioPort);
        }
    }
}
