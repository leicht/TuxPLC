#include <QtGui/QApplication>
#include "mainwindow.h"
#include "lib/QPlc.h"
#include "lib/QPlcTag.h"
#include <tuxeip/TuxEip.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    
    return a.exec();
}
