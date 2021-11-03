#include "selectnetwork.h"
#include "ui_selectnetwork.h"

SelectNetWork::SelectNetWork(QWidget *parent) : QDialog(parent), ui(new Ui::SelectNetWork)
{
    ui->setupUi(this);

    setWindowIcon(QIcon(":/images/ATV.ico"));
    ListNetWork();
}

SelectNetWork::~SelectNetWork()
{
    delete ui;
}

void SelectNetWork::ListNetWork(void)
{
    int i = 0, j = 0;

    QStringList                            NewWorkList;
    QPair<QNetworkInterface, QHostAddress> IP;
    QList<QNetworkInterface>               networkInterface = QNetworkInterface::allInterfaces();

    for (QList<QNetworkInterface>::const_iterator it = networkInterface.constBegin(); it != networkInterface.constEnd(); ++it)
    {
        //获取连接地址列表
        QList<QNetworkAddressEntry> addressEntriesList = (*it).addressEntries();
        for (QList<QNetworkAddressEntry>::const_iterator jIt = addressEntriesList.constBegin(); jIt != addressEntriesList.constEnd(); ++jIt)
        {
            if (!(*it).hardwareAddress().isNull())
            {
                if ((*jIt).ip().protocol() == QAbstractSocket::IPv4Protocol)
                {
                    QString str = (*it).humanReadableName() + " IPV4 " + (*jIt).ip().toString().split('%').at(0);
                    IP.first    = *it;
                    IP.second   = QHostAddress((*jIt).ip().toString().split('%').at(0));
                    IPList.append(IP);

                    NewWorkList.append(str);
                }
                else if ((*jIt).ip().protocol() == QAbstractSocket::IPv6Protocol)
                {
                    QString str = (*it).humanReadableName() + " IPV6 " + (*jIt).ip().toString().split('%').at(0);

                    IP.first  = *it;
                    IP.second = QHostAddress((*jIt).ip().toString().split('%').at(0));
                    IPList.append(IP);

                    NewWorkList.append(str);
                }

                j++;
            }
        }
        i++;
    }

    QStringListModel *Model = new QStringListModel(NewWorkList);
    ui->ListView->setModel(Model);
}

void SelectNetWork::on_ListView_doubleClicked(const QModelIndex &index)
{
    //    widget = new Widget(IPList.at(index.row()));
    Form = new PCMForm(IPList.at(index.row()));
    hide();
    Form->show();
}
