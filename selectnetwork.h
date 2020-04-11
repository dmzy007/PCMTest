#ifndef SELECTNETWORK_H
#define SELECTNETWORK_H

#include <QDebug>
#include <QDialog>
#include <QNetworkInterface>
#include <QStringListModel>

#include "widget.h"

namespace Ui
{
class SelectNetWork;
}

class SelectNetWork : public QDialog
{
    Q_OBJECT

public:
    explicit SelectNetWork(QWidget *parent = nullptr);
    ~SelectNetWork();

    void ListNetWork(void);
private slots:
    void on_ListView_doubleClicked(const QModelIndex &index);

private:
    Ui::SelectNetWork *ui;
    Widget *           widget;

    QList<QHostAddress> IPList;
};

#endif  // SELECTNETWORK_H
