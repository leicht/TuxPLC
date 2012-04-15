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

#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sutil/MySQL.h>
		
#include "TuxHisto.h"

#define space printf("\n");
#define LOG_TITLE "TuxHisto"
#define MAXERRORTAGPC 50
#define WAITING 60 // Gardian
#define WAIT_FOR_RECONNECT 60
#define SQLTOLIST 10

char DBHOST[20]={"localhost\0"};
char DB[20]={"histosql\0"};
char USER[20]={"histosql\0"};
char PASS[20]={"histosql\0"};

#define flush(header) _CipFlushBuffer(header,24+header->Length);

/****************** functions declarations *************************/

void SigHand(int num,siginfo_t *info,void *data);
void Log(int level,char *format,...);
int mainprog(void);
int Logger(TAG *TAGs,int Tag_count);
int CheckBySql(void);
int CheckList(void);

int AddListe(LISTE *liste,void *data);
int RemoveListe(LISTE *liste,void *data);
int CompactListe(LISTE *liste);
//TAG *FindTag(LISTE *liste,char *tagname);
TAG *FindTag(LISTE *liste,int Id);

/******************* Global Var ************************************/

unsigned int Tux_errno;
unsigned int Tux_ext_errno;
int Tux_err_type;
char Tux_err_msg[MAX_ERR_MSG_LEN+1];

int DEAMON=0;
int debuglevel=LOG_WARNING;

jmp_buf jb;
int Terminated=0;
//int Restart=0;

int starttime=0;

TAG *TAGs=NULL;
LISTE tags;

/******************* Functions **************************************/

