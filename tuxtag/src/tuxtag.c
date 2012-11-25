/***************************************************************************
 *  Copyright (C) 2006                                                     *
 *  Author : Stephane JEANNE    stephane.jeanne at gmail.com               *
 *           Stephane LEICHT    stephane at leicht.fr                      *
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


#define INCLUDE_SRC

#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <syslog.h>

#include <tuxeip/TuxEip.h>
#include <sutil/ChainList.h>

#include "tuxtag.h"
#include "cJSON.h"


#define space printf("\n");
#define LOG_TITLE "TuxTag"
#define DEF_UPDATE_RATE 30; //
#define DEF_INACTIVITY_TO 90;
#define WAITING 5 // Gardian

#define flush(header) _CipFlushBuffer(header,24+header->Length);

/******************* Global Var ************************************/

unsigned int Tux_errno;
unsigned int Tux_ext_errno;
int Tux_err_type;
char Tux_err_msg[MAX_ERR_MSG_LEN+1];

int DEAMON=0;
int debuglevel=LOG_WARNING;
int UPDATE_RATE=DEF_UPDATE_RATE;
int INACTIVITY_TO=DEF_INACTIVITY_TO;
int SPort=17560;
char Alias[50]="/etc/tuxeip.conf";

jmp_buf jb;
int Terminated=0;

int starttime=0;
char tempbuf[MAXBUFFERSIZE];
LISTE PLCs;
LISTE TAGs;
LISTE CLIENTs;
LISTE SESSIONs;
LISTE CONNECTIONs;

int Serial=0;
int tns;
int SetFD(int FD);
int Server=-1;
fd_set fd_clients,fd_plc;

/******************* Functions **************************************/

char *TuxGetInternalErrMsg(unsigned int ErrorCode)
{
	switch (ErrorCode){
		case Success:return("Success");
		case Error:return("Error");
		/*case :return("");
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
int GetPlc(LISTE *PLC_List,char *Aliasfile)
{	char Alias[30+1];
	char Path[50+1];
	char Type[10+1];
	char NetWork[10+1];
	int Node=0;
	PLC *plc;
	FILE *fp=fopen(Aliasfile,"r");
	if (fp!=NULL)
	{
		while (!feof(fp))
		{
			if (fscanf(fp,"%30s %50s %10s %10s %d",Alias,Path,Type,NetWork,&Node)==5)
			{
				Log(LOG_DEBUG,"%s = %s (%s,%s,%d)\n",Alias,Path,Type,NetWork,Node);
				plc=malloc(sizeof(PLC));
				if (plc==NULL)
				{
					Log(LOG_WARNING,"GetPlc error (%d)\n",__LINE__);
				} else
				{
					bzero(plc,sizeof(PLC));
					strncpy(plc->PlcName,Alias,sizeof(plc->PlcName));
					strncpy(plc->PlcPath,Path,sizeof(plc->PlcPath));
					if (!strncasecmp(Type,"PLC",strlen(Type))) plc->PlcType=PLC5;
						else
						{
							if (!strncasecmp(Type,"SLC",strlen(Type))) plc->PlcType=SLC500;
							else 
							{
								if (!strncasecmp(Type,"LGX",strlen(Type))) plc->PlcType=LGX;
								else plc->PlcType=Unknow;
							}
						}	
					if (!strncasecmp(NetWork,"DHP",strlen(NetWork))) plc->NetWork=1;
						else 
						{
							if (!strncasecmp(NetWork,"DHP_A",strlen(NetWork))) plc->NetWork=1;
							else 
							{
								if (!strncasecmp(NetWork,"DHP_B",strlen(NetWork))) plc->NetWork=2;
								else plc->NetWork=0;
							}
						}
					plc->Node=Node;
					AddChListe(PLC_List,plc);
				}
			}
		}
		fclose(fp);
		return(PLC_List->Count);
	} else
	{
		Log(LOG_WARNING,"Alias file %s : %s\n",Aliasfile,strerror(errno));
		return(T_Error);
	}
}

int SetCoe(int fd) /* set Close On Exit*/
{
	int res=fcntl(fd,F_SETFD,FD_CLOEXEC);
	Log(LOG_DEBUG,"Set COE for %d : %s (%d)\n",fd,strerror(errno),res);
	return(res);
}

void CloseList(LISTE *List)
{	
	if (List==NULL) return;
	ELEMENT *elt;
	if (List->Count<=0) return;
	while ((elt=GetFirst(List))!=NULL)
	{
		if (elt->Data!=NULL) free(elt->Data);
		RemoveChListe_Ex(List,elt);
		free(elt);
	}
	return;
}
void Affiche_TAGs(int fd)
{
	Reply(fd,"- %d Tags\n",TAGs.Count);
	if (TAGs.Count<=0) return;
	ELEMENT *elt=GetFirst(&TAGs);
	if (elt!=NULL) do
	{
		TAG *tag=elt->Data;
		Reply(fd,"Tag %p : %s \t\t(Plc %p)\n",tag,tag->TagName,tag->Plc);
	} while ((elt=GetNext(&TAGs,elt))!=NULL);
}
void Affiche_PLCs(int fd)
{
	Reply(fd,"- %d Plcs\n",PLCs.Count);
	if (PLCs.Count<=0) return;
	ELEMENT *elt=GetFirst(&PLCs);
	if (elt!=NULL) do
	{
		PLC *plc=elt->Data;
		Reply(fd,"Plc %p : %s \t\t(%p/%p) %d ref\n",plc,plc->PlcName,plc->Session,plc->Connection,plc->References);
	} while ((elt=GetNext(&PLCs,elt))!=NULL);
}
void Affiche_Connections(int fd)
{
	Reply(fd,"- %d Connections\n",CONNECTIONs.Count);
	if (CONNECTIONs.Count<=0) return;
	ELEMENT *elt=GetFirst(&CONNECTIONs);
	if (elt!=NULL) do
	{
		Eip_Connection *conn=elt->Data;
		Reply(fd,"Connection %p : %d \t\t %d ref\n",conn,conn->ConnectionSerialNumber,conn->References);
	} while ((elt=GetNext(&CONNECTIONs,elt))!=NULL);
}
void Affiche_Sessions(int fd)
{
	Reply(fd,"- %d Sessions\n",SESSIONs.Count);
	if (SESSIONs.Count<=0) return;
	ELEMENT *elt=GetFirst(&SESSIONs);
	if (elt!=NULL) do
	{
		Eip_Session *ses=elt->Data;
		Reply(fd,"Sessions %p : %d \t\t %d ref\n",ses,ses->sock,ses->References);
	} while ((elt=GetNext(&SESSIONs,elt))!=NULL);
}
void Affiche_Clients(int fd)
{
	Reply(fd,"- %d Clients\n",CLIENTs.Count);
	if (CLIENTs.Count<=0) return;
	ELEMENT *elt=GetFirst(&CLIENTs);
	if (elt!=NULL) do
	{
		CLIENT *client=elt->Data;
		Reply(fd,"Clients %p : %d\n",client,client->FD);
	} while ((elt=GetNext(&CLIENTs,elt))!=NULL);
}
void Affiche(int fd,char *listname)
{
	if (strncasecmp(listname,"all",strlen(listname))==0)
	{
		Affiche_Sessions(fd);
		Affiche_Connections(fd);
		Affiche_Clients(fd);
		Affiche_TAGs(fd);
		Affiche_PLCs(fd);
	}
	if (strncasecmp(listname,"session",strlen(listname))==0) Affiche_Sessions(fd);
	if (strncasecmp(listname,"connection",strlen(listname))==0) Affiche_Connections(fd);
	if (strncasecmp(listname,"client",strlen(listname))==0) Affiche_Clients(fd);
	if (strncasecmp(listname,"tag",strlen(listname))==0) Affiche_TAGs(fd);
	if (strncasecmp(listname,"plc",strlen(listname))==0) Affiche_PLCs(fd);
}
/****************************************************/
int OpenServerSock(int portnum)
{	struct sockaddr_in monadr;
	int opt=1;
	int sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock<0) return(-1);
	/* option de réutilisation d'adresse */
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,(char *) &opt, sizeof(opt));
	/* init socket serveur */
	bzero( (char *)&monadr, sizeof monadr);
	monadr.sin_family = AF_INET;
	monadr.sin_port = htons(portnum);
	monadr.sin_addr.s_addr = INADDR_ANY;

	if( bind(sock, (struct sockaddr *)&monadr, sizeof(monadr)) == -1 ) 
	{
		close(sock);
		return(-1);
	}
	/* mise en ecoute de notre socket */
	if( listen(sock,1) == -1 ) 
	{
		close(sock);
		return(-1);
	}
	return(sock);
}
/********************** ***************************/
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
int ParsePath(char *strpath,char Ip[],char Path[])
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

