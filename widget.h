#ifndef WIDGET_H
#define WIDGET_H

#include <QDebug>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QTimerEvent>
#include <QUdpSocket>
#include <QWidget>

typedef enum
{
    AlarmIDLE = 0,
    AlarmCall,
    ALarmCom,
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

    void CheckAlarm(void);
public slots:
    void AlarmProcess();
    void RevData();

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
};
#endif  // WIDGET_H
