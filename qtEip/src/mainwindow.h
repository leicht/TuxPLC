#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "lib/QPlc.h"
#include "lib/QPlcTag.h"
#include <tuxeip/TuxEip.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;
    QPlc myPlc;
    QPlcTag myTag;
    QPlcTag myTag2;
    QPlcTag BP_IHM_Light_SS_Milieu;
    QPlcTag Temp_Ext;
};

#endif // MAINWINDOW_H
