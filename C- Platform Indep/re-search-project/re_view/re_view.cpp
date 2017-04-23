#include <QtGui/QApplication>
#include "rv_mainwin.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    rv_mainwin w;
    w.show();

    return a.exec();
}
