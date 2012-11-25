/***************************************************************************
 *  Copyright (C) 2006                                                     *
 *  Author : Stephane JEANNE    stephane.jeanne@gmail.com                  *
 *           Stephane LEICHT    stephane.leicht@gmail.com                  *
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

#ifndef _TUXHISTO_H
#define _TUXHISTO_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <mysql/mysql.h>

#define MAX_LISTE_SIZE 500
#define MAX_PLC_NUMBERS 20
#define MAX_TAG_NUMBERS 50
#define MAX_ERR_MSG_LEN 255
#define MIN_SAMPLE 5

/************************** Error codes ************************************/

#define Tux_Error	100

#define T_Success 0
#define T_Error -1
#define T_DBError -2

typedef char D_Definition[100];
typedef char D_Path[255];
typedef char D_Tag[30];

typedef struct _TAG{
								//D_Tag TagName;
								int Id;
                                                                int Time_Sample;
								int Time_Refresh;
								int Hysteresis;
								double Value;
								time_t Time_Value;
                                                                time_t Insert_Time_Value;
								int exist;
							} TAG;

typedef struct _LISTE{
								int Count;
								void *Data[MAX_LISTE_SIZE];
								} LISTE;

//typedef enum _Tux_Error_type{Tux_Internal_Error=100,Tux_Sys_Error,Tux_Cip_Error} Tux_Error_type;

/******************* Global Var ************************************/

extern unsigned int Tux_errno;
extern unsigned int Tux_ext_errno;
extern int Tux_err_type;
extern char Tux_err_msg[MAX_ERR_MSG_LEN+1];

extern int Tag_count;
extern LISTE tags;

/******************* Functions declaration **************************/

char *TuxGetInternalErrMsg(unsigned int ErrorCode);
char *TuxGetErrMsg(int s_err_type,unsigned int s_errno,unsigned int ext_errno);

#define TUXERROR(type,no,ext_no) {Tux_err_type=type;Tux_errno=no;Tux_ext_errno=ext_no;memcpy(&Tux_err_msg,GetErrMsg(Tux_err_type,Tux_errno,Tux_ext_errno),MAX_ERR_MSG_LEN);}

int _GetTag(MYSQL *db,TAG tags[]);
#define GetTag(plc,tags) _GetTag(&Default_Db, plc,tags)

int _InsertHisto(MYSQL *db,TAG *tag);
#define InsertHisto(tag) _InsertHisto(&Default_Db,tag)

#ifdef __cplusplus
}
#endif

#endif /* _TUXHISTO_H */
