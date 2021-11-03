#ifndef PCMFORM_H
#define PCMFORM_H

#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <QAudioOutput>
#include <QDebug>
#include <QNetworkInterface>
#include <QTime>
#include <QUdpSocket>
#include <QWidget>

namespace Ui
{
class PCMForm;
}

class PCMForm : public QWidget
{
    Q_OBJECT

public:
    explicit PCMForm(QPair<QNetworkInterface, QHostAddress> ip, QWidget *parent = nullptr);
    ~PCMForm();

    void OutputDeviceBoxInit(int index);
    void InputDeviceBoxInit(int index);

private slots:
    void on_OutputAddress_currentIndexChanged(const QString &arg1);

    void on_OutputPortBox_currentIndexChanged(const QString &arg1);

    void on_OutputSampleRateBox_currentIndexChanged(const QString &arg1);

    void on_OutputSampleSizesBox_currentIndexChanged(const QString &arg1);

    void on_OutputChannelsBox_currentIndexChanged(const QString &arg1);

    void on_OutputCodecsBox_currentIndexChanged(const QString &arg1);

    void on_InputAddress_currentIndexChanged(const QString &arg1);

    void on_InputPortBox_currentIndexChanged(const QString &arg1);

    void on_InputSampleRateBox_currentIndexChanged(const QString &arg1);

    void on_InputSampleSizesBox_currentIndexChanged(const QString &arg1);

    void on_InputChannelsBox_currentIndexChanged(const QString &arg1);

    void on_InputCodecsBox_currentIndexChanged(const QString &arg1);

    void on_PlaybackPB_clicked();

    void on_ClearOutputTextPB_clicked();

    void on_RecodePB_clicked();

    void on_ClearInputTextPB_clicked();

private:
    Ui::PCMForm *ui;

    QPair<QNetworkInterface, QHostAddress> IP;

    bool        OutputUdpStatus, InputUdpStatus;
    QUdpSocket *OutputUdpSocket, *InputUdpSocket;

    QByteArray                   OutputData, InputData;
    QIODevice *                  OutputDevice, *InputDevice;
    QAudioDeviceInfo             OutputDeviceInfo, InputDeviceInfo;
    QAudioFormat                 OutputSettings, InputSettings;
    QHostAddress                 OutputAddress, InputAddress;
    int                          OutputPort, InputPort;
    QScopedPointer<QAudioOutput> AudioOutput;
    QScopedPointer<QAudioInput>  AudioInput;
    bool                         AudioOutputStatus = false;
    bool                         AudioInputStatus  = false;

    int   OutputCnt = 0, InputCnt = 0;
    QTime OutputTime, InputTime;
};

#endif  // PCMFORM_H
