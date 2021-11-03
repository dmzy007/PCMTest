#include "pcmform.h"
#include "ui_pcmform.h"

PCMForm::PCMForm(QPair<QNetworkInterface, QHostAddress> ip, QWidget *parent) : QWidget(parent), ui(new Ui::PCMForm), IP(ip)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/images/ATV.ico"));

    qDebug() << IP.second;

    InputUdpSocket = new QUdpSocket(this);

    OutputUdpSocket = new QUdpSocket(this);
    connect(OutputUdpSocket, &QUdpSocket::readyRead, this, [=]() {
        if (OutputUdpSocket->hasPendingDatagrams())
        {
            QByteArray datagram;
            int        size = OutputUdpSocket->pendingDatagramSize();
            datagram.resize(size);
            QHostAddress peerAddr;
            quint16      peerPort;

            OutputUdpSocket->readDatagram(datagram.data(), size, &peerAddr, &peerPort);
            OutputDevice->write(datagram);

            QString peer = "[From " + peerAddr.toString() + ":" + QString::number(peerPort) + "] " +
                           QString("间隔 %1ms Rev Data %2").arg(InputTime.msecsTo(QTime::currentTime()), 3, 10, QChar(' ')).arg(InputCnt);
            ui->OutputPlainTextEdit->appendPlainText(peer);
            OutputTime = QTime::currentTime();
        }
    });

    InputUdpStatus = InputUdpSocket->bind(IP.second, 20000, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    if (InputUdpStatus)
    {
        // SUdpSocket->setMulticastInterface(IP.first);
        InputUdpSocket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 1);
    }
    else
    {
        qDebug() << "输出组播绑定失败";
        ui->InputPlainTextEdit->appendPlainText("输出组播绑定失败");
    }

    for (auto &deviceInfo : QAudioDeviceInfo::availableDevices(QAudio::AudioOutput))
    {
        ui->OutputDeviceBox->addItem(deviceInfo.deviceName(), QVariant::fromValue(deviceInfo));
    }

    for (auto &deviceInfo : QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
    {
        ui->InputDeviceBox->addItem(deviceInfo.deviceName(), QVariant::fromValue(deviceInfo));
    }

    OutputDeviceBoxInit(ui->OutputDeviceBox->currentIndex());
    InputDeviceBoxInit(ui->InputDeviceBox->currentIndex());

    connect(ui->OutputDeviceBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PCMForm::OutputDeviceBoxInit);
    connect(ui->InputDeviceBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PCMForm::InputDeviceBoxInit);
}

PCMForm::~PCMForm()
{
    delete ui;
}

void PCMForm::OutputDeviceBoxInit(int index)
{
    OutputAddress.setAddress(ui->OutputAddress->currentText());
    OutputPort = ui->OutputPortBox->currentText().toInt();

    if (ui->OutputDeviceBox->count() && index <= ui->OutputDeviceBox->count())
    {
        OutputDeviceInfo = ui->OutputDeviceBox->itemData(index).value<QAudioDeviceInfo>();

        ui->OutputSampleRateBox->clear();
        QList<int> sampleRate = OutputDeviceInfo.supportedSampleRates();
        for (int i = 0; i < sampleRate.size(); ++i)
        {
            ui->OutputSampleRateBox->addItem(QString("%1").arg(sampleRate.at(i)));
        }
        OutputSettings.setSampleRate(sampleRate.at(0));

        ui->OutputSampleSizesBox->clear();
        QList<int> sampleSize = OutputDeviceInfo.supportedSampleSizes();
        for (int i = 0; i < sampleSize.size(); ++i)
        {
            ui->OutputSampleSizesBox->addItem(QString("%1").arg(sampleSize.at(i)));
        }
        OutputSettings.setSampleSize(sampleSize.at(0));

        ui->OutputChannelsBox->clear();
        QList<int> channel = OutputDeviceInfo.supportedChannelCounts();
        for (int i = 0; i < channel.size(); ++i)
        {
            ui->OutputChannelsBox->addItem(QString("%1").arg(channel.at(i)));
        }
        OutputSettings.setChannelCount(channel.at(0));

        ui->OutputCodecsBox->clear();
        QStringList codecs = OutputDeviceInfo.supportedCodecs();
        for (int i = 0; i < codecs.size(); ++i)
        {
            ui->OutputCodecsBox->addItem(QString("%1").arg(codecs.at(i)));
        }
        OutputSettings.setCodec(codecs.at(0));
        OutputSettings.setSampleType(QAudioFormat::SignedInt);
        OutputSettings.setByteOrder(QAudioFormat::LittleEndian);
    }
}