int ParseRequest(char *Alias,char *Tag,double *pWriteValue, char *requete, char *pIsWrite, int *timeRefresh) {
	char *varvalue;
	double FValue;
        int Itime;
	char Action[6];
        *pIsWrite = FALSE;
	Log(LOG_DEBUG,"Entering ParseRequest (%s)[%i]\n",requete,strlen(requete));
	cJSON *json = cJSON_Parse(requete);
	cJSON *tag = cJSON_GetObjectItem(json,"tag");// object tag
	if (!tag) {
		return ERROR;
	}
	if (cJSON_GetObjectItem(tag,"tagname")) {
		strcpy(Tag,get_string(cJSON_GetObjectItem(tag,"tagname")));
		Log(LOG_DEBUG,"Result ParseRequest tagname=%s\n",Tag);
	}
	if (cJSON_GetObjectItem(tag,"plcname")) {
		strcpy(Alias,get_string(cJSON_GetObjectItem(tag,"plcname")));
		Log(LOG_DEBUG,"Result ParseRequest plcname=%s\n",Alias);
	}
        if (cJSON_GetObjectItem(tag,"timerefresh")) {
		cJSON *Wtime = cJSON_GetObjectItem(tag,"timerefresh");
                Itime = cJSON_Get_int(Wtime);
	} else {
            Itime=0;
        }
        *timeRefresh = Itime;
        Log(LOG_DEBUG,"Result ParseRequest timerefresh=%d\n",Itime);
	if (cJSON_GetObjectItem(tag,"action")) {
		strcpy(Action,get_string(cJSON_GetObjectItem(tag,"action")));
		Log(LOG_DEBUG,"Result ParseRequest action=%s\n",Action);
		if (strcmp(Action,"write")==0) {
			if (cJSON_GetObjectItem(tag,"value")) {
                                cJSON *Wvalue = cJSON_GetObjectItem(tag,"value");
                                FValue = cJSON_Get_double(Wvalue);
                                *pWriteValue = FValue;
                                *pIsWrite = TRUE; 
                                Log(LOG_DEBUG,"Result ParseRequest value=%f\n",FValue);
        		}
		}
	}
	cJSON_Delete(json);
	return SUCCESS;
}

