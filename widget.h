#ifndef WIDGET_H
#define WIDGET_H

#include <QAudioInput>
#include <QAudioOutput>
#include <QDebug>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QTimerEvent>
#include <QUdpSocket>
#include <QWidget>

#include <QFile>

typedef enum
{
    IDLE = 0,
    Call,
    Com,
    Hang
} InterComStatusDef;  //对讲状态

QT_BEGIN_NAMESPACE
namespace Ui
{
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QHostAddress IP, QWidget *parent = nullptr);
    ~Widget();

    void SoundCardInit(void);
    void CheckAlarm(void);

public slots:
    void AlarmProcess();
    void RevDCUData();
    void RevDCUAudioData();
    void RevAlarmData();
    void RevAlarmAudioData();
    void SetLocal(QString str);
    void InterComProcess();
    void SendAudioData();

protected:
    void timerEvent(QTimerEvent *);

private:
    Ui::Widget * ui;
    QHostAddress ip;

    QList<QLabel *>      AlarmLBList;
    QList<QPushButton *> AlarmPBList;

    QMap<int, InterComStatusDef>  AlarmStatus;
    int                           AlarmIndex;
    QPair<int, InterComStatusDef> DriverStatus;

    QUdpSocket *RAlarmUdpSocket, *RAlarmAudioUdpSocket, *SUdpSocket;
    QUdpSocket *DCUUdpSocket, *DCUAudioUdpSocket;
    int         id;

    QAudioFormat            Format;
    QByteArray              AudioData;
    QList<QAudioDeviceInfo> AudioOutputDeviceList;
    QList<QAudioDeviceInfo> AudioInputDeviceList;
    QAudioOutput *          AudioOutput;
    QAudioInput *           AudioInput;
    QIODevice *             AudioOutputDevice;
    QIODevice *             AudioInputDevice;
};
#endif  // WIDGET_H