char *TuxGetInternalErrMsg(unsigned int ErrorCode)
{
	switch (ErrorCode){
		case T_Success:return("Success");
		case T_Error:return("Error");
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
	default:return(TuxGetInternalErrMsg(s_errno));
  //default :return(GetErrMsg(s_err_type,s_errno,ext_errno));
 }
}
/******************** DB Functions ******************/
int _InsertHisto(MYSQL *db,TAG *tag)
{
	char exec_str[255];
	//sprintf(exec_str,"insert into HISTO (TAGNAME,TIMEVALUE,DATAVALUE) values('%s',now(),'%f')",tag->TagName,tag->Value);
	sprintf(exec_str,"insert into HISTO (ID,TIMEVALUE,DATAVALUE) values('%d',now(),'%f')",tag->Id,tag->Value);
	mysql_real_query(db,exec_str,strlen(exec_str));
	return(_GetErrorCode(db,MysqlErrorMsg));
}
/******************** LISTE Functions ***************/
int AddListe(LISTE *liste,void *data)
{
	Log(LOG_DEBUG,"AddListe %p count : %d\n",data,liste->Count+1);
	if (liste->Count>=MAX_LISTE_SIZE) return(-1);
	liste->Data[liste->Count]=data;
	return(liste->Count++);
}
int RemoveListe(LISTE *liste,void *data)
{ int i;
	Log(LOG_DEBUG,"RemoveListe %p count : %d\n",data,liste->Count);
	for (i=0;i<liste->Count;i++)
		if (liste->Data[i]==data) liste->Data[i]=NULL;
	return(CompactListe(liste));
}
int CompactListe(LISTE *liste)
{ int i,j;
	for (i=0;i<MAX_LISTE_SIZE;i++)
	{
		if (liste->Data[i]==NULL)
			for (j=i+1;j<MAX_LISTE_SIZE;j++)
				liste->Data[j-1]=liste->Data[j];
	}
	j=0;
	for (i=0;i<MAX_LISTE_SIZE;i++)
		if (liste->Data[i]!=NULL) j++;
	liste->Count=j;
	Log(LOG_DEBUG,"CompactListe %d\n",liste->Count);
	return(j);
}
//TAG *FindTag(LISTE *liste,char *tagname)
TAG *FindTag(LISTE *liste,int Id)
{
	int i=0;
	//Log(LOG_DEBUG,"FindTag search for tag %d (%d tags in list)\n",Id,liste->Count);
	for (i=0;i<liste->Count;i++)
	{
		//if (!strcmp(tagname,((TAG*)(liste->Data[i]))->TagName))
		if (Id==((TAG*)(liste->Data[i]))->Id)
			return(liste->Data[i]);
	}
	return(NULL);
}
/********************** ***************************/
void SigHand(int num,siginfo_t *info,void *data)
{ usleep(100000);
	//
	if (info==NULL) Log(LOG_CRIT,"SigHand : %d\n",num);

	switch (info->si_signo)
	{
		case SIGTERM:	Log(LOG_NOTICE,"receive SIGTERM\n");
									Terminated=1;
									break;
		case SIGINT:	Log(LOG_NOTICE,"receive SIGINT\n");
									Terminated=1;
									break;
		case SIGIO:	Log(LOG_INFO,"receive SIGIO (%d / %d)\n",info->si_errno,info->si_code);
									//Terminated=1;
									break;
		case SIGSEGV:	Log(LOG_CRIT,"receive SIGSEGV, program ERROR (%d / %d)\n",info->si_errno,info->si_code);
									longjmp(jb,-1);
									//Terminated=1;
									//exit(1);
									break;
		case SIGUSR1:	Log(LOG_INFO,"receive SIGUSR1 (%d / %d)\n",info->si_errno,info->si_code);
									//Restart=1;
									break;
		default:	Log(LOG_CRIT,"receive signal: %d (%d / %d)\n",info->si_signo,info->si_errno,info->si_code);
									Terminated=1;
									break;
	}
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
				printf("%d : %s",time(NULL)-starttime,str);
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
	struct sigaction action;
	action.sa_sigaction=SigHand;
	action.sa_flags=SA_SIGINFO;
	sigaction(SIGTERM,&action,NULL);
	sigaction(SIGINT,&action,NULL);
	sigaction(SIGPOLL,&action,NULL);
	sigaction(SIGIO,&action,NULL);
	sigaction(SIGUSR1,&action,NULL);
	sigaction(SIGSEGV,&action,NULL);

	int c;

	while ((c=getopt(argc,argv,"dl:s:b:u:p:?h"))!=-1)
		switch(c)
		{
			case 'd':DEAMON=1;break;
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
			case '?':
			case 'h':
				{
					printf("Tuxhisto (Build on %s %s)\n",__DATE__,__TIME__);
					printf("usage: tuxhisto:[-d] [-l0-3] [-s] [-b] [-u] [-p] [-?,h]\n");
					printf("-d\tDaemonize Tuxhisto\n");
					printf("-l{0..3}\t(default :1)\n");
					printf("\t0\tLOG_ERR\n");
					printf("\t1\tLOG_WARNING (Default)\n");
					printf("\t2\tLOG_NOTICE\n");
					printf("\t3\tLOG_DEBUG\n");
					printf("-s\tDb host (Default :\"%s\")\n",DBHOST);
					printf("-b\tDb name (Default : \"%s\")\n",DB);
					printf("-u\tDb Username (Default : \"%s\")\n",USER);
					printf("-p\tDb Password\n");
					return(0);
				}
				break;
			default:break;
		}

	//printf("flags : %d %d\n",DEAMON,debuglevel);exit(0);
	starttime=time(NULL);
	if (DEAMON)
	{
		//openlog(LOG_TITLE,LOG_NDELAY,LOG_USER);
		openlog("Tuxhisto",LOG_NDELAY,LOG_USER);
	}

	Log(LOG_ALERT,"starting Tuxhisto, Database is %s on %s\n",DB,DBHOST);

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
								mainprog();
								Log(LOG_ALERT,"stopped\n");
								closelog();
								exit(0);
							}
		default : exit(0);
		}
	}else
	{
		mainprog();
		Log(LOG_ALERT,"stopped\n");
		exit(0);
	}
}
int mainprog(void)
{	int i=0;
	int res=OpenDb(DBHOST,USER,PASS,DB);
	if (res<0) {Log(LOG_CRIT,"OpenDb (%d) : %s\n",res,MysqlErrorMsg);return(res);} else
	{
		while (!Terminated)
		{
			if (i++>=SQLTOLIST) {Log(LOG_DEBUG,"CheckList\n");CheckList();i=0;} else	CheckBySql();
			sleep(1);
		}
		Log(LOG_NOTICE,"Exiting\n");
		CloseDb;
	}
	return(0);
}
int CheckBySql(void)
{
	int index=0,res=0,historise=0;
	TAG *tag=NULL;
	char *sel_str="select ID,TIME_REFRESH,TIME_CLEANING,HYSTERESIS,SNAPSHOT_VALUE,UNIX_TIMESTAMP(SNAPSHOT_TIME), \
	RECORDING,TIME_SAMPLE from DEFINITION";
	MYSQL_ROW row;

	res=mysql_real_query(&Default_Db,sel_str,strlen(sel_str));
	if (res)
	{
		_GetErrorCode(&Default_Db,MysqlErrorMsg);
		return(-1);
	}
	SqlResult=mysql_store_result(&Default_Db);
	if (SqlResult==NULL)
	{
		_GetErrorCode(&Default_Db,MysqlErrorMsg);
		return(-1);
	}
	while ((row = mysql_fetch_row(SqlResult)))
	{
		//long int last_time=0;
		int cleaning=0;
		//float last_value=0;
                //time_t temps=time (NULL);
		//unsigned long *lengths = mysql_fetch_lengths(SqlResult);
		if (row[0]==NULL) continue;
		tag=FindTag(&tags,atoi(row[0]));
		if (tag==NULL)
		{
			tag=malloc(sizeof(TAG));
			memset(tag,0,sizeof(TAG));
			AddListe(&tags,tag);
			//tag->exist=1;
			Log(LOG_NOTICE,"creating %s\n",row[0]);
		} else
		{
			//Log(LOG_DEBUG,"find %s (%d)\n",row[0],tag->Id);
		}

		tag->exist=1;
		//snprintf(tag->TagName,lengths[0]+1,"%s",row[0]);
		if (row[0]!=NULL)	tag->Id=atoi(row[0]);
		if (row[1]!=NULL)	tag->Time_Refresh=atoi(row[1]); else
			tag->Time_Refresh=MIN_SAMPLE;
		if (row[2]!=NULL)	cleaning=atoi(row[2]);
		if (row[3]!=NULL) tag->Hysteresis=atoi(row[3]);
		if (row[4]!=NULL) tag->Value=atof(row[4]);//SNAPSHOT_VALUE
		if (row[5]!=NULL) tag->Time_Value=atoi(row[5]);//SNAPSHOT_TIME en UNIX TIMESTAMP
                if (row[7]!=NULL)	tag->Time_Sample=atoi(row[7]);

                Log(LOG_DEBUG,"Tag id:%d t_refresh:%d t_sample:%d snap_value:%f snap_time:%d insert_time:%ld\n",
                        tag->Id,tag->Time_Refresh,tag->Time_Sample,tag->Value,tag->Time_Value,tag->Insert_Time_Value);
		if (cleaning)
		{
			//int del=_Execute(&Default_Db,"delete from HISTO where TAGNAME='%s' and (TIMEVALUE<(now() - interval %d day))",tag->TagName,cleaning);
			int del=_Execute(&Default_Db,"delete from HISTO where ID='%d' and (TIMEVALUE<(now() - interval %d day))",tag->Id,cleaning);
			/*if (del>=0) Log(LOG_DEBUG,"cleaning %s (%d rows deleted : %s)\n",row[0],del,MysqlErrorMsg);
				else Log(LOG_ERR,"Error on cleaning %s (%s)\n",row[0],MysqlErrorMsg);*/
		}
		if (tag->Time_Value>tag->Time_Sample+tag->Insert_Time_Value)
		{
			//Log(LOG_NOTICE,"historise %s : %f (%d) / %f (%d)\n",tag->TagName,last_value,last_time,tag->Value,tag->Time_Value);
			Log(LOG_NOTICE,"historise %d : %f (%d) / (%d)\n",tag->Id,tag->Value,tag->Time_Value,tag->Insert_Time_Value);
			//tag->Value=last_value;
			//tag->Time_Value=last_time;
                        tag->Insert_Time_Value=tag->Time_Value;
			int res2=InsertHisto(tag);
			if (!res2) Log(LOG_DEBUG,"insert %s on histo (%s)\n",row[0],MysqlErrorMsg);
				else Log(LOG_ERR,"Error on insert %s (%s)\n",row[0],MysqlErrorMsg);
		}
		/*if (last_value) historise=(100*(fabs(tag->Value-last_value)/fabs(last_value))>tag->Hysteresis);
			else historise=(tag->Value!=last_value);
		if (((last_time-tag->Time_Value)>=tag->Time_Refresh)||historise)
		{
			//Log(LOG_NOTICE,"historise %s : %f (%d) / %f (%d)\n",tag->TagName,last_value,last_time,tag->Value,tag->Time_Value);
			Log(LOG_NOTICE,"historise %d : %f (%d) / %f (%d)\n",tag->Id,last_value,last_time,tag->Value,tag->Time_Value);
			tag->Value=last_value;
			tag->Time_Value=last_time;
			int res2=InsertHisto(tag);
			if (!res2) Log(LOG_DEBUG,"insert %s on histo (%s)\n",row[0],MysqlErrorMsg);
				else Log(LOG_ERR,"Error on insert %s (%s)\n",row[0],MysqlErrorMsg);
		}*/
	}
	mysql_free_result(SqlResult);
	res=_GetErrorCode(&Default_Db,MysqlErrorMsg);
	if (res) return(res); else return(index);
}
int CheckList(void)
{
	int index=0;
	TAG *tag=NULL;

	Log(LOG_DEBUG,"CheckList %d tags\n",tags.Count);
	for (index=0;index<tags.Count;index++)
	{
		tag=tags.Data[index];
		if (tag==NULL) {Log(LOG_ERR,"Error CheckByList tag %d is NULL\n",index);continue;};
		if (!tag->exist)
		{
			//Log(LOG_NOTICE,"Removing tag %s (%d)\n",tag->TagName,index);
			Log(LOG_NOTICE,"Removing tag %d (%d)\n",tag->Id,index);
			free(tag);
			tags.Data[index]=NULL;
		}
		tag->exist=0;
	}
	CompactListe(&tags);
	return(0);
}
/*int CheckList(void)
{
	int index=0,res=0;
	TAG *tag=NULL;
	MYSQL_ROW row;

	for (index=0;index<tags.Count;index++)
	{
		tag=tags.Data[index];
		if (tag==NULL) {Log(LOG_ERR,"Error CheckByList tag %d is NULL\n",index);continue;};
		if (!_Select(&Default_Db,"select TAGNAME,TIME_REFRESH,TIME_CLEANING,HYSTERESIS,SNAPSHOT_VALUE,\
			UNIX_TIMESTAMP(SNAPSHOT_TIME),RECORDING from DEFINITION where TAGNAME='%s'",tag->TagName))
		{
			SqlResult=mysql_use_result(&Default_Db);
			if (SqlResult==NULL)
			{
				_GetErrorCode(&Default_Db,MysqlErrorMsg);
				return(-1);
			}
			if ((row = mysql_fetch_row(SqlResult))==NULL)
			{
				Log(LOG_NOTICE,"Removing tag %s (%d)\n",tag->TagName,index);
				free(tag);
				tags.Data[index]=NULL;
			}
			mysql_free_result(SqlResult);
			res=_GetErrorCode(&Default_Db,MysqlErrorMsg);
			if (res) Log(LOG_ERR,"Error CheckByList search tag %s : %s\n",tag->TagName,MysqlErrorMsg);
		} else Log(LOG_ERR,"Error CheckByList search tag %s : %s\n",tag->TagName,MysqlErrorMsg);
	}
	CompactListe(&tags);
	return(res);
}*/
