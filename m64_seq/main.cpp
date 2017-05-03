#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    if(argc > 1) {
        w.set_simulate(true);
        w.setGeometry(400, 200, 800, 480);
        w.show();

    } else w.showFullScreen();

    return a.exec();
}