void PCMForm::InputDeviceBoxInit(int index)
{
    InputAddress.setAddress(ui->InputAddress->currentText());
    InputPort = ui->InputPortBox->currentText().toInt();

    if (ui->InputDeviceBox->count() && index <= ui->InputDeviceBox->count())
    {
        InputDeviceInfo = ui->InputDeviceBox->itemData(index).value<QAudioDeviceInfo>();

        ui->InputSampleRateBox->clear();
        QList<int> sampleRate = InputDeviceInfo.supportedSampleRates();
        for (int i = 0; i < sampleRate.size(); ++i)
        {
            ui->InputSampleRateBox->addItem(QString("%1").arg(sampleRate.at(i)));
        }
        InputSettings.setSampleRate(sampleRate.at(0));

        ui->InputSampleSizesBox->clear();
        QList<int> sampleSize = InputDeviceInfo.supportedSampleSizes();
        for (int i = 0; i < sampleSize.size(); ++i)
        {
            ui->InputSampleSizesBox->addItem(QString("%1").arg(sampleSize.at(i)));
        }
        InputSettings.setSampleSize(sampleSize.at(0));

        ui->InputChannelsBox->clear();
        QList<int> channel = InputDeviceInfo.supportedChannelCounts();
        for (int i = 0; i < channel.size(); ++i)
        {
            ui->InputChannelsBox->addItem(QString("%1").arg(channel.at(i)));
        }
        InputSettings.setChannelCount(channel.at(0));

        ui->InputCodecsBox->clear();
        QStringList codecs = InputDeviceInfo.supportedCodecs();
        for (int i = 0; i < codecs.size(); ++i)
        {
            ui->InputCodecsBox->addItem(QString("%1").arg(codecs.at(i)));
        }
        InputSettings.setCodec(codecs.at(0));
        InputSettings.setSampleType(QAudioFormat::SignedInt);
        InputSettings.setByteOrder(QAudioFormat::LittleEndian);
    }
}

void PCMForm::on_OutputAddress_currentIndexChanged(const QString &arg1)
{
    qDebug() << "OutputAddress = " << arg1;
    OutputAddress.setAddress(arg1);
}

void PCMForm::on_OutputPortBox_currentIndexChanged(const QString &arg1)
{
    qDebug() << "OutputPort = " << arg1;
    OutputPort = arg1.toInt();
}

void PCMForm::on_OutputSampleRateBox_currentIndexChanged(const QString &arg1)
{
    qDebug() << "OutputSampleRate = " << arg1;
    OutputSettings.setSampleRate(arg1.toInt());
}

void PCMForm::on_OutputSampleSizesBox_currentIndexChanged(const QString &arg1)
{
    qDebug() << "OutputSampleSize = " << arg1;
    OutputSettings.setSampleSize(arg1.toInt());
}

void PCMForm::on_OutputChannelsBox_currentIndexChanged(const QString &arg1)
{
    qDebug() << "OutputChannel = " << arg1;
    OutputSettings.setChannelCount(arg1.toInt());
}

void PCMForm::on_OutputCodecsBox_currentIndexChanged(const QString &arg1)
{
    qDebug() << "OutputCodecs = " << arg1;
    OutputSettings.setCodec(arg1);
}

void PCMForm::on_InputSampleRateBox_currentIndexChanged(const QString &arg1)
{
    qDebug() << "InputSampleRate = " << arg1;
    InputSettings.setSampleRate(arg1.toInt());
}

void PCMForm::on_InputSampleSizesBox_currentIndexChanged(const QString &arg1)
{
    qDebug() << "InputSampleSize = " << arg1;
    InputSettings.setSampleSize(arg1.toInt());
}

void PCMForm::on_InputChannelsBox_currentIndexChanged(const QString &arg1)
{
    qDebug() << "InputChannel = " << arg1;
    InputSettings.setChannelCount(arg1.toInt());
}

void PCMForm::on_InputCodecsBox_currentIndexChanged(const QString &arg1)
{
    qDebug() << "InputCodecs = " << arg1;
    InputSettings.setCodec(arg1);
}

void PCMForm::on_InputAddress_currentIndexChanged(const QString &arg1)
{
    qDebug() << "InputAddress = " << arg1;
    InputAddress.setAddress(arg1);
}

void PCMForm::on_InputPortBox_currentIndexChanged(const QString &arg1)
{
    qDebug() << "InputPort = " << arg1;
    InputPort = arg1.toInt();
}

