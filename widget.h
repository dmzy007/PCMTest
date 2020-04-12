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
    AlarmIDLE = 0,
    AlarmCall,
    AlarmCom,
    AlarmHang
} AlarmStatusDef;

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
    void RevData();
    void RevAudioData();

protected:
    void timerEvent(QTimerEvent *);

private:
    Ui::Widget * ui;
    QHostAddress ip;

    QList<QLabel *>      AlarmLBList;
    QList<QPushButton *> AlarmPBList;

    QMap<int, AlarmStatusDef> AlarmStatus;

    QUdpSocket *RCmdUdpSocket, *RAudioUdpSocket, *SUdpSocket;
    int         id;

    QList<QAudioDeviceInfo> AudioOutputDeviceList;
    QList<QAudioDeviceInfo> AudioInputDeviceList;
    QAudioOutput *          AudioOutput;
    QAudioInput *           AudioInput;
    QIODevice *             AudioOutputDevice;
    QIODevice *             AudioInputDevice;
};
#endif  // WIDGET_H
