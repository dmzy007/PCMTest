#include "selectnetwork.h"
#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    SelectNetWork w;
    w.show();
    return a.exec();
}
