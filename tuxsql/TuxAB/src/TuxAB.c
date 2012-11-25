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
//#define _GNU_SOURCE // used for debugging purpose

#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <tuxeip/TuxEip.h>
#include <sutil/MySQL.h>
#include <sutil/ChainList.h>
#include "TuxAB.h"

#define space printf("\n");
#define LOG_TITLE "TuxOptim"
#define WAITING 60 // Gardian
#define MAX_ERR_BEFORE_CHECK 3

char DBHOST[20]={"localhost\0"};
char DB[20]={"histosql\0"};
char USER[20]={"histosql\0"};
char PASS[20]={"histosql\0"};
char PIDFILE[50]={"\0"};

#define flush(header) _CipFlushBuffer(header,24+header->Length);

/****************** functions declarations *************************/

void SigHand(int num);
int CreatePidFile(char *PidFile);
int DeletePidFile(char *PidFile);
void Log(int level,char *format,...);
int mainprog(void);
int Logger(LISTE *plcs);
int ListePlc(LISTE *plcs);
int GetIP(char *path,char *serveur);
int ParsePath(char *strpath,char Ip[],BYTE Path[]);

int BuildSession(PLC *plc);
int BuildConnection(PLC *plc);
int BuildConnections(LISTE *plcs);

Eip_Session *FindSession(char *Ip,LISTE *plcs);
Eip_Connection *FindConnection(char *Path,int Network,LISTE *plcs);
int KillConnection(Eip_Connection *connection);
int KillSession(Eip_Session *session);
void KillAll(void);
void FreeAll(LISTE *plcs);
int ReadTag(PLC *plc,TAG *tag);
int ReadPacket(PLC *plc,PACKET *packet);
int Gardian(void);

int GetSerial(void);
int CheckSession(PLC *plc);
int CheckConnection(PLC *plc);

int compare_plc(const void *Data1,const void *Data2);
void Optimise_Plc(PLC *plc);
void Optimise(PLC *plc);
void AddPacket(PACKET *packet,TAG *tag);

/******************* Global Var ************************************/

unsigned int Tux_errno;
unsigned int Tux_ext_errno;
int Tux_err_type;
char Tux_err_msg[MAX_ERR_MSG_LEN+1];

int DEAMON=0;
int TEST=0;
int DEFAULT_TIMEOUT=1000;
int WAIT_FOR_RECONNECT=60;
int debuglevel=LOG_WARNING;

jmp_buf jb;
int Terminated=0;
//int Restart=0;

int starttime=0;

LISTE PLCs;
LISTE SESSIONs;
LISTE CONNECTIONs;
LISTE CONTROLLERs;

int Plc_count,Tag_count;
int Serial=0;

/******************* Functions **************************************/

char *TuxGetInternalErrMsg(unsigned int ErrorCode)
{
	switch (ErrorCode){
		case Success:return("Success");
		case Error:return("Error");
		/*case :return("");
		case :return("");
		case :return("");
		case :return("");
		case :return("");	*/
		default :return("Reserved for future expansion");
	}
}