void PCMForm::on_PlaybackPB_clicked()
{
    if (AudioOutputStatus)
    {
        AudioOutputStatus = false;

        ui->PlaybackPB->setText("开始播放");
        ui->OutputAddress->setEnabled(true);
        ui->OutputPortBox->setEnabled(true);
        ui->OutputDeviceBox->setEnabled(true);
        ui->OutputSampleRateBox->setEnabled(true);
        ui->OutputSampleSizesBox->setEnabled(true);
        ui->OutputChannelsBox->setEnabled(true);
        ui->OutputCodecsBox->setEnabled(true);

        OutputDevice->disconnect(this);
        AudioOutput->stop();
        OutputUdpSocket->leaveMulticastGroup(OutputAddress);
        OutputUdpSocket->abort();
    }
    else
    {
        if (OutputUdpSocket->bind(QHostAddress::AnyIPv4, OutputPort, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint))
        {
            qDebug() << "输入端口绑定成功";
            ui->OutputPlainTextEdit->appendPlainText("输入端口绑定成功");

            if (OutputUdpSocket->joinMulticastGroup(OutputAddress))
            {
                qDebug() << "输入组播绑定成功";
                ui->OutputPlainTextEdit->appendPlainText("输入组播绑定成功");

                AudioOutputStatus = true;

                ui->PlaybackPB->setText("停止播放");
                ui->OutputAddress->setEnabled(false);
                ui->OutputPortBox->setEnabled(false);
                ui->OutputDeviceBox->setEnabled(false);
                ui->OutputSampleRateBox->setEnabled(false);
                ui->OutputSampleSizesBox->setEnabled(false);
                ui->OutputChannelsBox->setEnabled(false);
                ui->OutputCodecsBox->setEnabled(false);

                AudioOutput.reset(new QAudioOutput(OutputDeviceInfo, OutputSettings, this));
                OutputDevice = AudioOutput->start();

                OutputTime = QTime::currentTime();
            }
            else
            {
                qDebug() << "输入组播绑定失败";
                ui->OutputPlainTextEdit->appendPlainText("输入组播绑定失败");
            }
        }
        else
        {
            qDebug() << "输入端口绑定失败";
            ui->OutputPlainTextEdit->appendPlainText("输入端口绑定失败");
        }
    }
}

void PCMForm::on_ClearOutputTextPB_clicked()
{
    ui->OutputPlainTextEdit->clear();
}

void PCMForm::on_RecodePB_clicked()
{
    if (AudioInputStatus)
    {
        AudioInputStatus = false;

        ui->RecodePB->setText("开始发送");
        ui->InputAddress->setEnabled(true);
        ui->InputPortBox->setEnabled(true);
        ui->InputDeviceBox->setEnabled(true);
        ui->InputSampleRateBox->setEnabled(true);
        ui->InputSampleSizesBox->setEnabled(true);
        ui->InputChannelsBox->setEnabled(true);
        ui->InputCodecsBox->setEnabled(true);

        InputDevice->disconnect(this);
        AudioInput->stop();
        InputCnt = 0;
    }
    else
    {
        AudioInputStatus = true;

        ui->RecodePB->setText("停止发送");
        ui->InputAddress->setEnabled(false);
        ui->InputPortBox->setEnabled(false);
        ui->InputDeviceBox->setEnabled(false);
        ui->InputSampleRateBox->setEnabled(false);
        ui->InputSampleSizesBox->setEnabled(false);
        ui->InputChannelsBox->setEnabled(false);
        ui->InputCodecsBox->setEnabled(false);

        AudioInput.reset(new QAudioInput(InputDeviceInfo, InputSettings, this));
        InputDevice = AudioInput->start();
        InputTime   = QTime::currentTime();

        connect(InputDevice, &QIODevice::readyRead, this, [=]() {
            InputData.append(InputDevice->readAll());
            while (InputData.size() > 1024)
            {
                InputCnt++;
                QByteArray datagram = InputData.mid(0, 1024);
                InputData.remove(0, 1024);
                InputUdpSocket->writeDatagram(datagram, InputAddress, InputPort);
                ui->InputPlainTextEdit->appendPlainText(
                    QString("间隔 %1ms Send Data %2").arg(InputTime.msecsTo(QTime::currentTime()), 3, 10, QChar(' ')).arg(InputCnt));
                InputTime = QTime::currentTime();
            }
        });
    }
}

void PCMForm::on_ClearInputTextPB_clicked()
{
    ui->InputPlainTextEdit->clear();
}
