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
#define _GNU_SOURCE

#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdio.h>
#include <modbus/modbus.h>
#include "TuxMdb.h"

#define space printf("\n");
#define LOG_TITLE "TuxModbus"
#define WAITING 60 // Gardian
#define WAIT_FOR_RECONNECT 60
#define MAX_CONTROLLER_LIST 20

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

int BuildSockets(LISTE *plcs);
int BuildSocket(PLC *plc);
PLC *FindSocket(PLC *plc,LISTE *plcs);
void CloseSocket(int socket,LISTE *plcs);
//int KillSocket(int socket);
void KillAll(LISTE *plcs);
void FreeAll(LISTE *plcs);

int ReadTag(PLC *plc,TAG *tag);
int ReadPacket(PLC *plc,PACKET *packet);
int Gardian(void);

int GetSerial(void);
int CheckSession(PLC *plc);

int compare_plc(const void *Data1,const void *Data2);
void Optimise_Plc(PLC *plc);
void Optimise(PLC *plc);
void AddPacket(PACKET *packet,TAG *tag);

/******************* Global Var ************************************/

unsigned int Tux_errno;
unsigned int Tux_ext_errno;
int Tux_err_type;
char Tux_err_msg[MAX_ERR_MSG_LEN+1];
int MAX_SAMPLE=0;

int DEAMON=0;
int NOOPTIMIZATION=0;
int TEST=0;
int debuglevel=LOG_WARNING;

int Terminated=0;
int starttime=0;

LISTE PLCs;
//LISTE SOCKETs;
LISTE CONTROLLERs;

int Plc_count,Tag_count;
int Serial=0;

/******************* Functions **************************************/

