/***************************************************************************
 *  Copyright (C) 2006 http://www.foxinfo.fr                               *
 *  Author : Stephane LEICHT    stephane.leicht@foxinfo.fr                 *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   * 
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        * 
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/

#ifndef _HISTOCHART_H
#define _HISTOCHART_H

#include "cgic.h"
#include "gd.h"
#include "gdfonts.h"
#include <syslog.h>
#include <mysql/mysql.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <sutil/MySQL.h>

//const
#define ERROR_MSG_LEN 512
#define PENNUMBER 8
#define PROG_NAME "TUXHISTO"
#define YOFFSET 20
#define XOFFSET 40
#define TITLE_BOTTOM 40

//Error
#define SUCCES 0
#define ERROR -1
#define EMPTYRESULT 1

typedef struct {
	char TagName[30];
	int ID;
	double I_MIN;
	double I_MAX;
	double O_MIN;
	double O_MAX;
	int penColor;
	char UNIT[20];
	int isDigital;
} TPen;

/* Define Debug for a console appli, else Daemon*/
//#define DEBUG
#ifdef DEBUG
	#define MyLog printf
#else
	#define MyLog(m) syslog(LOG_NOTICE,m) 
#endif

//Variables
extern gdImagePtr im;
extern int BackgroundColor;
extern int black, red, green, blue, white,	magenta, cyan,	yellow, orange, lightgray, midgray,	darkgray, midgreen,	darkmagenta, darkcyan;
extern int styleDotted[4], styleDashed[6];
extern TPen pen[8];
extern TPen onePen;
extern int TagNumber;
extern int timerange;
extern int lag;
extern	int height,width;
extern int Legend_Right;
extern time_t curtime, timeMin, timeMax;
extern int hChart, wChart;

extern MYSQL Default_Db;
extern unsigned int MysqlError;
extern char MysqlErrorMsg[ERROR_MSG_LEN];
extern MYSQL_RES *SqlResult;

/****************/
/* functions */
/****************/
int drawChart( );	//drawChart.c
int drawGrid(); //drawGrid.c
void print_error(char *errormsg) ;
int color();	//color.c
double calcScale(double inputValue, double I_MIN, double I_MAX, double O_MIN, double O_MAX); //common.c
int  GetSpan(double Vmin,double Vmax,double *Interval,double *Origine,int MinGrad,int MaxGrad);

//int GetErrorCode(MYSQL *db,char *msg);//MySQL.c
//int OpenDb(char *host,char *user,char *password,char *dbname);//MySQL.c
//void CloseDb();//MySQL.c
//int GetTag(int index);	//MySQL.c
int GetTag(int index); //common.c
#endif /* _HISTOCHART_H */