Eip_Session *FindSession(char *Ip,LISTE *PLC_List)
{	
	if (PLC_List->Count<=0) return(NULL);
	int path_size;
	char ip[16],path[40];
	ELEMENT *elt=GetFirst(PLC_List);
	if (elt!=NULL) do
	{	
		PLC *plc=elt->Data;
		if (plc->Session==NULL) continue;
		path_size=ParsePath(plc->PlcPath,ip,path);
		if (path_size<=0) continue;
		if (!strncmp(ip,Ip,strlen(Ip))) return(plc->Session);
	} while ((elt=GetNext(PLC_List,elt))!=NULL);
	return(NULL);
}
Eip_Connection *FindConnection(char *Path,LISTE *PLC_List)
{	
	if (PLC_List->Count<=0) return(NULL);
	ELEMENT *elt=GetFirst(PLC_List);
	if (elt!=NULL) do
	{
		PLC *plc=elt->Data;
		if (plc->Connection==NULL) continue;
		if (!strncmp(plc->PlcPath,Path,strlen(Path))) return(plc->Connection);
	} while ((elt=GetNext(PLC_List,elt))!=NULL);
	return(NULL);
}
CLIENT *FindClient(int fd,LISTE *CLIENT_List)
{
	if (CLIENT_List->Count<=0) return(NULL);
	ELEMENT *elt=GetFirst(CLIENT_List);
	if (elt!=NULL) do
	{
		CLIENT *client=elt->Data;
		if (client->FD==fd) return(client);
	} while ((elt=GetNext(CLIENT_List,elt))!=NULL);
	return(NULL);	
}
PLC *FindPLC(char *PlcName,LISTE *PLC_List)
{
	if (PLC_List->Count<=0) return(NULL);
	ELEMENT *elt=GetFirst(PLC_List);
	if (elt!=NULL) do
	{
		PLC *plc=elt->Data;
		if (!strncasecmp(PlcName,plc->PlcName,strlen(plc->PlcName))) return(plc);
	} while ((elt=GetNext(PLC_List,elt))!=NULL);
	return(NULL);	
}
TAG *FindTag(char *TagName,char *PlcName,LISTE *TAG_List)
{
	if (TAG_List->Count<=0) return(NULL);
	if ((TagName==NULL)||(PlcName==NULL)) return(NULL);
	ELEMENT *elt=GetFirst(TAG_List);
	if (elt!=NULL) do
	{
		TAG *tag=elt->Data;
		if ((strcasecmp(TagName,tag->TagName)==0)&&(tag->Plc!=NULL)&&
			(!strncasecmp(PlcName,tag->Plc->PlcName,strlen(tag->Plc->PlcName)))) return(tag);
	} while ((elt=GetNext(TAG_List,elt))!=NULL);
	return(NULL);	
}

int BuildSession(PLC *plc)
{	int path_size=0;
	char ip[16],path[40];
	Eip_Session *session=NULL;
	
	Log(LOG_DEBUG,"Building Session for %s (%d sessions)\n",plc->PlcName,SESSIONs.Count);
	
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
					if (RegisterSession(session)<0)
					{
						CloseSession(session);
						Log(LOG_CRIT,"Unable to register session for Plc: %s (%s) \n",plc->PlcName,cip_err_msg);
					}
					AddChListe(&SESSIONs,session);
					session->References=1;
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
				session->References++;
				Log(LOG_DEBUG,"%s Sharing Session (%p) with another PLC (%d references)\n",plc->PlcName,session,session->References);
				return(1);
			}
		} else return(0);
	} else return(0);
}

