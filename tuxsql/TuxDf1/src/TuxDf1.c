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
//#include <modbus/modbus.h>
#include "TuxDf1.h"
#include "df1.h"

#define space printf("\n");
#define LOG_TITLE "TuxDf1"
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
int mainprog(void);
int Logger(LISTE *plcs);
int ListePlc(LISTE *plcs);

int BuildSockets(LISTE *plcs);
int BuildSocket(PLC *plc);
void KillAll(LISTE *plcs);
void FreeAll(LISTE *plcs);

int ReadTag(PLC *plc,TAG *tag);
int WriteTag(PLC *plc,TAG *tag);
int _Read_WriteValue_Tag(MYSQL *db,TAG *tag);
int Gardian(void);

int GetSerial(void);
int CheckSession(PLC *plc);

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

char *TuxGetErrMsg(int s_err_type,int s_errno)
{
 switch (s_err_type){
  case Tux_Error:return TuxGetInternalErrMsg(s_errno);
  default :return Df1_GetDf1ErrMsg(s_errno);
 }
}

/***********************************************************************/
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
		res=asprintf(&sel_str,"SELECT distinct p.* FROM DF1 as p LEFT JOIN DEFINITION as d ON p.PLCNAME=\
			d.PLCNAME where p.PLC_TTY is not null and d.ADDRESS is not null and d.READING=1\
	and d.TAG_SYSTEM='DF1' and p.PLCNAME='%s'",plcname);
	else res=asprintf(&sel_str,"SELECT distinct p.* FROM DF1 as p LEFT JOIN DEFINITION as d ON p.PLCNAME=d.PLCNAME where p.PLC_ENABLE=1 and p.PLC_TTY is not null and d.ADDRESS is not null and d.READING=1 and d.TAG_SYSTEM='DF1'");
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
			Log(LOG_NOTICE,"speed : %d\n",atoi(row[2]));
			plc->PlcSpeed=atoi(row[2]);
			//if(row[2]!=NULL) plc->DeviceId=atoi(row[2]);
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

int _UpdateWriteTag(MYSQL *db,TAG *tag,double value)
{
	char exec_str[255];
	int res1=0;
	int actual_time=time(NULL);
	if (!TEST)
	{
		sprintf(exec_str,"update DEFINITION set SNAPSHOT_VALUE='%f',SNAPSHOT_TIME=now(),WRITE_VALUE=NULL where TAGNAME='%s'",value,tag->TagName);
		res1=mysql_real_query(db,exec_str,strlen(exec_str));
	}
	if (!res1) tag->Time_Value=actual_time;
	if (!TEST) return(_GetErrorCode(db,MysqlErrorMsg)); else return(0);
}

int _Read_WriteValue_Tag(MYSQL *db,TAG *tag)
{
	char exec_str[255];
	int res1=0;
        MYSQL_ROW row;
	//int actual_time=time(NULL);
        sprintf(exec_str,"select WRITE_VALUE from DEFINITION where TAGNAME='%s'",tag->TagName);
        res1=mysql_real_query(db,exec_str,strlen(exec_str));
	if (res1)
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
	res1=mysql_num_rows(SqlResult);
        while ((row = mysql_fetch_row(SqlResult)))
	{
		unsigned long *lengths;
		lengths = mysql_fetch_lengths(SqlResult);
		if (tag!=NULL)
		{
			if (row[0]!=NULL) {
                            tag->WriteValue=atof(row[0]);
                            tag->WriteExist=1;
                        } else {
                            //tag->WriteValue=NULL;
                            tag->WriteExist=0;
                        }
		} else
		{
			Log(LOG_CRIT,"_Read_WriteValue_Tag : Error in _Read_WriteValue : %s\n",strerror(errno));
			exit(0);
		}
	}
	mysql_free_result(SqlResult);
	return(_GetErrorCode(db,MysqlErrorMsg));
}