char *TuxGetErrMsg(int s_err_type,unsigned int s_errno,unsigned int ext_errno)
{
 switch (s_err_type){
  case Tux_Error:return(TuxGetInternalErrMsg(s_errno));
  default :return(CIPGetErrMsg(s_err_type,s_errno,ext_errno));
 }
}
/******************** DB Functions ******************/
int _GetPlcList(MYSQL *db,LISTE *plcs,LISTE *plclistname)
{
	int res=0;
	ELEMENT *elt=GetFirst(&CONTROLLERs);
	if (elt==NULL)
	{
		Log(LOG_DEBUG,"GetPlcList \n");
		res=_GetPlc(db,plcs,NULL);
	} else
	while (elt!=NULL)
	{
		Log(LOG_DEBUG,"GetPlcList : %s\n",elt->Data);
		res+=_GetPlc(db,plcs,elt->Data);
		elt=GetNext(&CONTROLLERs,elt);
	}
	return(res);
}
int _GetPlc(MYSQL *db,LISTE *plcs,char *plcname)
{
	char *sel_str;
	int res;
	MYSQL_ROW row;
	if ((plcname!=NULL)&&(strlen(plcname)>0))
		res=asprintf(&sel_str,"SELECT distinct p.* FROM PLC as p LEFT JOIN DEFINITION as d ON p.PLCNAME=\
			d.PLCNAME where p.PLC_PATH is not null and d.ADDRESS is not null and d.READING=1\
	and p.PLCNAME='%s'",plcname);
	else res=asprintf(&sel_str,"SELECT distinct p.* FROM PLC as p LEFT JOIN DEFINITION as d ON p.PLCNAME=\
		d.PLCNAME where p.PLC_ENABLE=1 and p.PLC_PATH is not null and d.ADDRESS is not null and d.READING=1");
	if (res<0)
	{
		Log(LOG_CRIT,"GetPLC : Unable to build query: %s\n",strerror(errno));
		exit(0);
	}
	res=mysql_real_query(db,sel_str,strlen(sel_str));
	free(sel_str);
	if (res)
	{
		_GetErrorCode(db,MysqlErrorMsg);
		return(-1);
	}
	SqlResult=mysql_store_result(db);
	if (SqlResult==NULL)
	{
		_GetErrorCode(db,MysqlErrorMsg);
		return(-1);
	}
	if (plcs==NULL)
	{
		res=mysql_num_rows(SqlResult);
		mysql_free_result(SqlResult);
		return(res);
	}
	while ((row = mysql_fetch_row(SqlResult)))
	{
		unsigned long *lengths;
		lengths = mysql_fetch_lengths(SqlResult);
		PLC *plc=malloc(sizeof(PLC));
		if (plc!=NULL)
		{
			memset(plc,0,sizeof(PLC));
			AddChListe(&PLCs,plc);
			snprintf(plc->PlcName,lengths[0]+1,"%s",row[0]);
			snprintf(plc->PlcPath,lengths[1]+1,"%s",row[1]);
			if(row[2]==NULL) plc->PlcType=Unknow;
				else
				if (!strncasecmp(row[2],"PLC",lengths[2])) plc->PlcType=PLC5;
					else
					{
						if (!strncasecmp(row[2],"SLC",lengths[2])) plc->PlcType=SLC500;
						else
						{
							if (!strncasecmp(row[2],"LGX",lengths[2])) plc->PlcType=LGX;
							else plc->PlcType=Unknow;
						}
					}
			if(row[3]==NULL) plc->NetWork=0;
				else
				if (!strncasecmp(row[3],"DHP",lengths[3])) plc->NetWork=1;
					else
					{
						if (!strncasecmp(row[3],"DHP_A",lengths[3])) plc->NetWork=1;
						else
						{
							if (!strncasecmp(row[3],"DHP_B",lengths[3])) plc->NetWork=2;
							else plc->NetWork=0;
						}
					}
			if(row[4]!=NULL) plc->Node=atoi(row[4]);
		} else
		{
			Log(LOG_CRIT,"GetPLC : Unable to allocate memory: %s\n",strerror(errno));
			exit(0);
		}
	}
	mysql_free_result(SqlResult);
	res=_GetErrorCode(db,MysqlErrorMsg);
	if (res) return(res); else return(PLCs.Count);
}
int _GetTag(MYSQL *db,PLC *plc)
{
	char sel_str[255];
	char *tmp_str="select TAGNAME,ADDRESS,DATA_TYPE,TIME_SAMPLE\
	from DEFINITION where ADDRESS is not null and READING=1";
	if (plc!=NULL) sprintf(sel_str,"%s and PLCNAME='%s'",tmp_str,plc->PlcName);
		else sprintf(sel_str,"%s",tmp_str);
	MYSQL_ROW row;
	int res=mysql_real_query(db,sel_str,strlen(sel_str));
	if (res)
	{
		_GetErrorCode(db,MysqlErrorMsg);
		return(-1);
	}
	SqlResult=mysql_store_result(db);
	if (SqlResult==NULL)
	{
		_GetErrorCode(db,MysqlErrorMsg);
		return(-1);
	}
	res=mysql_num_rows(SqlResult);
	InitChListe(&(plc->Tags));
	while ((row = mysql_fetch_row(SqlResult)))
	{
		unsigned long *lengths;
		lengths = mysql_fetch_lengths(SqlResult);
		TAG *tag=malloc(sizeof(TAG));
		if (tag!=NULL)
		{
			memset(tag,0,sizeof(TAG));
			AddChListe(&(plc->Tags),tag);
			snprintf(tag->TagName,lengths[0]+1,"%s",row[0]);
			snprintf(tag->Address,lengths[1]+1,"%s",row[1]);
			if(row[2]==NULL) tag->DataType=AB_UNKNOW;
				else
				if (!strncasecmp(row[3],"bit",lengths[3])) tag->DataType=AB_BIT;
					else
					{
						if (!strncasecmp(row[3],"sint",lengths[3])) tag->DataType=AB_SINT;
						else
						{
							if (!strncasecmp(row[3],"int",lengths[3])) tag->DataType=AB_INT;
							else
							{
								if (!strncasecmp(row[3],"dint",lengths[3])) tag->DataType=AB_DINT;
								else
								{
									if (!strncasecmp(row[3],"real",lengths[3])) tag->DataType=AB_REAL;
									else tag->DataType=AB_UNKNOW;
								}
							}
						}
					}
			if (row[3]!=NULL)	tag->Time_Sample=atoi(row[3]);
			if (tag->Time_Sample < MIN_SAMPLE) tag->Time_Sample=MIN_SAMPLE;
			_BuildLogicalBinaryAddress(tag->Address,&(tag->lba));
			/*if (row[4]!=NULL) tags[index].Time_Refresh=atoi(row[4]);
			if (tags[index].Time_Refresh<tags[index].Time_Sample)
				tags[index].Time_Refresh=tags[index].Time_Sample;
			if (row[5]!=NULL) tags[index].Hysteresis=atoi(row[5]);
			if (row[6]!=NULL) tags[index].Record=atoi(row[6]);*/
			tag->Time_Value=0;
		} else
		{
			Log(LOG_CRIT,"GetTag : Unable to allocate memory: %s\n",strerror(errno));
			exit(0);
		}
	}
	mysql_free_result(SqlResult);
	res=_GetErrorCode(db,MysqlErrorMsg);
	if (res) return(res); else return(plc->Tags.Count);
}
int compare_plc(const void *Data1,const void *Data2)
{ int i;
	Logical_Binary_Address lba1,lba2;
	_BuildLogicalBinaryAddress(((TAG *)Data1)->Address,&lba1);
	_BuildLogicalBinaryAddress(((TAG *)Data2)->Address,&lba2);
	for(i=1;i<lba1.size;i++)
	{
		if (lba1.address[i]==lba2.address[i]) continue;
			else
			{
				return(lba1.address[i]-lba2.address[i]);
			}
	}
	return(lba1.mask-lba2.mask);
	//return(0);
}
void AddPacket(PACKET *packet,TAG *tag)
{
    Log(LOG_DEBUG,"On passe ici :AddPacket\n");
	AddChListe(&(packet->Tags),tag);
	if (!(packet->Time_Sample)) packet->Time_Sample=tag->Time_Sample;
	if ((packet->Time_Sample)>(tag->Time_Sample)) packet->Time_Sample=tag->Time_Sample;
}
void Optimise_Plc(PLC *plc)
{
	TAG *btag=NULL;
	PACKET *packet=NULL;
	//int MaxIntPacketSize=100;
	int MaxRealPacketSize=50;
	int MaxEltByPacket=MaxRealPacketSize;
	Logical_Binary_Address blba,lba;
	//Log(LOG_DEBUG,"Optimise : %s\n",plc->PlcName);
	TrieChListe(&(plc->Tags),&compare_plc);

	LISTE *tags=&(plc->Tags);
	LISTE *packets=&(plc->Packets);
	Log(LOG_DEBUG,"Optimise Plc: %s (%d Tags)\n",plc->PlcName,tags->Count);
	ELEMENT *elt=GetFirst(tags);
	/*Freeing  Tags*/
	while (elt!=NULL)
	{
		if (btag==NULL)
		{
			btag=elt->Data;
			_BuildLogicalBinaryAddress(btag->Address,&blba);
			elt=GetNext(tags,elt);
			continue;
		} else
		{
			TAG *tag=elt->Data;
			_BuildLogicalBinaryAddress(tag->Address,&lba);
			if ((blba.address[0]==lba.address[0])&& // same encoding level
					(blba.address[0]==6)&&
					(blba.address[1]==lba.address[1])&& //  same file number
					((blba.address[2]+MaxEltByPacket)>=lba.address[2])) //
			{
				if (packet==NULL) // create a new packet
				{
					packet=malloc(sizeof(PACKET));
					if (packet!=NULL) // new packet
					{
						AddChListe(packets,packet);
						memset(packet,0,sizeof(PACKET));
						AddPacket(packet,btag); // base tag
						memcpy(packet->BaseAddress,btag->Address,sizeof(packet->BaseAddress));
						RemoveChListe(tags,btag);
					} else Log(LOG_CRIT,"Optimise_Plc : Unable to allocate memory: %s\n",strerror(errno));
				};
				if (packet!=NULL) // add to actual packet
				{
					AddPacket(packet,tag);
					tag->Index=lba.address[2]-blba.address[2];
					elt=GetNext(tags,elt);
					RemoveChListe(tags,tag);
					packet->NumElt=lba.address[2]-blba.address[2]+1;
					continue;
				}
			} else
			{
				btag=NULL; // if not, there is no possibility of optimization
				packet=NULL;
				continue;
			}
			elt=GetNext(tags,elt);
		}
	}
}
void Optimise(PLC *plc)
{
	switch (plc->PlcType)
	{
		case PLC5:
		case SLC500:Optimise_Plc(plc);
			break;
		case LGX:
			break;
		default:
			break;
	}

}
int _UpdateTag(MYSQL *db,TAG *tag,double value)
{
	char exec_str[255];
	int res1=0;
	int actual_time=time(NULL);
	if (!TEST)
	{
		sprintf(exec_str,"update DEFINITION set SNAPSHOT_VALUE='%f',SNAPSHOT_TIME=now() where TAGNAME='%s'",value,tag->TagName);
		res1=mysql_real_query(db,exec_str,strlen(exec_str));
	}
	if (!res1) tag->Time_Value=actual_time;
	if (!TEST) return(_GetErrorCode(db,MysqlErrorMsg)); else return(0);
}
int GetSerial(void)
{
	return(getpid()+Serial++);
}
int GetIP(char *path,char *serveur)
{
	char *pos=index(path,',');
	if (pos!=NULL)
	{
		int size=pos-path;
		strncpy(serveur,path,size);
		return(size);
	}else return(0);
}
int ParsePath(char *strpath,char Ip[],BYTE Path[])
{ int index=0,len=0;
	len=strlen(strpath);
	char *str=malloc(len+1);
	strcpy(str,strpath);
	char *pos=strtok(str,",");
	if (pos!=NULL)
	{
		strcpy(Ip,pos);
		while ((pos=strtok(NULL,","))!=NULL)
		{
			Path[index]=strtol(pos,(char**)NULL,10);
			if (errno!=ERANGE) index++;
		};
		free(str);
		return(index);
	} else return(0);
}
Eip_Session *FindSession(char *Ip,LISTE *plcs)
{
	if (plcs->Count<=0) return(NULL);
	int path_size;
	char ip[16];
	BYTE path[40];
	ELEMENT *elt=GetFirst(plcs);
	while (elt!=NULL)
	{
		PLC *plc=elt->Data;
		path_size=ParsePath(plc->PlcPath,ip,path);
		if ((path_size>0)&&(!strncmp(ip,Ip,strlen(Ip)))) return(plc->Session);
		elt=GetNext(plcs,elt);
	}
	return(NULL);
}
Eip_Connection *FindConnection(char *Path,int Network,LISTE *plcs)
{
	if (plcs->Count<=0) return(NULL);
	ELEMENT *elt=GetFirst(plcs);
	while (elt!=NULL)
	{
		PLC *plc=elt->Data;
		if ((plc->Connection!=NULL)&&(!strncmp(plc->PlcPath,Path,strlen(Path)))&&(plc->NetWork==Network)) return(plc->Connection);
		elt=GetNext(plcs,elt);
	}
	return(NULL);
}
int BuildConnection(PLC *plc)
{	int path_size=0;
	char ip[16];
	BYTE path[40];

	Log(LOG_DEBUG,"Building connection for %s (%d)\n",plc->PlcName,CONNECTIONs.Count);

	if ((plc==NULL)||(plc->Connection!=NULL)) return(0);

	Eip_Session *session=plc->Session;
	Eip_Connection *connection=plc->Connection;

	path_size=ParsePath(plc->PlcPath,ip,path);
	if (path_size>0)
	{ // Creating Sessions

		if (plc->Connection==NULL)
		{
			// Creating Connections
			connection=FindConnection(plc->PlcPath,plc->NetWork,&PLCs);
			if (connection==NULL)	Log(LOG_DEBUG,"Connection for %s not find in existing connections creating ...\n",plc->PlcName);
			if (connection==NULL) // not found
			{
				if (plc->NetWork)
					connection=_ConnectPLCOverDHP(session,
					plc->PlcType,
					_Priority,_TimeOut_Ticks,
					(int)session, //TO_ConnID,
					GetSerial(), //ConnSerialNumber
					_OriginatorVendorID,_OriginatorSerialNumber,_TimeOutMultiplier,
					MAX_SAMPLE,
					_Transport,
					plc->NetWork,
					path,
					path_size);
				else
					connection=_ConnectPLCOverCNET(session,
					plc->PlcType,
					_Priority,_TimeOut_Ticks,
					(int)session, //TO_ConnID,
					GetSerial(), //ConnSerialNumber
					_OriginatorVendorID,_OriginatorSerialNumber,_TimeOutMultiplier,
					MAX_SAMPLE,
					_Transport,
					path,
					path_size);

				if (connection!=NULL)
				{
					plc->Connection=connection;
					AddChListe(&CONNECTIONs,connection);
					Log(LOG_DEBUG,"Connection (%p) created for PLC : %s (%s) (%d connections)\n",connection,plc->PlcName,cip_err_msg,CONNECTIONs.Count);
					return(1);
					//CONNECTIONs[Connection_count++]=connection;
				} else
				{
					Log(LOG_CRIT,"Unable to create connection for Plc: %s (%s)\n",plc->PlcName,cip_err_msg);
					return(0);
				}
			}	else
			{
				plc->Connection=connection;
				Log(LOG_DEBUG,"%s Sharing Connection (%p) with another PLC\n",plc->PlcName,connection);
				return(1);
			}
		} else return(0);
	} else
	{
		Log(LOG_CRIT,"Error while parsing IP/Path for Plc : %s\n",plc->PlcName);
		return(0);
	}
}
int BuildSession(PLC *plc)
{	int path_size=0;
	char ip[16];
	BYTE path[40];
	Eip_Session *session=NULL;

	Log(LOG_DEBUG,"Building Session for %s (%d)\n",plc->PlcName,SESSIONs.Count);

	if (plc->Session!=NULL) return(0);

	path_size=ParsePath(plc->PlcPath,ip,path);
	if (path_size>0)
	{ // Creating Sessions
		if (plc->Session==NULL)
		{
			session=FindSession(ip,&PLCs);
			if (session==NULL) // not found
			{
				session=OpenSession(ip);
				if (session!=NULL)
				{
					session->timeout=DEFAULT_TIMEOUT;
					if (_RegisterSession(session)<0)
					{
						CloseSession(session);
						Log(LOG_CRIT,"Unable to register session for Plc: %s (%s) \n",plc->PlcName,cip_err_msg);
					}
					AddChListe(&SESSIONs,session);
					//session->timeout=2000;
					Log(LOG_DEBUG,"Session (%p) created for PLC : %s (%d sessions)\n",session,plc->PlcName,SESSIONs.Count);
					plc->Session=session;
					return(1);
				} else
				{
					Log(LOG_CRIT,"Unable to open session for Plc: %s (%s)\n",plc->PlcName,cip_err_msg);
					return(0);
				}
			}	else
			{
				plc->Session=session;
				Log(LOG_DEBUG,"%s Sharing Session (%p) with another PLC\n",plc->PlcName,session);
				return(1);
			}
		} else return(0);
	} else return(0);
}
int BuildConnections(LISTE *plcs)
{
	ELEMENT *elt=GetFirst(plcs);
	while (elt!=NULL)
	{
		PLC *plc=elt->Data;
		if (BuildSession(plc)) BuildConnection(plc);
		elt=GetNext(plcs,elt);
	}
	return(CONNECTIONs.Count);
}
int CheckSession(PLC *plc)
{
	Eip_Session *session=plc->Session;

	Log(LOG_WARNING,"Checking Session for %s\n",plc->PlcName);

	if (session==NULL) return (BuildSession(plc));
	if (_ListServices(session)<=0)
	{
		Log(LOG_DEBUG,"CheckSession Error : %p\n",plc->Session);
		if (cip_errno==E_RcvTimeOut)
		{
			return(0);
		}
		else
		{
			KillSession(session);
			ELEMENT *elt=GetFirst(&PLCs);
			while (elt!=NULL)
			{
				PLC *plc=elt->Data;
				if (plc->Session==session) plc->Session=NULL;
				elt=GetNext(&PLCs,elt);
			}
			return (BuildSession(plc));
		}
	}else
	{
		Log(LOG_DEBUG,"CheckSession : %p OK\n",plc->Session);
		return(1);
	}
}
int CheckConnection(PLC *plc)
{
	if (plc==NULL) return(0);

	Log(LOG_DEBUG,"Checking Connection for %s\n",plc->PlcName);

	Eip_Connection *new_connection=NULL;
	Eip_Connection *connection=plc->Connection;
	Eip_Session *session=plc->Session;

	if (session==NULL) return(0);
	if (connection==NULL)
	{
		Log(LOG_DEBUG,"Connection for %s is NULL\n",plc->PlcName);
		return(BuildConnection(plc));
	}

	BYTE *path=(BYTE*)(((void*)connection)+sizeof(Eip_Connection));

	if (plc->NetWork)
		new_connection=_ConnectPLCOverDHP(session,
		plc->PlcType,
		_Priority,_TimeOut_Ticks,
		(int)session, //TO_ConnID,
		connection->ConnectionSerialNumber, //ConnSerialNumber
		connection->OriginatorVendorID,
		connection->OriginatorSerialNumber,
		_TimeOutMultiplier,
		MAX_SAMPLE,
		_Transport,
		plc->NetWork,
		path,
		connection->Path_size);
	else
		new_connection=_ConnectPLCOverCNET(session,
		plc->PlcType,
		_Priority,_TimeOut_Ticks,
		(int)session, //TO_ConnID,
		connection->ConnectionSerialNumber, //ConnSerialNumber
		connection->OriginatorVendorID,
		connection->OriginatorSerialNumber,
		_TimeOutMultiplier,
		MAX_SAMPLE,
		_Transport,
		path,
		connection->Path_size);

	//flush(session->query);
	//flush(session->reply);

	Log(LOG_DEBUG,"Checking Connection for %s : %s (%X/%X) -> %p\n",plc->PlcName,cip_err_msg,cip_errno,cip_ext_errno,new_connection);

	if (new_connection==NULL)
	{
		if ((cip_errno==0x01)&&(cip_ext_errno==0x100))
		{
			Log(LOG_WARNING,"Connection OK for %s\n",plc->PlcName);
			return(1); // duplicate Forward open
		}
		// error
	}else
	{
		if (cip_errno)
		{
			free(new_connection);
			new_connection=NULL;
		}
	};

	//Log(LOG_WARNING,"connection = %p\n",connection);

	RemoveChListe(&CONNECTIONs,connection);
	ELEMENT *elt=GetFirst(&PLCs);
	while (elt!=NULL)
	{
		PLC *plc=elt->Data;
		if (plc->Connection==connection) plc->Connection=new_connection;
		elt=GetNext(&PLCs,elt);
	}
	if (new_connection!=NULL)
	{
		AddChListe(&CONNECTIONs,new_connection);
		Log(LOG_WARNING,"Connection OK for %s (%d connections)\n",plc->PlcName,CONNECTIONs.Count);
		return(1);
	}else
	{
		Log(LOG_WARNING,"Connection Error for %s (%d connections)\n",plc->PlcName,CONNECTIONs.Count);
		return(0);
	}
}
int KillConnection(Eip_Connection *connection)
{ int res=0;
	if (connection==NULL) return(0);
	if ((res=Forward_Close(connection))>=0)
		Log(LOG_DEBUG,"Connection (%p) Killed\n",connection);
		else Log(LOG_WARNING,"Unable to kill Connection (%p)\n",connection);
	RemoveChListe(&CONNECTIONs,connection);
	Log(LOG_DEBUG,"There is %d Connection\n",CONNECTIONs.Count);
	return(res);
}
int KillSession(Eip_Session *session)
{ int res=0;
	if (session==NULL) return(0);
	if ((res=_UnRegisterSession(session))>=0)
		Log(LOG_DEBUG,"Session (%p) Killed\n",session);
		else Log(LOG_WARNING,"Unable to kill session (%p)\n",session);
	RemoveChListe(&SESSIONs,session);
	CloseSession(session);
	Log(LOG_DEBUG,"There is %d Session\n",SESSIONs.Count);
	return(res);
}
void KillAll(void)
{
	Log(LOG_NOTICE,"There is %d Sessions and %d Connections\n",SESSIONs.Count,CONNECTIONs.Count);
	ELEMENT *elt;
	while ((elt=GetFirst(&CONNECTIONs))!=NULL)
	{
		KillConnection(elt->Data);
	}
	while ((elt=GetFirst(&SESSIONs))!=NULL)
	{
		KillSession(elt->Data);
	}
}
void SigHand(int num)
{
	switch (num)
	{
		case SIGTERM:	Log(LOG_NOTICE,"receive SIGTERM\n");
									Terminated=1;
									break;
		case SIGINT:	Log(LOG_NOTICE,"receive SIGINT\n");
									Terminated=1;
									break;
		case SIGIO:	Log(LOG_INFO,"receive SIGIO\n");
									//Terminated=1;
									break;
		case SIGSEGV:	Log(LOG_CRIT,"receive SIGSEGV, program ERROR\n");
									exit(1);
									break;
		default:	Log(LOG_CRIT,"receive signal: %d\n",num);
									Terminated=1;
									break;
	}
}

