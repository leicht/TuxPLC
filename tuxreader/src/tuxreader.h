/***************************************************************************
 *  Copyright (C) 2006 http://www.foxinfo.fr                               *
 *  Author : Stephane JEANNE    stephane.jeanne@gmail.com                  *
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

#ifndef _TUXREADER_H
#define _TUXREADER_H

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_LISTE_SIZE 50
#define MAX_PLC_NUMBERS 20
#define MAX_TAG_NUMBERS 50
#define MAX_ERR_MSG_LEN 255
#define MIN_SAMPLE 5
#define MAX_SAMPLE 20000 // timeout connection
#define ScanRate 5
#define uScanRate 500000
#define MAXBUFFERSIZE 50

#define FALSE 0
#define TRUE 1
	
/************************** Error codes ************************************/

#define Tux_Error	100
#define SUCCESS 0
#define ERROR -1	
#define T_Success 0
#define T_Error -1
	
typedef char D_Definition[100];
typedef char D_Path[255];
typedef char D_Tag[30];

typedef struct _BUFFER
	{	int size;
		char data[MAXBUFFERSIZE];
	} BUFFER;
	
typedef struct _CLIENT
	{	int FD;
		BUFFER InBuffer;
		char Reply[MAXBUFFERSIZE];
	} CLIENT;

typedef struct _PLC
	{	D_Tag PlcName;
		char PlcPath[50];
		Plc_Type PlcType;
		int NetWork;
		int Node;
		int References;
		Eip_Session *Session;
		Eip_Connection *Connection;
	} PLC;

typedef struct _TAG
	{	D_Tag TagName;
		PLC *Plc;
		//char Address[40];
		//int DataType;
		//short int Time_Sample;
		double Value;
		time_t Time_Value;
	} TAG;

//typedef enum _Tux_Error_type{Tux_Internal_Error=100,Tux_Sys_Error,Tux_Cip_Error} Tux_Error_type;

/******************* Global Var ************************************/

extern unsigned int Tux_errno;
extern unsigned int Tux_ext_errno;
extern int Tux_err_type;
extern char Tux_err_msg[MAX_ERR_MSG_LEN+1];

extern char tempbuf[MAXBUFFERSIZE];
extern LISTE PLCs;
extern LISTE TAGs;
extern LISTE CLIENTs;
extern LISTE SESSIONs;
extern LISTE CONNECTIONs;

/******************* Functions declaration **************************/

char *TuxGetInternalErrMsg(unsigned int ErrorCode);
char *TuxGetErrMsg(int s_err_type,unsigned int s_errno,unsigned int ext_errno);

#define TUXERROR(type,no,ext_no) {Tux_err_type=type;Tux_errno=no;Tux_ext_errno=ext_no;memcpy(&Tux_err_msg,GetErrMsg(Tux_err_type,Tux_errno,Tux_ext_errno),MAX_ERR_MSG_LEN);}

int GetPlc(LISTE *PLC_List,char *Aliasfile);
void CloseList(LISTE *List);

int SetCoe(int fd);

void SigHand(int num);
void Log(int level,char *format,...);
int mainprog(void);
//int Logger(PLC *PLCs,int Plc_count,TAG *TAGs,int Tag_count);
int GetIP(char *path,char *serveur);
int ParsePath(char *strpath,char Ip[],char Path[]);
int ParseRequest(char *Alias,char *Tag,char *writevalue, char *requete);

void Affiche(int fd,char *listname);
void Affiche_TAGs(int fd);
void Affiche_PLCs(int fd);
void Affiche_Connections(int fd);
void Affiche_Sessions(int fd);
void Affiche_Clients(int fd);

int OpenServerSock(int portnum);
int lire_client(CLIENT *client);
int Reply(int fd,char *format,...);

int BuildSession(PLC *plc);
int BuildConnection(PLC *plc);
int BuildConnections(LISTE *PLC_List);
int DisconnectPlc(PLC *plc);

Eip_Session *FindSession(char *Ip,LISTE *PLC_List);
Eip_Connection *FindConnection(char *Path,LISTE *PLC_List);
CLIENT *FindClient(int fd,LISTE *CLIENT_List);
PLC *FindPLC(char *PlcName,LISTE *PLC_List);
TAG *FindTag(char *TagName,char *PlcName,LISTE *TAG_List);
int ReadTag(TAG *tag);

void Traite(CLIENT *client);

int KillConnection(Eip_Connection *connection);
int KillSession(Eip_Session *session);
void KillConnections(void);
int Gardian(char *argv[]);

int GetSerial(void);

int CheckSession(PLC *plc);
int CheckConnection(PLC *plc);

#ifdef __cplusplus
}
#endif

#endif