int BuildConnection(PLC *plc)
{	int path_size=0;
	char ip[16],path[40];
	
	Log(LOG_DEBUG,"Building connection for %s (%d connections)\n",plc->PlcName,CONNECTIONs.Count);
	
	if ((plc==NULL)||(plc->Connection!=NULL)) return(0);
	
	Eip_Session *session=plc->Session;
	Eip_Connection *connection=plc->Connection;
	
	path_size=ParsePath(plc->PlcPath,ip,path);
	if (path_size>0)
	{ // Creating Sessions
		if (plc->Connection==NULL)
		{
			// Creating Connections
			connection=FindConnection(plc->PlcPath,&PLCs);
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
					connection->References=1;
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
				connection->References++;
				Log(LOG_DEBUG,"%s Sharing Connection (%p) with another PLC (%d references)\n",plc->PlcName,connection,connection->References);
				return(1);
			}
		} else return(0);
	} else 
	{
		Log(LOG_CRIT,"Error while parsing IP/Path for Plc : %s\n",plc->PlcName);
		return(0);
	}
}
int DisconnectPlc(PLC *plc)
{
	if (plc==NULL) return(0);
	Log(LOG_DEBUG,"Entering DisconnectPlc (%s)\n",plc->PlcName);
	if (plc->Connection!=NULL)
	{
		Log(LOG_DEBUG,"DisconnectPlc : Connection %p (%d References)\n",plc->Connection,plc->Connection->References);
		plc->Connection->References--;
		if (plc->Connection->References<=0)
		{
			Log(LOG_DEBUG,"DisconnectPlc : No more Reference to Connection %p, closing\n",plc->Connection);
			KillConnection(plc->Connection);
		}
	}
	if (plc->Session!=NULL)
	{
		Log(LOG_DEBUG,"DisconnectPlc : Session %p (%d References)\n",plc->Session,plc->Session->References);
		plc->Session->References--;
		if (plc->Session->References<=0)
		{
			Log(LOG_DEBUG,"DisconnectPlc : No more Reference to Session %p, closing\n",plc->Session);
			KillSession(plc->Session);
		}
	}
	plc->Connection=NULL;
	plc->Session=NULL;
	return(0);
}
int BuildConnections(LISTE *PLC_List)
{	
	ELEMENT *elt=GetFirst(PLC_List);
	if (elt!=NULL) do
	{	
		PLC *plc=elt->Data;
		if (BuildSession(plc)) BuildConnection(plc);
	} while ((elt=GetNext(PLC_List,elt))!=NULL);
	return(CONNECTIONs.Count);
}
int CheckSession(PLC *plc)
{
	Eip_Session *session=plc->Session;
	Log(LOG_WARNING,"Checking Session for %s\n",plc->PlcName);
	
	if (session==NULL) return (BuildSession(plc));
	ListServices_Reply *Services=ListServices(session);
	if (Services==NULL)
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
			if (elt!=NULL) do
			{
				PLC *plc=elt->Data;
				if (plc->Session==session) plc->Session=NULL;
			} while ((elt=GetNext(&PLCs,elt))!=NULL);			
			return (BuildSession(plc));
		}
	}else 
	{
		free(Services);
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
	if (elt!=NULL) do
	{
		PLC *plc=elt->Data;
		if (plc->Connection==connection) plc->Connection=new_connection;
	} while ((elt=GetNext(&PLCs,elt))!=NULL);	
	
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
	RemoveChListe(&CONNECTIONs,connection);
	if ((res=Forward_Close(connection))>=0) 
		Log(LOG_DEBUG,"Connection (%p) Killed\n",connection);
		else Log(LOG_WARNING,"Unable to kill Connection (%p)\n",connection);
	Log(LOG_DEBUG,"There is %d Connection\n",CONNECTIONs.Count);
	return(res);
}
int KillSession(Eip_Session *session)
{ int res=0;
	if (session==NULL) return(0);
	RemoveChListe(&SESSIONs,session);
	if ((res=_UnRegisterSession(session))>=0) 
		Log(LOG_DEBUG,"Session (%p) Killed\n",session);
		else Log(LOG_WARNING,"Unable to kill session (%p)\n",session);
	CloseSession(session);
	Log(LOG_DEBUG,"There is %d Session\n",SESSIONs.Count);
	return(res);
}
void KillConnections(void)
{
	Log(LOG_NOTICE,"There is %d Sessions and %d Connections\n",SESSIONs.Count,CONNECTIONs.Count);
	ELEMENT *elt;
	while ((elt=GetFirst(&CONNECTIONs))!=NULL)
	{
		Eip_Connection *connexion=elt->Data;
		RemoveChListe_Ex(&CONNECTIONs,elt);
		free(elt);
		if (connexion==NULL) continue;
		if (Forward_Close(connexion)>=0) 
			Log(LOG_NOTICE,"Connection (%p) Killed\n",connexion);
			else Log(LOG_WARNING,"Unable to kill Connection (%p)\n",connexion);
	}
	memset(&CONNECTIONs,0,sizeof(CONNECTIONs));
	
	while ((elt=GetFirst(&SESSIONs))!=NULL)
	{
		Eip_Session *session=elt->Data;
		RemoveChListe_Ex(&SESSIONs,elt);
		free(elt);
		if (UnRegisterSession(session)>=0) 
			Log(LOG_NOTICE,"Session (%p) Killed\n",session);
			else Log(LOG_WARNING,"Unable to kill session (%p)\n",session);
		CloseSession(session);
	}
	memset(&SESSIONs,0,sizeof(SESSIONs));
}

