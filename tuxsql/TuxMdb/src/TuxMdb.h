/***************************************************************************
 *  Copyright (C) 2006 http://www.foxinfo.fr                               *
 *  Author : Stephane JEANNE    stephane.jeanne@gmail.com                  *
 *           Stephane LEICHT    stephane.leicht@foxinfo.fr                 *
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

#ifndef _TUXMDB_H
#define _TUXMDB_H

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_ERR_MSG_LEN 255
#define MIN_SAMPLE 10
#define MODBUS_SOCK_TIMEOUT 10

#include <sutil/MySQL.h>
#include <sutil/ChainList.h>

/************************** Error codes ************************************/

#define Tux_Error	100

#define T_Success 0
#define T_Error -1
#define T_DBError -2

typedef char D_Definition[100];
typedef char D_Path[255];
typedef char D_Tag[30];

typedef struct _PLC{
								D_Tag PlcName;
								char PlcPath[20];
								int socket;
								int DeviceId;
								int Next_Time;
								LISTE Tags;
								LISTE Packets;
							} PLC;

typedef struct _TAG{
								D_Tag TagName;
								char Address[20];
								int DataType;
								short int Time_Sample;
								time_t Time_Value;
								int Index;
							} TAG;

typedef struct _PACKET{
								char BaseAddress[20];
								int NumElt;
								short int Time_Sample;
								int Time_Value;
								LISTE Tags;
								} PACKET;

//typedef enum _Tux_Error_type{Tux_Internal_Error=100,Tux_Sys_Error,Tux_Cip_Error} Tux_Error_type;

/******************* Global Var ************************************/

extern unsigned int Tux_errno;
extern unsigned int Tux_ext_errno;
extern int Tux_err_type;
extern char Tux_err_msg[MAX_ERR_MSG_LEN+1];

extern LISTE PLCs;
//extern LISTE SOCKETs;

/******************* Functions declaration **************************/

char *TuxGetInternalErrMsg(unsigned int ErrorCode);
char *TuxGetErrMsg(int s_err_type,unsigned int s_errno,unsigned int ext_errno);

#define TUXERROR(type,no,ext_no) {Tux_err_type=type;Tux_errno=no;Tux_ext_errno=ext_no;memcpy(&Tux_err_msg,GetErrMsg(Tux_err_type,Tux_errno,Tux_ext_errno),MAX_ERR_MSG_LEN);}

int _GetPlc(MYSQL *db,LISTE *plcs,char *plcname);
#define GetPlc(plcs,plcname) _GetPlc(&Default_Db, plcs,plcname)

int _GetPlcList(MYSQL *db,LISTE *plcs,LISTE *plclistname);
#define GetPlcList(plcs,plclistname) _GetPlcList(&Default_Db,plcs,plclistname)

int _GetTag(MYSQL *db,PLC *plc);
#define GetTag(plc) _GetTag(&Default_Db, plc)

int _UpdateTag(MYSQL *db,TAG *tag,double value);
#define UpdateTag(tag,value) _UpdateTag(&Default_Db,tag,value)

#ifdef __cplusplus
}
#endif

#endif /* _TUXMDB_H */
