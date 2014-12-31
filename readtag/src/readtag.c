/***************************************************************************
 *  Copyright (C) 2006                                                     *
 *  Author : Stephane JEANNE	stephane.jeanne@gmail.com                  *
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

#include <tuxeip/TuxEip.h>

#include <time.h>
#include <unistd.h>
#ifdef _WIN32
	enum LOG_LEVELS {LOG_ALERT=1, LOG_CRIT, LOG_ERR, LOG_WARNING, LOG_NOTICE, LOG_INFO, LOG_DEBUG};
#else
	#include <syslog.h>
#endif
#include <sys/stat.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#ifdef __MINGW32__
	#define SIGIO 29
#endif

#define LOG_TITLE "ReadTag"
#define MAXPATHSIZE 100
#define MAXALIASSIZE 50
#define MAX_SAMPLE 1000 // timeout connection
#define flush(header) _CipFlushBuffer(header,24+header->Length);
#define TR_Error 1
#define TR_Ok 0

#define TRUE 1
#define FALSE 0

#define SUCCESS 0
#define ERROR 1

typedef struct _PLC
{	char PlcName[MAXALIASSIZE];
	char PlcPath[MAXPATHSIZE];
	Plc_Type PlcType;
	int NetWork;
	int Node;
} PLC;

/****** Global Var *************************/
unsigned int TR_errno;
unsigned int TR_ext_errno;
int TR_err_type;
char TR_err_msg[MAX_ERR_MSG_LEN+1];

int ReqId=0;
int Terminated=0;
int starttime=0;
int debuglevel=LOG_WARNING;
char AliasFile[MAXALIASSIZE]="/etc/tuxeip.conf\0";
int debug=0;
char writeValue[30]="\0";
int isWrite=0;

/**************** Functions ********************/
void Log(int level,char *format,...);
void SigHand(int num);
int GetPlcType(char *Type);
int GetNetType(char *NetWork);
int GetPlc(char *Alias,char *Aliasfile,PLC *plc);
int ParsePath(char *strpath,char Ip[],BYTE Path[]);
int Connect(PLC *plc,char *TagName, char *responseValue);
int ReadTag(PLC *Plc, Eip_Session *Session, Eip_Connection *Connection, char *TagName, int *DataType, char *responseValue);
int WriteTag(PLC *Plc, Eip_Session *Session, Eip_Connection *Connection, char *TagName, int DataType);
int GetSerial(void);


int GetSerial(void)
{
	return(getpid());
}

