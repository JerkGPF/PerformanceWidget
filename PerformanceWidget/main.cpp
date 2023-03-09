#include "PerformanceWidget.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PerformanceWidget w;
    w.show();
    return a.exec();
}
