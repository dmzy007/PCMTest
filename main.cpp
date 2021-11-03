#include "selectnetwork.h"
#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qSetMessagePattern(
        "[%{time yyyy-MM-dd hh:mm:ss.zzz} "
        "%{if-debug}Debug%{endif}%{if-info}Info%{endif}%{if-warning}Warn%{endif}%{if-critical}Critical%{endif}%{if-fatal}Fatal%{endif}] "
        "%{file}:%{line} - %{message}");
    SelectNetWork w;
    w.show();
    return a.exec();
}
