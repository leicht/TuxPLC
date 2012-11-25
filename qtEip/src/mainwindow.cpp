#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "lib/QPlc.h"
#include "lib/QPlcTag.h"
#include <tuxeip/TuxEip.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
    {
        ui->setupUi(this);

        myPlc.init("192.168.1.18","1,0",LGX,0,0);

        myTag.init(myPlc,"intensite_A");
        myTag.setScan(true);
        connect( &myTag, SIGNAL( valueChanged(double) ), ui->label1, SLOT( setNum(double) ) );

        Temp_Ext.init(myPlc,"Temp_Ext");
        Temp_Ext.setDuration(1000);
        Temp_Ext.setScan(true);
        connect( &Temp_Ext, SIGNAL( valueChanged(double) ), ui->lcdNumber_temperature, SLOT( display(double) ) );

        myTag2.init(myPlc,"Autorisation_IHM_Plancher");
        myTag2.setDuration(1500);
        myTag2.setScan(true);
        connect( &myTag2, SIGNAL( valueChanged(bool) ), ui->checkBox_Plancher, SLOT( setChecked(bool) ) );
        connect( ui->checkBox_Plancher, SIGNAL( clicked(bool) ), &myTag2, SLOT( setValue(bool))) ;

        BP_IHM_Light_SS_Milieu.init(myPlc,"BP_IHM_Light_SS_Milieu");
        BP_IHM_Light_SS_Milieu.setScan(true);
        connect( ui->radioButton_light_SS, SIGNAL( clicked(bool) ), &BP_IHM_Light_SS_Milieu, SLOT( setOnOff())) ;
    }

MainWindow::~MainWindow()
{
    delete ui;
}