int ReadTag(TAG *tag)
{
	int result=0;
	DHP_Header dhp={0,0,0,0};
	if (tag->Plc->Session==NULL)
	{
		if (!BuildSession(tag->Plc)) return(0);
	}
	if (tag->Plc->Connection==NULL)
	{
		if (!BuildConnection(tag->Plc)) return(0);
	}
	Log(LOG_DEBUG,"ReadTag : %s (%p / %p)\n",tag->TagName,tag->Plc->Session,tag->Plc->Connection);
	switch (tag->Plc->PlcType)
	{
		case PLC5:
		case SLC500:
			{ PLC_Read *data;
				dhp.Dest_adress=tag->Plc->Node;
				if (tag->Plc->NetWork) // DHP
					data=ReadPLCData(tag->Plc->Session,tag->Plc->Connection,&dhp,NULL,0,tag->Plc->PlcType,tns++,tag->TagName,1);
				else data=ReadPLCData(tag->Plc->Session,tag->Plc->Connection,NULL,NULL,0,tag->Plc->PlcType,tns++,tag->TagName,1);
				//Log(LOG_DEBUG,"Reading : %s on %s (%s)\n",TAGs[i].TagName,(TAGs[i].Plc)->PlcName,s_err_msg);
				if (data!=NULL)
				{
					tag->Value=PCCC_GetValueAsFloat(data,0);
					if (!cip_errno) 
					{
						Log(LOG_DEBUG,"%s on %s = %f (%s)\n",tag->TagName,tag->Plc->PlcName,tag->Value,cip_err_msg);
						result=1;
					}	else 
					{
						Log(LOG_WARNING,"Get PCCC value on tag %s: (%d / ext %d) %s\n",tag->TagName,cip_errno,cip_err_msg,cip_ext_errno);
						result=0;
					}
					free(data);
				} else
				{
					Log(LOG_WARNING,"ReadPLCData error on tag %s: (%d) %s\n",tag->TagName,cip_errno,cip_err_msg);
					result=0;
				}
			}; break;
		case LGX:
		{
			LGX_Read *data=ReadLgxData(tag->Plc->Session,tag->Plc->Connection,tag->TagName,1);
			if (data!=NULL)
			{
				tag->Value=GetLGXValueAsFloat(data,0);
				if (!cip_errno) 
				{
					Log(LOG_DEBUG,"%s on %s = %f (%s)\n",tag->TagName,tag->Plc->PlcName,tag->Value,cip_err_msg);
					result=1;
				}	else 
				{
					Log(LOG_WARNING,"Get value : (%d) %s\n",cip_errno,cip_err_msg);
					result=0;
				}
				free(data);
			} else 
			{
				Log(LOG_WARNING,"ReadLgxData error on tag : %s (%s)\n",tag->TagName,cip_err_msg);
				result=0;
			}
		}; break;
		default:Log(LOG_WARNING,"Plc type unknow for : %s\n",tag->Plc->PlcName);
			break;
	}
	if (result) tag->Time_Value=time(NULL);
	return(result);
}

int WriteTag(TAG *tag, float *writeValue ) {
	int result=ERROR;
        int IValue;
	DHP_Header dhp={0,0,0,0};
	if (tag->Plc->Session==NULL)
	{
		if (!BuildSession(tag->Plc)) return(0);
	}
	if (tag->Plc->Connection==NULL)
	{
		if (!BuildConnection(tag->Plc)) return(0);
	}
	Log(LOG_DEBUG,"WriteTag : %s (%p / %p)\n",tag->TagName,tag->Plc->Session,tag->Plc->Connection);
	switch (tag->Plc->PlcType)
	{
		case PLC5:
		case SLC500:
			{ PLC_Read *data;
				dhp.Dest_adress=tag->Plc->Node;
				if (tag->Plc->NetWork) // DHP
                                    data=ReadPLCData(tag->Plc->Session,tag->Plc->Connection,&dhp,NULL,0,tag->Plc->PlcType,tns++,tag->TagName,1);
				else 
                                    data=ReadPLCData(tag->Plc->Session,tag->Plc->Connection,NULL,NULL,0,tag->Plc->PlcType,tns++,tag->TagName,1);
				//Log(LOG_DEBUG,"Reading : %s on %s (%s)\n",TAGs[i].TagName,(TAGs[i].Plc)->PlcName,s_err_msg);
				if (data!=NULL)
				{
                                    if (tag->Plc->NetWork) { // DHP
					if (WritePLCData(tag->Plc->Session,tag->Plc->Connection,&dhp,NULL,0,tag->Plc->PlcType,tns++,tag->TagName,data->type,writeValue,1)>Error)
                                            result=SUCCESS;
                                        else
                                            result=ERROR;
                                    } else {
                                        if (WritePLCData(tag->Plc->Session,tag->Plc->Connection,NULL,NULL,0,tag->Plc->PlcType,tns++,tag->TagName,data->type,writeValue,1)>Error)
                                            result=SUCCESS;
                                        else
                                            result=ERROR;
                                    }
                                    /* pour remplacer au dessus lorsque le temps
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
					Log(LOG_WARNING,"[WriteTag] Datatype unknow for : %s\n",TagName);
					result=ERROR;
				}
				break;
			} 
                                     */
                                    free(data);
				} else
				{
					Log(LOG_WARNING,"ReadPLCData error on tag %s: (%d) %s\n",tag->TagName,cip_errno,cip_err_msg);
					result= ERROR;
				}
			}; break;
		case LGX:
		{
			LGX_Read *data=ReadLgxData(tag->Plc->Session,tag->Plc->Connection,tag->TagName,1);
			if (data!=NULL)
			{
                            switch(data->type)
                            {
                                    case LGX_BOOL:
                                    {
                                            IValue=(int) *writeValue;
                                            if (IValue!=0) IValue=1;
                                            if (WriteLgxData(tag->Plc->Session,tag->Plc->Connection,tag->TagName,data->type,&IValue,1)>0)
                                                    result=SUCCESS;
                                            else
                                                    result=ERROR;
                                            Log(LOG_DEBUG,"[WriteTag] %s on %s = %x (%s) [%x]\n",tag->TagName,tag->Plc->PlcName,IValue,cip_err_msg,result);
                                    }break;
                                    case LGX_BITARRAY:
                                    {
                                            //
                                    }break;
                                    case LGX_SINT:
                                    case LGX_INT:
                                    case LGX_DINT:
                                    {
                                            IValue=(int) *writeValue;
                                            if (WriteLgxData(tag->Plc->Session,tag->Plc->Connection,tag->TagName,data->type,&IValue,1)>0)
                                                    result=SUCCESS;
                                            else
                                                    result=ERROR;
                                            Log(LOG_DEBUG,"[WriteTag] %s on %s = %x (%s) [%x]\n",tag->TagName,tag->Plc->PlcName,IValue,cip_err_msg,result);
                                    }break;
                                    case LGX_REAL:
                                    {
                                            if (WriteLgxData(tag->Plc->Session,tag->Plc->Connection,tag->TagName,data->type,writeValue,1)>0)
                                                    result=SUCCESS;
                                            else
                                                    result=ERROR;
                                            Log(LOG_DEBUG,"[WriteTag] %s on %s = %f (%s) [%x]\n",tag->TagName,tag->Plc->PlcName,*writeValue,cip_err_msg,result);
                                    }break;
                                    default:
                                    {
                                            Log(LOG_WARNING,"[WriteTag] Datatype unknow for : %s\n",tag->TagName);
                                            result=ERROR;
                                    }
                                    break;
                            }
                            free(data);
			} else 
			{
				Log(LOG_WARNING,"ReadLgxData error on tag : %s (%s)\n",tag->TagName,cip_err_msg);
				result=ERROR;
			}
		}
                break;
		default:
                    Log(LOG_WARNING,"Plc type unknow for : %s\n",tag->Plc->PlcName);
		break;
	}
	//if (result) tag->Time_Value=time(NULL);
	return(result);    
}

