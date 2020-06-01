#include "WIADemo.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    WIADemo w;
    w.show();
    return a.exec();
}