char *TuxGetInternalErrMsg(unsigned int ErrorCode)
{
	switch (ErrorCode){
		case T_Success:return("Success");
		case T_Error:return("Error");
		case T_DBError:return("DB Error");
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
  default :return(MBGetMBErrMsg(s_errno,ext_errno));
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
		res=asprintf(&sel_str,"SELECT distinct p.* FROM MODBUS as p LEFT JOIN DEFINITION as d ON p.PLCNAME=\
			d.PLCNAME where p.PLC_PATH is not null and d.ADDRESS is not null and d.READING=1\
	and d.TAG_SYSTEM='MB' and p.PLCNAME='%s'",plcname);
	else res=asprintf(&sel_str,"SELECT distinct p.* FROM MODBUS as p LEFT JOIN DEFINITION as d ON p.PLCNAME=\
		d.PLCNAME where p.PLC_ENABLE=1 and p.PLC_PATH is not null and d.ADDRESS is not null and d.READING=1 and d.TAG_SYSTEM='MB'");
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
			plc->socket=-1;
			snprintf(plc->PlcName,lengths[0]+1,"%s",row[0]);
			snprintf(plc->PlcPath,lengths[1]+1,"%s",row[1]);
			if(row[2]!=NULL) plc->DeviceId=atoi(row[2]);
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
	char *tmp_str="select TAGNAME,ADDRESS,TIME_SAMPLE\
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
			tag->DataType=0;
			{
			};

			if (row[2]!=NULL)	tag->Time_Sample=atoi(row[2]);
			if (tag->Time_Sample < MIN_SAMPLE) tag->Time_Sample=MIN_SAMPLE;
			tag->Time_Value=0;
			if (tag->Time_Sample>MAX_SAMPLE) MAX_SAMPLE=tag->Time_Sample;
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
{
	int i1=_MBGetAddress(((TAG *)Data1)->Address);
	int i2=_MBGetAddress(((TAG *)Data2)->Address);
	return(i1-i2);
}
void AddPacket(PACKET *packet,TAG *tag)
{
	AddChListe(&(packet->Tags),tag);
	if (!(packet->Time_Sample)) packet->Time_Sample=tag->Time_Sample;
	if ((packet->Time_Sample)>(tag->Time_Sample)) packet->Time_Sample=tag->Time_Sample;
}
void Optimise_Plc(PLC *plc)
{
	TAG *btag=NULL; // Base Tag for optimized Packet
	PACKET *packet=NULL;
	int bfindex=0,findex=0,bfunc=0,func=0;
	int MaxRealPacketSize=50;
	int MaxEltByPacket=MaxRealPacketSize;

	TrieChListe(&(plc->Tags),&compare_plc);
	LISTE *tags=&(plc->Tags);
	LISTE *packets=&(plc->Packets);
	Log(LOG_DEBUG,"Optimise Plc: %s (%d Tags)\n",plc->PlcName,tags->Count);
	ELEMENT *elt=GetFirst(tags);
	while (elt!=NULL)
	{
		if (btag==NULL)
		{
			btag=elt->Data;
			bfindex=_MBGetC_F_Index(btag->Address);
			//bfunc=_MBGetC_F_Index(btag->Address);
			bfunc=FTable[bfindex].Read_Function;
			elt=GetNext(tags,elt);
			continue;
		} else
		{
			TAG *tag=elt->Data;
			findex=_MBGetC_F_Index(btag->Address);
			//func=_MBGetC_F_Index(tag->Address);
			func=FTable[findex].Read_Function;
			if ((bfunc==func)&&(_MBGetAddress(btag->Address)+MaxEltByPacket>=_MBGetAddress(tag->Address)))
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
					tag->Index=_MBGetAddress(tag->Address)-_MBGetAddress(btag->Address);//-1;
					elt=GetNext(tags,elt);
					RemoveChListe(tags,tag);
					packet->NumElt=_MBGetAddress(tag->Address)-_MBGetAddress(btag->Address)+_MBGetDataSize(FTable[findex].DataType);
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
	if (!NOOPTIMIZATION)	Optimise_Plc(plc);
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
PLC *FindSocket(PLC *plc,LISTE *plcs)
{
	if (plcs->Count<=0) return(NULL);
	ELEMENT *elt=GetFirst(plcs);
	while (elt!=NULL)
	{
		PLC *_plc=elt->Data;
		if ((!strncmp(_plc->PlcPath,plc->PlcPath,strlen(plc->PlcPath)))&&(plc!=_plc)&&(_plc->socket>=0))return(_plc);
		elt=GetNext(plcs,elt);
	}
	return(NULL);
}
int BuildSockets(LISTE *plcs)
{
	ELEMENT *elt=GetFirst(plcs);
	while (elt!=NULL)
	{
		PLC *plc=elt->Data;
		BuildSocket(plc);
		elt=GetNext(plcs,elt);
	}
	return(0);
}
int BuildSocket(PLC *plc)
{
	if (plc->socket>=0) return(plc->socket);
	plc->socket=MBOpenSock(plc->PlcPath,MODBUS_PORT);
	if (plc->socket>=0)
	{
		Log(LOG_DEBUG,"Socket (%d) created for PLC : %s\n",plc->socket,plc->PlcName);
		return(plc->socket);
	} else
	{
		Log(LOG_CRIT,"Unable to open socket for Plc: %s (%s)\n",plc->PlcName,mb_err_msg);
		return(-1);
	}
}
int CheckSession(PLC *plc)
{
	return(1);
}
void KillAll(LISTE *plcs)
{
	if (plcs->Count<=0)  return;
	ELEMENT *elt=GetFirst(plcs);
	while (elt!=NULL)
	{
		PLC *plc=elt->Data;
		if (plc->socket>=0)
		{
			plc->socket=-1;
			close(plc->socket);
		}
		elt=GetNext(plcs,elt);
	}
}
void CloseSocket(int socket,LISTE *plcs)
{
	if (socket>=0) close(socket);
	if (plcs->Count<=0)  return;
	ELEMENT *elt=GetFirst(plcs);
	while (elt!=NULL)
	{
		PLC *plc=elt->Data;
		if (plc->socket==socket) plc->socket=-1;
		elt=GetNext(plcs,elt);
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

	while ((c=getopt(argc,argv,"doTt:l:s:b:u:p:c:f:?h"))!=-1)
		switch(c)
		{
			case 'd':DEAMON=1;break;
			case 'o':NOOPTIMIZATION=1;break;
			case 'T':
				{
					TEST=1;
					break;
				}
			case 't':
				{
					MB_TIMEOUT=1000*atoi(optarg);
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
					printf("usage: %s:[-d] [-o] [-T] [-t timeout] [-l0-3] [-s] [-b] [-u] [-p] [-c] [-f pidfile] [-?,h]\n",argv[0]);
					printf("-d\tDaemonize %s\n",LOG_TITLE);
					printf("-o\tNo read optimization\n");
					printf("-T\tTest %s (There is no DB update)\n",LOG_TITLE);
					printf("-t\tTimeout (Default : %d s)\n",MB_TIMEOUT/1000);
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

	Log(LOG_ALERT,"starting %s, Database is %s on %s\n",LOG_TITLE,DB,DBHOST);

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
{
	double value;
	mb_read_rsp *data=MBRead_Ext(plc->socket,plc->DeviceId,tag->Address,1);
	if (data!=NULL)
	{
		value=MBGetValueAsReal(data,0);
		Log(LOG_DEBUG,"%s Tag %s =%f (%s)\n",plc->PlcName,tag->Address,value,mb_err_msg);
		if (!mb_errno) _UpdateTag(&Default_Db,tag,value);
			else Log(LOG_WARNING,"Error on tag %s: (%d) %s\n",tag->TagName,mb_errno,mb_err_msg);
		free(data);
	}else
	{
		Log(LOG_WARNING,"Error on tag (no response) %s: (%d) %s\n",tag->TagName,mb_errno,mb_err_msg);
	}
	return(mb_errno);
}
int ReadPacket(PLC *plc,PACKET *packet)
{
	double value;
	int result=0;
	mb_read_rsp *data=MBRead_Raw(plc->socket,plc->DeviceId,packet->BaseAddress,packet->NumElt);
	Log(LOG_DEBUG,"Packet %s on %s (%s)\n",packet->BaseAddress,plc->PlcName,mb_err_msg);
	if (data!=NULL)
	{
		ELEMENT *elt=GetFirst(&(packet->Tags));
		while (elt!=NULL)
		{
			TAG *tag=elt->Data;
			value=MBGetValueAsReal(data,tag->Index);
			if (!mb_errno)
			{
				_UpdateTag(&Default_Db,tag,value);
				Log(LOG_DEBUG,"\t%s = %f (%s)\n",tag->TagName,value,mb_err_msg);
			}	else Log(LOG_WARNING,"Error on tag %s: (%d) %s\n",tag->TagName,mb_errno,mb_err_msg);
			elt=GetNext(&(packet->Tags),elt);
		}
		packet->Time_Value=time(NULL);
	}	else
	{
		Log(LOG_WARNING,"Error on Packet (no response) %s: (%d ) %s\n",packet->BaseAddress,mb_errno,mb_err_msg);
		result=mb_errno;
	}
	free(data);
	return(result);
}
int Logger(LISTE *plcs)
{
	int res=0,Comm_err=0,Read_Something=0;
	if (TEST)
	{
		ListePlc(plcs);
		//return(0);
	}
	int now=time(NULL);
	res=BuildSockets(plcs);
	while (!Terminated)
	{
		ELEMENT *elt=GetFirst(plcs);
		while (elt!=NULL)  // PLCs
		{
			PLC *plc=elt->Data;
			/* Something to do ? */
			if (plc->Next_Time>time(NULL))
			{
				elt=GetNext(plcs,elt);			
				continue;
			}
			/* Test Socket */
			if (plc->socket<0)
			{
				if (BuildSocket(plc)<0)
				{
					Log(LOG_WARNING,"Socket unavailable for : %s\n",plc->PlcName);
					plc->Next_Time=now+WAIT_FOR_RECONNECT;
					elt=GetNext(plcs,elt);
					continue;
				} else Log(LOG_INFO,"Socket build for : %s\n",plc->PlcName);
			}
			now=time(NULL);
			Read_Something=0;
			Comm_err=1;
			//plc->Next_Time=now+0.95*MAX_SAMPLE/1000;
			plc->Next_Time=now+MAX_SAMPLE;
			Log(LOG_DEBUG,"Set plc->Next_Time in %d seconds (MAX_SAMPLE : %d)\n",plc->Next_Time-now,MAX_SAMPLE);
			/* Read Tags */
			ELEMENT *elt2=GetFirst(&(plc->Tags));
			while (elt2!=NULL)
			{
				TAG *tag=elt2->Data;
				if ((now-tag->Time_Value)>(1.5*tag->Time_Sample))
					Log(LOG_WARNING,"Time Sample exceed on tag : %s (%s)\n",tag->TagName,plc->PlcName);
				if ((now-tag->Time_Value)>=tag->Time_Sample)
				{
					//Log(LOG_DEBUG,"Reading tag : %s (%s) (%d - %d > %d)\n",tag->TagName,plc->PlcName,now,tag->Time_Value,tag->Time_Sample);
					Read_Something=1;
					res=ReadTag(plc,tag);
					if (res==0) Comm_err=0; // At least one tag is Ok
					if (mb_errno==EPIPE) CloseSocket(plc->socket,plcs);
				}
				if ((tag->Time_Value+tag->Time_Sample)<(plc->Next_Time))
				{
					plc->Next_Time=tag->Time_Value+tag->Time_Sample;
					Log(LOG_DEBUG,"plc->Next_Time in %d seconds*\n",plc->Next_Time-now);
				}
				elt2=GetNext(&(plc->Tags),elt2);
			}
			/* Read Packets */
			elt2=GetFirst(&(plc->Packets));
			while (elt2!=NULL)
			{
				PACKET *packet=elt2->Data;
				if ((now-packet->Time_Value)>(1.5*packet->Time_Sample))
					Log(LOG_WARNING,"Time Sample exceed on packet : %s (%s)\n",packet->BaseAddress,plc->PlcName);
				if ((now-packet->Time_Value)>=packet->Time_Sample)
				{
					Read_Something=1;
					res=ReadPacket(plc,packet);
					if (res>=0) Comm_err=0; // At least one tag is Ok
					if (mb_errno==EPIPE) CloseSocket(plc->socket,plcs);
				}
				if ((packet->Time_Value+packet->Time_Sample)<(plc->Next_Time))
				{
					plc->Next_Time=packet->Time_Value+packet->Time_Sample;
					Log(LOG_DEBUG,"plc->Next_Time in %d seconds\n",plc->Next_Time-now);
				}
				elt2=GetNext(&(plc->Packets),elt2);
			}
			/* Check Plc */
			if (Comm_err && Read_Something) // All Tags & packets are in error
			{
				Log(LOG_WARNING,"All tags in error for : %s suspending for %d seconds\n",plc->PlcName,WAIT_FOR_RECONNECT);
				plc->Next_Time=now+WAIT_FOR_RECONNECT;
			}
			if (plc->Next_Time>0.8*(time(NULL)+MODBUS_SOCK_TIMEOUT))
			{
				close(plc->socket);
				plc->socket=-1;
				Log(LOG_DEBUG,"Closing socket for plc : %s \n",plc->PlcName);
			}			
			elt=GetNext(plcs,elt);
		}
		sleep(1);
	}
	Log(LOG_NOTICE,"Killing Connections\n");
	KillAll(plcs);
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
		while ((elt2=GetFirst(packets))!=NULL)
		{
			PACKET *packet=elt2->Data;
			Log(LOG_DEBUG,"\tPacket: %s (%d tags , size : %d) sampling : %d\n" \
			,packet->BaseAddress,(packet->Tags).Count,packet->NumElt,packet->Time_Sample);
			/*Freeing Tags in Packets*/
			while ((elt3=GetFirst(&(packet->Tags)))!=NULL)
			{
				TAG *tag=elt3->Data;
				Log(LOG_DEBUG,"\tTag: %s (%s index=%d))\n",tag->TagName,tag->Address,tag->Index);
				free(tag);
				RemoveChListe_Ex(&(packet->Tags),elt3);
				free(elt3);
			}
			free(packet);
			RemoveChListe_Ex(packets,elt2);
			free(elt2);
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