void SigHand(int num)
{ 
	//
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
									siglongjmp(jb,-1);
									//Terminated=1;
									//exit(1);
									break;

		default:	Log(LOG_CRIT,"receive signal: %d\n",num);
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
int Reply(int fd,char *format,...)
{	va_list list;
	va_start(list,format);//NULL
	char str[MAXBUFFERSIZE];
	memset(str,0,MAXBUFFERSIZE);
	vsnprintf(str,MAXBUFFERSIZE,format,list);
	va_end(list);
	int result=write(fd,str,strlen(str));
	Log(LOG_DEBUG,"-> %s (socket : %d, %d bytes)\n",str,fd,result);
	return(result);
}

int ReplyJSON(int fd, char *tagname, char *plcname, char *status, char * error, double value) {
    cJSON *root, *tag;
    char *str;
    //memset(str,0,MAXBUFFERSIZE);
    root=cJSON_CreateObject();
    cJSON_AddItemToObject(root, "tag", tag=cJSON_CreateObject());
    cJSON_AddStringToObject(tag,"tagname",tagname);
    cJSON_AddStringToObject(tag,"plcname",plcname);
    cJSON_AddStringToObject(tag,"status",status);
    if (strcmp(error,"")!=0)
        cJSON_AddStringToObject(tag,"error",error);
    if (strcmp(status,"success")==0)
        cJSON_AddNumberToObject(tag,"value",value);
    str = cJSON_Print(root);	
    cJSON_Delete(root);		
    strcat(str,"\n");
    int result=write(fd,str,strlen(str));
    Log(LOG_DEBUG,"-> %s (socket : %d, %d bytes)\n",str,fd,result);
    free(str);
    return(result);
}

int main (int argc,char *argv[])
{	
	// Gestion des signaux
	signal(SIGTERM,SigHand);
	signal(SIGINT,SigHand);
	signal(SIGSEGV,SigHand);

	int c;
	tns=getpid();
	
	while ((c=getopt(argc,argv,"dl:u:p:c:?h"))!=-1)
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
			case 'u': // UPDATE_RATE
				{
					UPDATE_RATE=atoi(optarg);
					break;
				}
			case 'p': // Server Port
				{
					SPort=atoi(optarg);
					break;
				}
			case 'c': //Alias file
				{
					strcpy(Alias,optarg);
					break;
				}
			case '?':
			case 'h':
				{
					printf("%s (Build on %s %s)\n",LOG_TITLE,__DATE__,__TIME__);
					printf("usage: %s:[-d] [-l0-3] [-u rate] [-p listen port] [-?,h]\n",argv[0]);
					printf("-d\tDaemonize %s\n",LOG_TITLE);
					printf("-l{0..3}\t(default :1)\n");
					printf("\t0\tLOG_ERR\n");
					printf("\t1\tLOG_WARNING (Default)\n");
					printf("\t2\tLOG_NOTICE\n");
					printf("\t3\tLOG_DEBUG\n");
					printf("-u\tUpdate rate (Default :\"%d\")\n",UPDATE_RATE);
					printf("-p\tListen port (Default :\"%d\")\n",SPort);
					printf("-c\tAlias file (Default :\"%s\")\n",Alias);
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

	Log(LOG_ALERT,"starting...Pid=%d\n",getpid());
	
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
								Gardian(argv);
								Log(LOG_ALERT,"stopped\n");
								closelog();
								exit(0);
							}
		default : exit(0);
		}
	}else
	{
		Gardian(argv);
		Log(LOG_ALERT,"stopped\n");
		exit(0);
	}
}
int Gardian(char *argv[])
{ //return(mainprog());
	int res=0;
	while(!Terminated)
	{
		if (sigsetjmp(jb,1))
		{
			Log(LOG_CRIT,"Gardian restarting main process in %d sec\n",WAITING);
			sleep(WAITING);
			if (Terminated) return(1);
			execv(argv[0],argv);
			Log(LOG_CRIT,"Gardian Error %s\n",strerror(errno));
		}
		res=mainprog();
		if (res!=0)
		{
			Log(LOG_CRIT,"Exit on Error (%d)\n",res);
			return(res);
		}
	}
	return(res);
}
int lire_client(CLIENT *client)
{	
	memset(tempbuf,0,sizeof(tempbuf));
	int nb_lu = read(client->FD,&tempbuf,sizeof(tempbuf));
	Log(LOG_DEBUG,"lire client : recu %d bytes (%s)\n",nb_lu,&tempbuf);
	if(nb_lu>0) 
	{
		if (nb_lu>(MAXBUFFERSIZE-client->InBuffer.size))
		{
			client->InBuffer.size=-1;
		} else
		{
			memcpy(&(client->InBuffer.data[client->InBuffer.size]),&tempbuf,nb_lu);
			client->InBuffer.size+=nb_lu;
		}
		return (nb_lu);
	}	else return 0;
}
int mainprog(void)
{	
	int nfds = getdtablesize();
	struct timeval timeout;
	fd_set rfds;	
	struct sockaddr_in sonadr;
	int	fd;
	CLIENT *client=NULL;
	FD_ZERO(&fd_clients);
	
	InitChListe(&PLCs);
	InitChListe(&TAGs);
	InitChListe(&CLIENTs);
	InitChListe(&SESSIONs);
	InitChListe(&CONNECTIONs);	
	
	if (GetPlc(&PLCs,Alias)<=0)
	{
		Log(LOG_CRIT,"Alias File is empty\n");
		return(T_Error);
	}
	Server=OpenServerSock(SPort);
	if (Server<0)
	{
		Log(LOG_CRIT,"Error creating Server socket : %s\n",strerror(errno));
		return(T_Error);
	}
	SetCoe(Server);
	FD_SET(Server,&fd_clients);
	while ((!Terminated))
	{
		memcpy(&rfds, &fd_clients, sizeof(rfds));
		timeout.tv_sec=ScanRate;
		timeout.tv_usec=uScanRate;
		
		switch (select(nfds, &rfds, 0, 0,&timeout))
		{
			case -1:if(errno == EINTR) continue; 
				else 
				{
					Log(LOG_WARNING,"surveillance des descripteurs : %s\n",strerror(errno));
					break;
				}
			case 0:	
				{ // timeout
					//continue; 
					break;
				}
			default:
				if(FD_ISSET(Server, &rfds)) 
				{
					int taille = sizeof (sonadr);
					if((fd = accept(Server, (struct sockaddr *)&sonadr,(socklen_t *)&taille))==-1) 
					{
						Log(LOG_WARNING, "accept: connexion impossible (%s)\n",strerror(errno));
					} else 
					{
						Log(LOG_INFO,"Connexion d'un client depuis %s (socket : %d)\n", inet_ntoa(sonadr.sin_addr),fd);
						SetCoe(fd);
						client=malloc(sizeof(CLIENT));
						if (client!=NULL)
						{
							if (AddChListe(&CLIENTs,client)>0)
							{ /* ajout du client dans les socket à surveiller */
								memset(client,0,sizeof(CLIENT));
								client->FD=fd;
								FD_SET(fd, &fd_clients); 
								fcntl(fd, F_SETFL, O_NONBLOCK | fcntl(fd, F_GETFL, 0));
								Log(LOG_INFO,"Client connecté depuis %s (socket : %d)\n", inet_ntoa(sonadr.sin_addr),fd);
							} else 
							{
								close(fd);
								Log(LOG_CRIT, "Erreur à l'ajout du client (%s)\n",strerror(errno));
							}
						} else 
						{
							close(fd);
							Log(LOG_WARNING, "Erreur à la création du client (%s)\n",strerror(errno));
						}
					}
				}
				/* Tester si les sockets clientes ont bougées */
				for(fd=0; fd<nfds; fd++ )
				{
					if((fd != Server) && FD_ISSET(fd, &rfds))
					{
						/* Recherche du client */
						client=FindClient(fd,&CLIENTs);
						if (client==NULL)
						{
							Log(LOG_WARNING, "Client inconnu !!! (socket : %d)--\n",fd);
							close(fd);
							FD_CLR(fd, &fd_clients);
							Reply(fd,"Erreur interne (line : %d)\n",__LINE__);
							continue;
						}
						switch (lire_client(client))
						{
							case 0:
								RemoveChListe(&CLIENTs,client);
								free(client);
								close(fd);
								FD_CLR(fd, &fd_clients);
								Log(LOG_INFO, "-- perte d'un client ! (socket : %d)--\n",fd);
								continue;
							default:/* Traitement requete */
								Log(LOG_DEBUG,"<-Client : %p (buffersize : %d)\n",client,client->InBuffer.size);
								Traite(client);
								continue;
						}
					}
				}
				break;
		} 
		/* Checking Tag inactivity */
		if (TAGs.Count>0)
		{
			time_t now=time(NULL);
			ELEMENT *elt=GetFirst(&TAGs);
			if (elt!=NULL) do
			{
				TAG *tag=elt->Data;
				if ((now-tag->Time_Value)>INACTIVITY_TO)
				{
					ELEMENT *elt_old=elt;
					Log(LOG_DEBUG,"\t-Deleting Tag %s on %s (%p / %p)\n",tag->TagName,tag->Plc->PlcName,tag,elt);
					elt=GetNext(&TAGs,elt);
					RemoveChListe_Ex(&TAGs,elt_old);
					free(elt_old);
					if (tag->Plc!=NULL) 
					{
						tag->Plc->References--;
						if (tag->Plc->References<=0)
						{
							Log(LOG_DEBUG,"No more Tag on plc : %s\n",tag->Plc->PlcName);
							DisconnectPlc(tag->Plc);
						}
					}
					free(tag);
					continue;
				}
			} while ((elt=GetNext(&TAGs,elt))!=NULL);
		}
	}	
	close(Server);
	Log(LOG_DEBUG,"Killing all connections\n");
	KillConnections();
	Log(LOG_DEBUG,"Closing list TAGs : %d counts\n",TAGs.Count);
	CloseList(&TAGs);
	Log(LOG_DEBUG,"Closing list PLCs : %d counts\n",PLCs.Count);
	CloseList(&PLCs);
	Log(LOG_DEBUG,"Closing list CLIENTs : %d counts\n",CLIENTs.Count);
	
	CloseList(&CLIENTs);
	return(0);
}
void Traite(CLIENT *client)
{
	char PlcName[30];
	char TagName[50];
	double writeValue = 0;
        float FValue;
        int Itime=0;//refresh time
	char requete[MAXBUFFERSIZE];
        char isWrite = FALSE;
	memset(PlcName,0,sizeof(PlcName));
	memset(TagName,0,sizeof(TagName));
	//memset(writevalue,0,sizeof(writevalue));
	memset(requete,0,sizeof(requete));	
	memcpy(requete,client->InBuffer.data,client->InBuffer.size);
	Log(LOG_DEBUG,"Entering Traite for %p (%s)[%i]\n",client,requete,client->InBuffer.size);
	if (ParseRequest(PlcName,TagName,&writeValue,requete, &isWrite, &Itime)== SUCCESS)
	{
		Log(LOG_DEBUG,"Traite for %s@%s=%f Write:%d \n", TagName, PlcName, writeValue, isWrite);
		if(strncasecmp(PlcName,"@",1)==0)
		{
			Affiche(client->FD,TagName);
			client->InBuffer.size=0;
			return;
		}
		PLC *plc=FindPLC(PlcName,&PLCs);
		if (plc!=NULL)
		{
			TAG *tag=FindTag(TagName,PlcName,&TAGs);
			if (tag!=NULL)
			{ /* Tag exist */
				Log(LOG_DEBUG,"\t=Tag %s already exist (%p)\n",TagName,tag);
			} else
			{
				tag=malloc(sizeof(TAG));
				if (tag!=NULL)
				{
					Log(LOG_DEBUG,"\t+Creating Tag %s (%p) on %s\n",TagName,tag,plc->PlcName);
					memset(tag,0,sizeof(TAG));
					strncpy(tag->TagName,TagName,strlen(TagName));
					tag->Plc=plc;
					plc->References++;
					AddChListe(&TAGs,tag);
				} else 
				{
					Log(LOG_CRIT, "Erreur à l'ajout du tag (%s)\n",strerror(errno));
					return;
				}
			}
                        if (isWrite==FALSE) {
                            if ((time(NULL)-tag->Time_Value)< Itime)
                            {
                                    Log(LOG_DEBUG,"\t=Reading buffered value for Tag %s\n",TagName);
                                    ReplyJSON(client->FD,TagName,plc->PlcName,"success","",tag->Value);
                            } else 
                            {
                                    if (ReadTag(tag)>0) 
                                        ReplyJSON(client->FD,TagName,plc->PlcName,"success","",tag->Value);
                                    else 
                                        ReplyJSON(client->FD,TagName,plc->PlcName,"error","Error",0);
                            }
                        } else {
                            FValue = (float) writeValue;
                            if (WriteTag(tag,&FValue)>ERROR) { 
                                if (ReadTag(tag)>0) 
                                        ReplyJSON(client->FD,TagName,plc->PlcName,"success","",tag->Value);
                                    else 
                                        ReplyJSON(client->FD,TagName,plc->PlcName,"error","Error ReadTag after write",0);
                            } else 
                                ReplyJSON(client->FD,TagName,plc->PlcName,"error","Error WriteTag",0);
                        }
			//Reply(client->FD,"TuxReader : %s (Path : %s) Tag : %s\n",plc->PlcName,plc->PlcPath,TagName);
		} else
		{
			Reply(client->FD,"[%s]%s=PLC not found in Config file\n",plc->PlcName,TagName);
			/* Test Gardian
			int *a=(int*)(NULL);
			*a=10;*/
		}
	} else
	{
		ReplyJSON(client->FD,TagName,PlcName,"error",client->InBuffer.data,0);
	}
	client->InBuffer.size=0;
}