void Log(int level,char *format,...)
{
	va_list list;
	va_start(list,format);//NULL

if (level<=debuglevel)
	{
		if (debug==1)
		{	char str[255];
			if (format[0]!='\n')
			{
				vsprintf(str,format,list);
				printf("%ld : %s",(long)(time(NULL)-starttime),str);
			} else vprintf(format,list);
		}
		else
		{ 
#ifndef _WIN32
			vsyslog(level,format,list);
#endif
		}
	}
	va_end(list);
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

int GetPlcType(char *Type)
{
	if (!strncasecmp(Type,"PLC",strlen(Type))) return(PLC5);
		else
		{
			if (!strncasecmp(Type,"SLC",strlen(Type))) return(SLC500);
			else 
			{
				if (!strncasecmp(Type,"LGX",strlen(Type))) return(LGX);
				else return(Unknow);
			}
		}		
}

int GetNetType(char *NetWork)
{
	if (!strncasecmp(NetWork,"DHP",strlen(NetWork))) return(1);
		else 
		{
			if (!strncasecmp(NetWork,"DHP_A",strlen(NetWork))) return(1);
			else 
			{
				if (!strncasecmp(NetWork,"DHP_B",strlen(NetWork))) return(2);
				else return(0);
			}
		}	
}

int GetPlc(char *Alias,char *Aliasfile,PLC *plc)
{	char Temp_Alias[MAXPATHSIZE];
	char Path[MAXPATHSIZE];
	char Type[MAXALIASSIZE];
	char NetWork[MAXALIASSIZE];
	int Node=0;
	FILE *fp=fopen(Aliasfile,"r");
	if (fp!=NULL)
	{
		while (!feof(fp))
		{
			if (fscanf(fp,"%30s %50s %10s %10s %d",Temp_Alias,Path,Type,NetWork,&Node)==5)
			{
				if (!strncasecmp(Temp_Alias,Alias,strlen(Alias))) // find
				{
					Log(LOG_DEBUG,"%s = %s (%s,%s,%d)\n",Alias,Path,Type,NetWork,Node);
					memset(plc,0,sizeof(PLC));
					strncpy(plc->PlcName,Temp_Alias,sizeof(plc->PlcName));
					strncpy(plc->PlcPath,Path,sizeof(plc->PlcPath));
					plc->PlcType=GetPlcType(Type);
					plc->NetWork=GetNetType(NetWork);
					plc->Node=Node;
					fclose(fp);
					return(TR_Ok);
				}
			}
		}
		fclose(fp);
		return(TR_Error);
	} else
	{
		Log(LOG_WARNING,"Alias file %s : %s\n",Aliasfile,strerror(errno));
		return(TR_Error);
	}
}

int ParsePath(char *strpath,char Ip[],BYTE Path[])
{ int index=0,len=0;
	len=strlen(strpath)+1;
	char *str=malloc(len);
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

int Connect(PLC *Plc,char *TagName, char *responseValue)
{
	int result=SUCCESS;
	int path_size=0;
	char ip[16];
	BYTE path[40];
	Eip_Session *Session=NULL;
	Eip_Connection *Connection=NULL;
	int dataType;
	
	Log(LOG_DEBUG,"[Connect] Building Session for %s\n",Plc->PlcName);
	path_size=ParsePath(Plc->PlcPath,ip,path);
	if (path_size>0)
	{
		Session=OpenSession(ip);
		if (Session!=NULL)
		{
			if (RegisterSession(Session)<0)
			{
				CloseSession(Session);
				Log(LOG_CRIT,"[Connect] Unable to register session for Plc: %s (%s) \n",Plc->PlcName,cip_err_msg);
				return ERROR;
			}
		} else
		{
			Log(LOG_CRIT,"[Connect] Unable to open session for Plc: %s (%s)\n",Plc->PlcName,cip_err_msg);
			return ERROR;
		}
		if (Plc->NetWork)
			Connection=ConnectPLCOverDHP(Session,Plc->PlcType,(int)Session,GetSerial(),MAX_SAMPLE,Plc->NetWork,path,path_size);
		else
			Connection=ConnectPLCOverCNET(Session,Plc->PlcType,(int)Session,GetSerial(),MAX_SAMPLE,path,path_size);
		if (Connection!=NULL)
		{
			Log(LOG_DEBUG,"[Connect] Connection (%p) created for PLC : %s (%s) )\n",Connection,Plc->PlcName,cip_err_msg);
		} else 
		{
			Log(LOG_CRIT,"[Connect] Unable to create connection for Plc: %s (%s)\n",Plc->PlcName,cip_err_msg);
			return ERROR;
		}
	} else Log(LOG_ERR,"[Connect] Invalid path : %s\n",Plc->PlcPath);

	Log(LOG_DEBUG,"[Connect] Connect : %s [%s](%p / %p)\n",TagName,writeValue,Session,Connection);
	if (ReadTag(Plc, Session, Connection, TagName, &dataType, responseValue)!=SUCCESS)
		return ERROR;
	else
	{
		if (isWrite)
		{
			if (WriteTag(Plc, Session, Connection, TagName,dataType)!=SUCCESS)
				return ERROR; 
			Log(LOG_DEBUG,"[Connect] %s [%s] %x (%p / %p)\n",TagName,writeValue,dataType,Session,Connection);
			if (ReadTag(Plc, Session, Connection, TagName, &dataType, responseValue)!=SUCCESS)
				return ERROR;
		}
	}
	return result;
}

int ReadTag(PLC *Plc, Eip_Session *Session, Eip_Connection *Connection, char *TagName, int *DataType, char *responseValue)
{
	int result=SUCCESS;
	DHP_Header dhp={0,0,0,0};
	int tns=getpid();
	float FValue;
	int IValue;
	
	Log(LOG_DEBUG,"[ReadTag] %s [%s](%p / %p)\n",TagName,writeValue,Session,Connection);
	switch (Plc->PlcType)
	{
		case PLC5:
		case SLC500:
		{ 
			PLC_Read *data;
			dhp.Dest_adress=Plc->Node;
			if (Plc->NetWork) // DHP
				data=ReadPLCData(Session,Connection,&dhp,NULL,0,Plc->PlcType,tns++,TagName,1);
			else data=ReadPLCData(Session,Connection,NULL,NULL,0,Plc->PlcType,tns++,TagName,1);
			if (data!=NULL)
			{
				Log(LOG_DEBUG,"[ReadTag] %s on %s Datatype:%x (%s)\n",TagName,Plc->PlcName,data->type,cip_err_msg);
				*DataType = data->type;
				switch (data->type)
				{
					case PLC_BIT:
					case PLC_INTEGER:
					// TODO case PLC_TIMER:
					{
						IValue=PCCC_GetValueAsBoolean(data,0);
						if (!cip_errno) 
						{
							Log(LOG_DEBUG,"[ReadTag] %s on %s = %d (%s)\n",TagName,Plc->PlcName,IValue,cip_err_msg);
							sprintf(responseValue,"%d",IValue);
							result=SUCCESS;
						}	else 
						{
							Log(LOG_WARNING,"[ReadTag] Get PCCC value on tag %s: (%d / ext %d) %s\n",TagName,cip_errno,cip_err_msg,cip_ext_errno);
							result=ERROR;
						}	
					}break;
					case PLC_FLOATING:
					{
						FValue=PCCC_GetValueAsFloat(data,0);
						if (!cip_errno) 
						{
							Log(LOG_DEBUG,"[ReadTag] %s on %s = %f (%s)\n",TagName,Plc->PlcName,FValue,cip_err_msg);
							sprintf(responseValue,"%f",FValue);
							result=SUCCESS;
						}	else 
						{
							Log(LOG_WARNING,"[ReadTag] Get PCCC value on tag %s: (%d / ext %d) %s\n",TagName,cip_errno,cip_err_msg,cip_ext_errno);
							result=ERROR;
						}
					}break;
					default:{
						Log(LOG_WARNING,"[ReadTag] Datatype type unknown for : %s\n",TagName);
						result=ERROR;
					}
					break;
				}
				free(data);
			} else
			{
				Log(LOG_WARNING,"[ReadTag] ReadPLCData error on tag %s: (%d) %s\n",TagName,cip_errno,cip_err_msg);
				result=ERROR;
			}
		}; break;
		case LGX:
		{
			LGX_Read *data=ReadLgxData(Session,Connection,TagName,1);
			if (data!=NULL)
			{
				Log(LOG_DEBUG,"[ReadTag] %s on %s Datatype:%x (%s)\n",TagName,Plc->PlcName,data->type,cip_err_msg);
				*DataType = data->type;
				switch(data->type)
				{
					case LGX_BOOL:
						{
							IValue=GetLGXValueAsInteger(data,0);
							if (!cip_errno) 
							{
								if (IValue!=0) IValue=1;
								Log(LOG_DEBUG,"[ReadTag] %s on %s = %d (%s)\n",TagName,Plc->PlcName,IValue,cip_err_msg);
								sprintf(responseValue,"%d",IValue);
								result=SUCCESS;
							}	else 
							{
								Log(LOG_WARNING,"[ReadTag] Get value : (%d) %s\n",cip_errno,cip_err_msg);
								result=ERROR;
							}
						}break;
					case LGX_BITARRAY:
						{
							//
						}break;
					case LGX_SINT:
					case LGX_INT:
					case LGX_DINT:
						{
							IValue=GetLGXValueAsInteger(data,0);
							if (!cip_errno) 
							{
								Log(LOG_DEBUG,"[ReadTag] %s on %s = %d (%s)\n",TagName,Plc->PlcName,IValue,cip_err_msg);
								sprintf(responseValue,"%d",IValue);
								result=SUCCESS;
							}	else 
							{
								Log(LOG_WARNING,"[ReadTag] Get value : (%d) %s\n",cip_errno,cip_err_msg);
								result=ERROR;
							}
						}break;
					case LGX_REAL:
						{
							FValue=GetLGXValueAsFloat(data,0);
							if (!cip_errno) 
							{
								Log(LOG_DEBUG,"[ReadTag] %s on %s = %f (%s)\n",TagName,Plc->PlcName,FValue,cip_err_msg);
								sprintf(responseValue,"%f",FValue);
								result=SUCCESS;
							}	else 
							{
								Log(LOG_WARNING,"[ReadTag] Get value : (%d) %s\n",cip_errno,cip_err_msg);
								result=ERROR;
							}
						}break;
				}
				free(data);
			} else 
			{
				Log(LOG_WARNING,"[ReadTag] ReadLgxData error on tag : %s (%s)\n",TagName,cip_err_msg);
				result=ERROR;
			}
		}; break;
		default:{
			Log(LOG_WARNING,"[ReadTag] Plc type unknown for : %s\n",Plc->PlcName);
				result=ERROR;
			}
			break;
	}
	return result;
}

int WriteTag(PLC *Plc, Eip_Session *Session, Eip_Connection *Connection, char *TagName, int DataType)
{
	int result=SUCCESS;
	DHP_Header dhp={0,0,0,0};
	int tns=getpid();
	float FValue;
	int IValue;
	
	Log(LOG_DEBUG,"[WriteTag] Start WriteTag %s = %s (%p / %p)\n",TagName,writeValue,Session,Connection);
	switch (Plc->PlcType)
	{
		case PLC5:
		case SLC500:
		{ 
			dhp.Dest_adress=Plc->Node;
			switch(DataType)
			{ //TODO one bit b3:0/4
				case PLC_BIT:
				case PLC_INTEGER:
				{
					IValue=atoi(writeValue);
					if (Plc->NetWork) // DHP
						if (WritePLCData(Session,Connection,&dhp,NULL,0,Plc->PlcType,tns++,TagName,DataType,&IValue,1)>Error)
							result=SUCCESS;
						else
							result=ERROR;
					else
					if (WritePLCData(Session,Connection,NULL,NULL,0,Plc->PlcType,tns++,TagName,DataType,&IValue,1)>Error)
							result=SUCCESS;
						else
							result=ERROR;
					Log(LOG_DEBUG,"[WriteTag] %s on %s = %x (%s) [%x]\n",TagName,Plc->PlcName,IValue,cip_err_msg,result);
				}break;
				case PLC_FLOATING:
				{
					FValue=atof(writeValue);
					if (Plc->NetWork) // DHP
						if (WritePLCData(Session,Connection,&dhp,NULL,0,Plc->PlcType,tns++,TagName,DataType,&FValue,1)>Error)
							result=SUCCESS;
					else
						result=ERROR;
					else
						if (WritePLCData(Session,Connection,NULL,NULL,0,Plc->PlcType,tns++,TagName,DataType,&FValue,1)>Error)
							result=SUCCESS;
					else
						result=ERROR;
					Log(LOG_DEBUG,"[WriteTag] %s on %s = %f (%s) [%x]\n",TagName,Plc->PlcName,FValue,cip_err_msg,result);
				}break;
				default:
				{
					Log(LOG_WARNING,"[WriteTag] Datatype unknown for : %s\n",TagName);
					result=ERROR;
				}
				break;
			}
		}; break;
		case LGX:
		{
			switch(DataType)
			{
				case LGX_BOOL:
				{
					IValue=atoi(writeValue);
					if (IValue!=0) IValue=1;
					if (WriteLgxData(Session,Connection,TagName,DataType,&IValue,1)>0)
						result=SUCCESS;
					else
						result=ERROR;
					Log(LOG_DEBUG,"[WriteTag] %s on %s = %x (%s) [%x]\n",TagName,Plc->PlcName,IValue,cip_err_msg,result);
				}break;
				case LGX_BITARRAY:
				{
					//
				}break;
				case LGX_SINT:
				case LGX_INT:
				case LGX_DINT:
				{
					IValue=atoi(writeValue);
					if (WriteLgxData(Session,Connection,TagName,DataType,&IValue,1)>0)
						result=SUCCESS;
					else
						result=ERROR;
					Log(LOG_DEBUG,"[WriteTag] %s on %s = %x (%s) [%x]\n",TagName,Plc->PlcName,IValue,cip_err_msg,result);
				}break;
				case LGX_REAL:
				{
					FValue=atof(writeValue);
					Log(LOG_DEBUG,"[WriteTag] %s on %s = %f \n",TagName,Plc->PlcName,FValue);
					if (WriteLgxData(Session,Connection,TagName,LGX_REAL,&FValue,1)>0)
						result=SUCCESS;
					else
						result=ERROR;
					Log(LOG_DEBUG,"[WriteTag] %s on %s = %f (%s) [%x]\n",TagName,Plc->PlcName,FValue,cip_err_msg,result);
				}break;
				default:
				{
					Log(LOG_WARNING,"[WriteTag] Datatype unknown for : %s\n",TagName);
					result=ERROR;
				}
				break;
			}
		}; break;
		default:
		{
			Log(LOG_WARNING,"[WriteTag] Plc type unknown for : %s\n",Plc->PlcName);
			result=ERROR;
		}
		break;
	}
	return result;
}

int main (int argc,char *argv[])
{	
	// Gestion des signaux
	signal(SIGTERM,SigHand);
	signal(SIGINT,SigHand);
	signal(SIGSEGV,SigHand);

	int c;
	PLC plc={"","",LGX,0,0}; // LGX on CNET
	char Alias[MAXALIASSIZE]="\0";
	char TagName[MAXALIASSIZE]="\0";
	char responseValue[30]="";
	
	while ((c=getopt(argc,argv,"dl:f:w:a:p:c:r:n:?h"))!=EOF)
		switch(c)
		{
			case 'd': //debug mode
				{
					debug=1;	
				}break;
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
			case 'f': //Alias file
				{
					strcpy(AliasFile,optarg);
				}break;
			case 'w': //Value to write
				{
					strcpy(writeValue,optarg);
					isWrite = TRUE;
				}break;
			case 'a': //Alias
				{
					strcpy(Alias,optarg);
				}break;
			case 'p': //Path
				{
					strcpy(plc.PlcPath,optarg);
				}break;
			case 'c': //Plc Type
				{
					plc.PlcType=GetPlcType(optarg);
				};break;
			case 'r': //Network Type
				{
					plc.NetWork=GetNetType(optarg);
				};break;
			case 'n': //Node address
				{
					plc.Node=atoi(optarg);
				};break;
			case '?':
			case 'h':
				{
					printf("%s (Build on %s %s)\n",LOG_TITLE,__DATE__,__TIME__);
					printf("usage: %s:[-d] [-l0-3] [-f Alias file] [-w Value] [-?,h] {-a alias || -p path -c Plc type -r network type -n node address} var\n",argv[0]);
					printf("-d\tDebug mode to screen\n");
					printf("-l{0..3}\t(default :1)\n");
					printf("\t0\tLOG_ERR\n");
					printf("\t1\tLOG_WARNING (Default)\n");
					printf("\t2\tLOG_NOTICE\n");
					printf("\t3\tLOG_DEBUG\n");
					printf("-f\tAlias file (Default :\"%s\")\n",AliasFile);
					printf("-w\tValue to write\n");
					printf("-a\tAlias name\n");
					printf("-p\tPath \n");
					printf("-c\tPlc type {LGX,PLC,SLC} (Default : LGX)\n");
					printf("-r\tNetwork type {CNET,DHP_A,DHP_B} (Default : CNET)\n");
					printf("-n\tNode address (Default : 0)\n");
					return(0);
				}break;
			default:break;
		}
	if (optind < argc)
	{
		strcpy(TagName,argv[optind]);
	}
	
	starttime=time(NULL);

	Log(LOG_ALERT,"[main] starting...Pid=%d\n",getpid());
	if (strlen(Alias)>0)
	{
		if (GetPlc(Alias,AliasFile,&plc)!=TR_Ok)
		{
			Log(LOG_ERR,"[main] Alias not found\n");
			exit(1);
		}
	}
	// affichage test
	Log(LOG_DEBUG,"[main] AliasFile : %s\n",AliasFile);
	Log(LOG_DEBUG,"[main] Alias : %s\n",Alias);
	Log(LOG_DEBUG,"[main] plc.PlcPath : %s\n",plc.PlcPath);
	Log(LOG_DEBUG,"[main] plc.PlcType : %d\n",plc.PlcType);
	Log(LOG_DEBUG,"[main] plc.NetWork : %d\n",plc.NetWork);
	Log(LOG_DEBUG,"[main] plc.Node : %d\n",plc.Node);
	Log(LOG_DEBUG,"[main] TagName : %s\n",TagName);

	if (Connect(&plc, TagName, responseValue)==SUCCESS) 
	{	
		printf("%s\n",responseValue);
		Log(LOG_ALERT,"[main] stopped\n");
		exit(SUCCESS);
	} else 
	{
		printf("ERR\n");
		Log(LOG_ALERT,"[main] stopped\n");
		exit(ERROR);
	}

}