int GetSerial(void)
{
	return(getpid()+Serial++);
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
	plc->socket=Df1_open_device(plc->PlcPath, plc->PlcSpeed,0,8,1); //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	if (plc->socket>=0)
	{
		Log(LOG_DEBUG,"Socket (%d) created for PLC : %s(%s,%d)\n",plc->socket,plc->PlcName,plc->PlcPath,plc->PlcSpeed);
		return(plc->socket);
	} else
	{
		Log(LOG_CRIT,"Unable to open com for Plc: %s \n",plc->PlcName); 
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
			Df1_close_device(plc->socket);
		}
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

int main (int argc,char *argv[])
{
	// Gestion des signaux
	signal(SIGTERM,SigHand);
	signal(SIGINT,SigHand);
	signal(SIGSEGV,SigHand);
	signal(SIGIO, SIG_IGN);  /* ne rien faire sur SIGIO */	

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
			/*case 't':
				{
					MB_TIMEOUT=1000*atoi(optarg);
					break;
				}*/
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
					//printf("-t\tTimeout (Default : %d s)\n",MB_TIMEOUT/1000);
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
					//Optimise(plc);
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

//****************************************************************************
int ReadTag(PLC *plc,TAG *tag)
{
	double value;
	int error;
	if ((error=read_Tag(plc->socket, tag->Address, &value))!=SUCCESS) {
		Log(LOG_WARNING,"Error on tag %s: (%s) \n",tag->TagName,TuxGetErrMsg(DF1_ERROR,error));
	} else {
		Log(LOG_DEBUG,"tag %s: (%d) : %f (%s)\n",tag->TagName,error,value,tag->Address);
		_UpdateTag(&Default_Db,tag,value);
	}
	return error;
}

int WriteTag(PLC *plc,TAG *tag)
{
	double value= tag->WriteValue;
	int error;
        Log(LOG_DEBUG,"tag write %s: (%d) : %f (%s)\n",tag->TagName,error,value,tag->Address);
	if ((error=write_Tag(plc->socket, tag->Address, &value))!=SUCCESS) {
		Log(LOG_WARNING,"Error on tag %s: (%s) \n",tag->TagName,TuxGetErrMsg(DF1_ERROR,error));
	} else {
		Log(LOG_DEBUG,"tag %s: (%d) : %f (%s)\n",tag->TagName,error,value,tag->Address);
		_UpdateWriteTag(&Default_Db,tag,value);
	}
	return error;
}

int Logger(LISTE *plcs)
{
	int res=0, Comm_OK = FALSE, Read_Something= FALSE;
	if (TEST)
	{
		ListePlc(plcs);
		//return(0);
	}
	int startTime=time(NULL);
	//res=BuildSockets(plcs);
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
			/* Test Comm */
			if (plc->socket<0)
			{
				if (BuildSocket(plc)<0) //try Open comm
				{
					Log(LOG_WARNING,"Socket unavailable for : %s\n",plc->PlcName);
					plc->Next_Time=startTime+WAIT_FOR_RECONNECT;
					elt=GetNext(plcs,elt);
					continue;
				} else Log(LOG_INFO,"Socket OK for : %s\n",plc->PlcName);
			}
			startTime=time(NULL);
			Read_Something = FALSE;
			Comm_OK = FALSE;
			Log(LOG_DEBUG,"Set plc->Next_Time in %d seconds \n",plc->Next_Time-startTime);
			/* Read Tags */
			ELEMENT *elt2=GetFirst(&(plc->Tags));
			while (elt2!=NULL)
			{
				TAG *tag=elt2->Data;
                                //insérer ici test si write_value<>NULL si pas NULL alors ecriture puis lecture
                                res=_Read_WriteValue_Tag(&Default_Db,tag);
 				if (res == SUCCESS) {
                                    if (tag->WriteExist== 1) {
                                        res=WriteTag(plc,tag);
                                        if (res == SUCCESS) { // At least one tag is Ok
                                                Comm_OK = TRUE;
                                        } else {
                                                Log(LOG_WARNING,"Error : %s on tag : %s (%s)\n",tag->TagName,plc->PlcName);
                                        }
                                    } else {
                                        res=ReadTag(plc,tag);
                                        if (res == SUCCESS) { // At least one tag is Ok
                                                Comm_OK = TRUE;
                                        } else {
                                                Log(LOG_WARNING,"Error : %s on tag : %s (%s)\n",tag->TagName,plc->PlcName);
                                        }
                                    }
				} else {
					Log(LOG_WARNING,"Error : %s on tag : %s (%s)\n",tag->TagName,plc->PlcName);
				}


				elt2=GetNext(&(plc->Tags),elt2);
			}
			/* Check Plc */
			if (Comm_OK==FALSE) // All Tags are in error
			{
				Log(LOG_WARNING,"All tags in error for : %s suspending for %d seconds\n",plc->PlcName,WAIT_FOR_RECONNECT);
				plc->Next_Time=startTime+WAIT_FOR_RECONNECT;
			}
			elt=GetNext(plcs,elt);
		}
		//sleep(1);
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