int CreatePidFile(char *PidFile)
{
	int res;
	char *cmd=NULL;

	if ((PidFile!=NULL)&&(strlen(PidFile)))
	{
		res=asprintf(&cmd,"echo \"%d\">%s",getpid(),PidFile);
		Log(LOG_NOTICE,"exec : %s\n",cmd);
		if (res<=0)
		{
			Log(LOG_CRIT, "Write PidFile : %s\n",PidFile);
			return(1);
		}
		res=system(cmd);
		if (res!=0)
		{
			Log(LOG_CRIT, "Unable to write to %s : %s\n",PidFile,strerror(errno));
			return(1);
		}
		free(cmd);
	}
	return(0);
}
int DeletePidFile(char *PidFile)
{
	int res;
	char *cmd=NULL;
	if ((PidFile!=NULL)&&(strlen(PidFile)))
	{
		res=asprintf(&cmd,"rm -f %s",PidFile);
		if (res<=0)
		{
			Log(LOG_CRIT, "Error remove PidFile : %s\n",strerror(errno));
			return(1);
		}
		res=system(cmd);
		if (res!=0)
		{
			Log(LOG_CRIT, "Unable to remove %s : %s\n",PidFile,strerror(errno));
			return(1);
		}
		free(cmd);
	}
	return(0);
}
void Log(int level,char *format,...)
{	va_list list;
	va_start(list,format);//NULL

if (level<=debuglevel)
	{
		if (!DEAMON)
		{	char str[255];
			if (format!="\n")
			{
				vsprintf(str,format,list);
				printf("%ld : %s",time(NULL)-starttime,str);
			} else vprintf(format,list);
		}
		else
		{
			vsyslog(level,format,list);
		}
	}
	va_end(list);
}
int main (int argc,char *argv[])
{
	// Gestion des signaux
	signal(SIGTERM,SigHand);
	signal(SIGINT,SigHand);
	signal(SIGSEGV,SigHand);

	InitChListe(&CONTROLLERs);
	int c;

	while ((c=getopt(argc,argv,"dto:w:l:s:b:u:p:c:f:?h"))!=-1)
		switch(c)
		{
			case 'd':DEAMON=1;break;
			case 't':
				{
					TEST=1;
					break;
				}
			case 'o':
				{
					DEFAULT_TIMEOUT=atoi(optarg);
					break;
				}
			case 'w':
				{
					WAIT_FOR_RECONNECT=atoi(optarg);
					break;
				}
			case 'l':
				{
					switch (atoi(optarg))
					{
						case 3:debuglevel=LOG_DEBUG;break;
						case 2:debuglevel=LOG_NOTICE;break;
						case 1:debuglevel=LOG_WARNING;break;
						case 0:debuglevel=LOG_ERR;break;
						default:debuglevel=LOG_WARNING;break;
					}
				};break;
			case 's': // DBHOST
				{
					strncpy(DBHOST,optarg,sizeof(DBHOST));
					break;
				}
			case 'b': // DB
				{
					strncpy(DB,optarg,sizeof(DB));
					break;
				}
			case 'u': // USER
				{
					strncpy(USER,optarg,sizeof(USER));
					break;
				}
			case 'p': // PASS
				{
					strncpy(PASS,optarg,sizeof(PASS));
					break;
				}
			case 'c': // CONTROLLER
				{
					Log(LOG_DEBUG,"controller : %s\n",optarg);
					int len=strlen(optarg);
					char *name=malloc(len+1);
					memset(name,0,len+1);
					strncpy(name,optarg,len);
					AddChListe(&CONTROLLERs,name);
					break;
				}
			case 'f': // Pidfile
				{
					strncpy(PIDFILE,optarg,sizeof(PIDFILE));
					break;
				}
			case '?':
			case 'h':
				{
					printf("%s (Build on %s %s)\n",LOG_TITLE,__DATE__,__TIME__);
					printf("usage: %s:[-d] [-l0-3] [-s] [-b] [-u] [-p] [-c] [-f pidfile] [-?,h]\n",argv[0]);
					printf("-d\tDaemonize %s\n",argv[0]);
					printf("-o\tCip Timeout (Default :\"%d\")\n",DEFAULT_TIMEOUT);
					printf("-w\tTime to Wait if error (Default :\"%d\")\n",WAIT_FOR_RECONNECT);
					printf("-t\tTest (There is no DB update)\n");
					printf("-l{0..3}\t(default :1)\n");
					printf("\t0\tLOG_ERR\n");
					printf("\t1\tLOG_WARNING (Default)\n");
					printf("\t2\tLOG_NOTICE\n");
					printf("\t3\tLOG_DEBUG\n");
					printf("-s\tDb host (Default :\"%s\")\n",DBHOST);
					printf("-b\tDb name (Default : \"%s\")\n",DB);
					printf("-u\tDb Username (Default :\"%s\")\n",USER);
					printf("-p\tDb Password\n");
					printf("-c\tController {-c...}\n");
					printf("-f\tPid file {-p...} \n");
					return(0);
				}
				break;
			default:break;
		}

	starttime=time(NULL);
	if (DEAMON)
	{
		openlog(LOG_TITLE,LOG_NDELAY,LOG_USER);
	}

	Log(LOG_ALERT,"starting TuxAB, Database is %s on %s\n",DB,DBHOST);

	if (DEAMON)
	{
	switch (fork())
		{
		case -1:
						Log(LOG_CRIT,"Daemon creation Error\n");
						closelog();
						exit(1);
		case 0:	setsid();
						chdir("/");
						umask(0);
						close(0);
						close(1);
						close(2);
						Log(LOG_NOTICE,"Daemon OK (Pid = %d)\n",getpid());
						if (0)
							{
								Log (LOG_CRIT,"Connecting Failed\n");
								Log(LOG_ALERT,"stopped\n");
								closelog();
								exit(2);
							}
							else
							{
								Gardian();
								Log(LOG_ALERT,"stopped\n");
								closelog();
								exit(0);
							}
		default : exit(0);
		}
	}else
	{
		Gardian();
		Log(LOG_ALERT,"stopped\n");
		exit(0);
	}
}
int Gardian(void)
{
	CreatePidFile(PIDFILE);
	int res=mainprog();
	DeletePidFile(PIDFILE);
	return(res);
	/*int res=0;
	while (!Terminated)
	{
		if ((res=setjmp(jb))!=0)
		{
			Log(LOG_CRIT,"Gardian (%d) : %s\n",res);
		}
		res=mainprog();
		if (!Terminated)
		{
			Log(LOG_CRIT,"Gardian restarting main process in %d sec\n",WAITING);
			sleep(WAITING);
		}
	}
	return(res);*/
}
int mainprog(void)
{
	int res=0;
	/* Initialisation des listes */
	InitChListe(&PLCs);
	InitChListe(&SESSIONs);
	InitChListe(&CONNECTIONs);

	res=OpenDb(DBHOST,USER,PASS,DB);
	if (res<0) {Log(LOG_CRIT,"OpenDb (%d) : %s\n",res,MysqlErrorMsg);return(res);}
	else
	{
		Plc_count=GetPlcList(NULL,&CONTROLLERs);
		if (Plc_count<0) Log(LOG_CRIT,"Get Plc count error (%d) : %s\n",Plc_count,MysqlErrorMsg);
			else
			{
				if (Plc_count==0) Log(LOG_NOTICE,"There is no Plcs\n");
					else Log(LOG_NOTICE,"There is : %d Plcs\n",Plc_count);
			}
		Tag_count=GetCount("select * from DEFINITION");
		if (Tag_count<0) Log(LOG_CRIT,"Get Tag count error (%d) : %s\n",Tag_count,MysqlErrorMsg);
			else
			{
				if (Tag_count==0) Log(LOG_NOTICE,"There is no Tags\n");
					else Log(LOG_NOTICE,"There is : %d Tags\n",Tag_count);
			}
		if ((Plc_count>0)&&(Tag_count>0))
		{
			Tag_count=0;
			Plc_count=GetPlcList(&PLCs,&CONTROLLERs);
			if (PLCs.Count>0)
			{
				Log(LOG_NOTICE,"There is %d PLC to create\n",PLCs.Count);
				ELEMENT *elt=GetFirst(&PLCs);
				while (elt!=NULL)
				{
					PLC *plc=elt->Data;
					res=GetTag(plc);
					if (res>0) Tag_count+=res;
					else
						{
							if (res==0) Log(LOG_NOTICE,"(%s) there is no Tag for PLC : %s\n",MysqlErrorMsg,plc->PlcName);
								else Log(LOG_CRIT,"Get Tag error (%d) : %s for PLC : %s\n",res,MysqlErrorMsg,plc->PlcName);
						}
					Optimise(plc);
					elt=GetNext(&PLCs,elt);
				}
			} else Log(LOG_CRIT,"Get Plc error (%d) : %s\n",res,MysqlErrorMsg);
			// Appel Logger
			if (Tag_count>0) res=Logger(&PLCs);
		} else Log(LOG_WARNING,"There is nothing to do. exiting...\n");
		CloseDb;
	}
	return(res);
}
int ReadTag(PLC *plc,TAG *tag)
{	DHP_Header temp={0,0,0,0};
	DHP_Header *dhp=&temp;
	//PCCC_Header *head=NULL;
	double value;
	int tns=getpid();

	switch (plc->PlcType)
	{
		case PLC5:
		case SLC500:
			{
				PLC_Read *data=NULL;
				dhp->Dest_adress=plc->Node;
				if (plc->NetWork) // DHP
					data=ReadPLCData(plc->Session,plc->Connection,dhp,NULL,0,plc->PlcType,tns++,tag->Address,1);
				else data=ReadPLCData(plc->Session,plc->Connection,NULL,NULL,0,plc->PlcType,tns++,tag->Address,1);
				if (data!=NULL)
				{
					if (!cip_errno)
					{
						value=PCCC_GetValueAsFloat(data,0);
						_UpdateTag(&Default_Db,tag,value);
						Log(LOG_DEBUG,"%s on %s = %f (%s)\n",tag->TagName,plc->PlcName,value,cip_err_msg);
					}	else
					{
						Log(LOG_WARNING,"Get PCCC value on tag %s (%s) : (%d / ext %d) %s\n",tag->TagName,plc->PlcName,cip_errno,cip_err_msg,cip_ext_errno);
					}
					free(data);
				} else
				{
					Log(LOG_WARNING,"Error while decoding PCCC value on tag %s (%s) : (%d) %s\n",tag->TagName,plc->PlcName,cip_errno,cip_err_msg);
				}
			}; break;
		case LGX:
		{
			LGX_Read *data=ReadLgxData(plc->Session,plc->Connection,tag->Address,1);
			if (data!=NULL)
			{
				if (!cip_errno)
				{
					value=GetLGXValueAsFloat(data,0);
					_UpdateTag(&Default_Db,tag,value);
					Log(LOG_DEBUG,"%s on %s = %f (%s)\n",tag->TagName,plc->PlcName,value,cip_err_msg);
				}	else
				{
					Log(LOG_WARNING,"Get value : (%d) %s\n",cip_errno,cip_err_msg);
				}
				free(data);
			} else
			{
				Log(LOG_WARNING,"ReadLgxData error on tag : %s (%d : %s)\n",tag->TagName,cip_errno,cip_err_msg);
			}
		}; break;
		default:Log(LOG_WARNING,"Plc type unknow for : %s\n",plc->PlcName);
			break;
	}
	return(cip_errno);
}
int ReadPacket(PLC *plc,PACKET *packet)
{	DHP_Header temp={0,0,0,0};
	DHP_Header *dhp=&temp;
	PCCC_Header *head=NULL;
	double value;
	int tns=getpid();

	switch (plc->PlcType)
	{
		case PLC5:
		case SLC500:
			{
				PLC_Read *data=NULL;
				dhp->Dest_adress=plc->Node;
				if (plc->NetWork) // DHP
					data=ReadPLCData(plc->Session,plc->Connection,dhp,NULL,0,plc->PlcType,tns++,packet->BaseAddress,packet->NumElt);
				else data=ReadPLCData(plc->Session,plc->Connection,NULL,NULL,0,plc->PlcType,tns++,packet->BaseAddress,packet->NumElt);
				if (data!=NULL)
				{
					if (!cip_errno)
					{
						Log(LOG_DEBUG,"Packet %s on %s (%s)\n",packet->BaseAddress,plc->PlcName,cip_err_msg);
						ELEMENT *elt=GetFirst(&(packet->Tags));
						while (elt!=NULL)
						{
							TAG *tag=elt->Data;
							value=PCCC_GetValueAsFloat(data,tag->Index);
							_UpdateTag(&Default_Db,tag,value);
							Log(LOG_DEBUG,"\t%s = %f (%s)\n",tag->TagName,value,cip_err_msg);
							elt=GetNext(&(packet->Tags),elt);
						}
					}	else
					{
						Log(LOG_WARNING,"Get PCCC value on Packet %s on %s : (%d / ext %d) %s\n",packet->BaseAddress,plc->PlcName,cip_errno,cip_err_msg,cip_ext_errno);
					}
					free(data);
				} else
				{
					Log(LOG_WARNING,"Error while Reading Packet %s on %s : (%d) %s\n",packet->BaseAddress,plc->PlcName,cip_errno,cip_err_msg);
					_CipFlushBuffer(head,30);
				}
			}; break;
		default:Log(LOG_WARNING,"Plc type unknow for : %s\n",plc->PlcName);
			break;
	}
	packet->Time_Value=time(NULL);
	return(cip_errno);
}
int Logger(LISTE *plcs)
{ int res=0,Comm_err=0,Read_Something=0;
	if (TEST)
	{
		ListePlc(plcs);
		//return(0);
	}
	int now=time(NULL);
	res=BuildConnections(plcs);
	Log(LOG_DEBUG,"%d Connections build\n",res);
	while (!Terminated)
	{
		ELEMENT *elt=GetFirst(plcs);
		while (elt!=NULL)
		{
			/***************** Liste PLC ************************/
			PLC *plc=elt->Data;
			now=time(NULL);
			/* Something to do ? */
			if (plc->Next_Time>now)
			{
				elt=GetNext(plcs,elt);
				continue;
			}
			/* Test Session */
			if ((plc->Session==NULL)&&(!CheckSession(plc)))
			{
				Log(LOG_WARNING,"Session unavailable for : %s\n",plc->PlcName);
				plc->Next_Time=now+WAIT_FOR_RECONNECT;
				elt=GetNext(plcs,elt);
				continue;
			}
			/* Test Connection */
			if ((plc->Connection==NULL)&&(!CheckConnection(plc)))
			{
				Log(LOG_WARNING,"Connection unavailable for : %s\n",plc->PlcName);
				plc->Next_Time=now+WAIT_FOR_RECONNECT;
				elt=GetNext(plcs,elt);
				continue;
			}
			Read_Something=0;
			Comm_err=1;
			plc->Next_Time=now+0.95*MAX_SAMPLE/1000;
			/************************** Read Tags ************************/
			ELEMENT *elt2=GetFirst(&(plc->Tags));
			while (elt2!=NULL)
			{
				TAG *tag=elt2->Data;
				if ((now-tag->Time_Value)>(1.5*tag->Time_Sample))
					Log(LOG_INFO,"Time Sample exceed on tag : %s (%s)\n",tag->TagName,plc->PlcName);
				if ((now-tag->Time_Value)>=tag->Time_Sample)
				{
					Read_Something=1;
					res=ReadTag(plc,tag);
					if (res>=0) Comm_err=0; // At least one tag is Ok
					Log(LOG_DEBUG,"__________________________\n");
				}
				if ((tag->Time_Value+tag->Time_Sample)<(plc->Next_Time))
					plc->Next_Time=tag->Time_Value+tag->Time_Sample;
				elt2=GetNext(&(plc->Tags),elt2);
			}
                        Log(LOG_DEBUG,"=======================================================\n");
			/*********************** Read Packets *************************/
                        if (plc->Packets.Count>0) {
			elt2=GetFirst(&(plc->Packets));
			while (elt2!=NULL)
			{
				PACKET *packet=elt2->Data;
				if ((now-packet->Time_Value)>(1.5*packet->Time_Sample))
					Log(LOG_INFO,"Time Sample exceed on packet : %s (%s)\n",packet->BaseAddress,plc->PlcName);
				if ((now-packet->Time_Value)>=packet->Time_Sample)
				{
					Read_Something=1;
					res=ReadPacket(plc,packet);
					if (res>=0) Comm_err=0; // At least one tag is Ok
					Log(LOG_DEBUG,"\n");
				}
				if ((packet->Time_Value+packet->Time_Sample)<(plc->Next_Time))
					plc->Next_Time=packet->Time_Value+packet->Time_Sample;
				elt2=GetNext(&(plc->Packets),elt2);
			}
                        }
			/* Check Plc */
			if (Comm_err && Read_Something) // All Tags & packets are in error
			{
				plc->Errors+=1;
				Log(LOG_WARNING,"All tags in error for : %s suspending for %d seconds (Errors :%d)\n",plc->PlcName,WAIT_FOR_RECONNECT,plc->Errors);
				plc->Next_Time=now+WAIT_FOR_RECONNECT;
				if (plc->Errors>=MAX_ERR_BEFORE_CHECK)
				{
					if (CheckSession(plc)&&CheckConnection(plc))
					{
						plc->Errors=0;
						plc->Next_Time=now;
					}else
					{
							Log(LOG_WARNING,"Unable to reconnect\n");
							plc->Next_Time=now+3*WAIT_FOR_RECONNECT;
					}
				};
			}else
			{ // Some Tags are Ok
				plc->Errors=0;
			}
			elt=GetNext(plcs,elt);
		}
		sleep(1);
	}
	Log(LOG_NOTICE,"Killing Connections\n");
	KillAll();
	FreeAll(plcs);
	return(0);
}
void FreeAll(LISTE *plcs)
{
	ELEMENT *elt,*elt2,*elt3;
	Log(LOG_DEBUG,"Freeing %d Plcs\n",plcs->Count);
	while ((elt=GetFirst(plcs))!=NULL)
	{
		PLC *plc=elt->Data;
		LISTE *tags=&(plc->Tags);
		LISTE *packets=&(plc->Packets);
		Log(LOG_DEBUG,"Plc: %s (%d Tags, %d Packets))\n",plc->PlcName,tags->Count,packets->Count);
		/*Freeing  Tags*/
		while ((elt2=GetFirst(tags))!=NULL)
		{
			TAG *tag=elt2->Data;
			Log(LOG_DEBUG,"\tTag: %s (%s))\n",tag->TagName,tag->Address);
			free(tag);
			RemoveChListe_Ex(tags,elt2);
			free(elt2);
		}
		/*Freeing  Packets*/
                if (packets->Count>0) {
		while ((elt2=GetFirst(packets))!=NULL)
		{
			PACKET *packet=elt2->Data;
			Log(LOG_DEBUG,"\tPacket: %s (%d tags , size : %d) sampling : %d\n" \
			,packet->BaseAddress,(packet->Tags).Count,packet->NumElt,packet->Time_Sample);
			/*Freeing Tags in Packets*/
			while ((elt3=GetFirst(&(packet->Tags)))!=NULL)
			{
				TAG *tag=elt3->Data;
				Log(LOG_DEBUG,"\tTag: %s (%s))\n",tag->TagName,tag->Address);
				free(tag);
				RemoveChListe_Ex(&(packet->Tags),elt3);
				free(elt3);
			}
			free(packet);
			RemoveChListe_Ex(packets,elt2);
			free(elt2);
		}
                }
		free(plc);
		RemoveChListe_Ex(plcs,elt);
		free(elt);
	}
	while ((elt=GetFirst(&CONTROLLERs))!=NULL)
	{
		free(elt->Data);
		RemoveChListe_Ex(&CONTROLLERs,elt);
		free(elt);
	}
}
int ListePlc(LISTE *plcs)
{
	ELEMENT *elt=GetFirst(plcs);
	Log(LOG_NOTICE,"There is %d Plcs\n",plcs->Count);
	while (elt!=NULL)
	{
		PLC *plc=elt->Data;
		if (!((plc->PlcType==PLC5)||(plc->PlcType==SLC500))) {elt=GetNext(plcs,elt);continue;};
		LISTE *tags=&(plc->Tags);
		LISTE *packets=&(plc->Packets);
		Log(LOG_NOTICE,"Plc: %s (%d Tags, %d Packets))\n",plc->PlcName,tags->Count,packets->Count);
		ELEMENT *elt2=GetFirst(tags);
		/*Freeing  Tags*/
		while (elt2!=NULL)
		{
			TAG *tag=elt2->Data;
			Log(LOG_NOTICE,"\tTag: %s (%s , index= %d))\n",tag->TagName,tag->Address,tag->Index);
			elt2=GetNext(tags,elt2);
		}
		/*Freeing  Packets*/
		elt2=GetFirst(packets);
		while (elt2!=NULL)
		{
			PACKET *packet=elt2->Data;
			Log(LOG_NOTICE,"\tPacket: %s (%d tags , size : %d) sampling : %d\n" \
			,packet->BaseAddress,(packet->Tags).Count,packet->NumElt,packet->Time_Sample);
			ELEMENT *elt3=GetFirst(&(packet->Tags));
			while (elt3!=NULL)
			{
				TAG *tag=elt3->Data;
				Log(LOG_NOTICE,"\t\tPacket Tag: %s (%s , index= %d))\n",tag->TagName,tag->Address,tag->Index);
				elt3=GetNext(&(packet->Tags),elt3);
			}
			elt2=GetNext(packets,elt2);
		}
		elt=GetNext(plcs,elt);
	}
	return(0);
}
