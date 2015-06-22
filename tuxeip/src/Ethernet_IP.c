/***************************************************************************
 *  Copyright (C) 2006                                                     *
 *  Author : Stephane JEANNE	stephane.jeanne@gmail.com                  *
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

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef _MSC_VER
	#include <io.h>
#else
	#include <unistd.h>
#endif
#ifdef _WIN32
	#include <winsock2.h>
#endif

#include "Ethernet_IP.h"
#include "SendData.h"
#include "EIP_Const.h"
#include "ErrCodes.h"
#include "CIP_Const.h"

void _AddRR(Encap_Header *request,CIP_UDINT interface_handle,CIP_UINT timeout);
void _AddCPF(Encap_Header *request,Eip_Item *adressitem,void *adress,Eip_Item *dataitem,void *data);
int _AddItem(Encap_Header *request,Eip_Item *item,void *data);

int IsEIP(void *Data)
{
	Eip_Command Command;
  Command=((Encap_Header *)Data)->Command;
  return((Command==EIP_NOP) ||
					(Command==EIP_LISTTARGETS) ||
					(Command==EIP_LISTSERVICES) ||
					(Command==EIP_LISTIDENTITY) ||
					(Command==EIP_LISTINTERFACES) ||
					(Command==EIP_REGISTERSESSION) ||
					(Command==EIP_UNREGISTERSESSION) ||
					(Command==EIP_SENDRRDATA) ||
					(Command==EIP_SENDUNITDATA) ||
					(Command==EIP_INDICATESTATUS) ||
					(Command==EIP_CANCEL));
}

CIP_UDINT _GetEipStatus(Encap_Header *header)
{
	return(header->Status);
}
Eip_Common_Packet *_GetEipCommonPacket(Encap_Header *header)
{
	return((Eip_Common_Packet *)(_GetEipCPF(header)));
}
Eip_CPF *_GetEipCPF(Encap_Header *header)
{
	if ((header==NULL)||(header->Length<2)) return(NULL);
	switch (header->Command)
	{
		case EIP_LISTSERVICES:
		case EIP_LISTIDENTITY:
		case EIP_LISTINTERFACES:return ((Eip_CPF*)((char*)header+sizeof(Encap_Header)));
		case EIP_SENDRRDATA:
		case EIP_SENDUNITDATA:return((Eip_CPF*)((char*)header+sizeof(SendRRData_Request)));
		default :return(NULL);
	}
}
Eip_Item *_GetItem(Eip_Common_Packet *CP,int id)
{
	Eip_Item *item=(Eip_Item *)(&CP->Adress);
 	int i;
  if ((CP==NULL)||(id>CP->Count)) return (NULL);
  for(i=0;i++;i=id)
  {
		item=(Eip_Item*)((char*)item+item->Length);
  }
  return(item);
}
Eip_Item *_GetAdressItem(Encap_Header *header)
{
	Eip_Common_Packet *CP=_GetEipCommonPacket(header);
	if (CP==NULL) return(NULL);
	return(&CP->Adress);
}
Eip_Item *_GetDataItem(Encap_Header *header)
{
	Eip_Item *adressitem=_GetAdressItem(header);
	if (adressitem==NULL) return(NULL);
	return((Eip_Item*)((char *)adressitem+sizeof(Eip_Item)+adressitem->Length));
}
ListInterface_Reply *_GetInterfaces(Encap_Header *header)
{
	if ((header==NULL)||(header->Length<2)) return(NULL);
		else return((ListInterface_Reply*)(header));
}
ListServices_Reply *_GetServices(Encap_Header *header)
{
	if ((header==NULL)||(header->Length<2)) return(NULL);
		else return((ListServices_Reply*)(header));
}
ListIdentity_Reply *_GetIdentity(Encap_Header *header)
{
	if ((header==NULL)||(header->Length<2)) return(NULL);
		else return((ListIdentity_Reply*)(header));
}
EXPORT Eip_Session *_OpenSession(char *serveur,int port,int timeout)
{ Eip_Session *session=NULL;
	CIPERROR(0,0,0);
	session=malloc(sizeof(Eip_Session));
	if (session==NULL)
	{
		CIPERROR(Sys_Error,errno,0);
		return(NULL);
	}
	memset(session,0,sizeof(Eip_Session));
	session->sock=_CipOpenSock(serveur,port);
	session->timeout=timeout;
	if (session->sock<0)
	{
		CIPERROR(Internal_Error,E_ConnectionFailed,0);
		free(session);
		return(NULL);
	}
	 else return(session);
}
EXPORT void CloseSession(Eip_Session *session)
{
	CIPERROR(0,0,0);
	close(session->sock);
#ifdef _WIN32
	WSACleanup();
#endif
	free(session);
}
EXPORT int _RegisterSession(Eip_Session *session)
{	RegisterSession_Request request;
	RegisterSession_Reply *reply=NULL;
	int res;
	CIPERROR(0,0,0);
	session->Session_Handle=0;
	memset(&request,0,sizeof(RegisterSession_Request));
	request.Header.Session_Handle=session->Session_Handle;
	request.Header.Sender_ContextL=session->Sender_ContextL;
	request.Header.Sender_ContextH=session->Sender_ContextH;
	request.Header.Command=EIP_REGISTERSESSION;
	request.Header.Length=4;
	request.Protocol_Version=EIP_VERSION;
	if ((res=CipSendData(session->sock,(Encap_Header*)&request))<0) return(res);

	if ((reply=(RegisterSession_Reply*)CipRecvData(session->sock,session->timeout))==NULL)
	{
		CIPERROR(Internal_Error,E_TimeOut,0);
		return(E_Error);
	}
	if (reply->Header.Command!=EIP_REGISTERSESSION)
		{
			CIPERROR(Internal_Error,E_UnsolicitedMsg,0);
			free(reply);
			return(E_Error);
		}
	CIPERROR(EIP_Error,reply->Header.Status,0);
	if (reply->Header.Status==EIP_SUCCESS) session->Session_Handle=reply->Header.Session_Handle;
	res=reply->Header.Status;
	free(reply);
	return(res);
}
EXPORT int _UnRegisterSession(Eip_Session *session)
{	UnRegisterSession_Request request;
	CIPERROR(0,0,0);
	memset(&request,0,sizeof(UnRegisterSession_Request));
	request.Session_Handle=session->Session_Handle;
	request.Sender_ContextL=session->Sender_ContextL;
	request.Sender_ContextH=session->Sender_ContextH;
	request.Command=EIP_UNREGISTERSESSION;
	return(CipSendData(session->sock,(Encap_Header*)(&request)));
}
ListServices_Reply *_ListServices(Eip_Session *session)
{	ListServices_Request request;
  ListServices_Reply *reply=NULL;
  int res;
	CIPERROR(0,0,0);
	memset(&request,0,sizeof(request));
	request.Session_Handle=session->Session_Handle;
	request.Sender_ContextL=session->Sender_ContextL;
	request.Sender_ContextH=session->Sender_ContextH;
	request.Command=EIP_LISTSERVICES;
	if ((res=CipSendData(session->sock,(Encap_Header*)(&request)))<0) return(NULL);
	if ((reply=(ListServices_Reply*)CipRecvData(session->sock,session->timeout))==NULL)
	{
		CIPERROR(Internal_Error,E_TimeOut,0);
		return(NULL);
	}
	if (reply->Header.Command!=EIP_LISTSERVICES)
	{
		CIPERROR(Internal_Error,E_UnsolicitedMsg,0);
		free(reply);
		return(NULL);
	}
	CIPERROR(EIP_Error,reply->Header.Status,0);
	if (reply->Header.Status!=0) {free(reply);return(NULL);}
		else return(reply);
}
ListIdentity_Reply *_ListIdentity(Eip_Session *session)
{	int res;
	ListIdentity_Request request;
	ListIdentity_Reply *reply=NULL;
  CIPERROR(0,0,0);
	memset(&request,0,sizeof(request));
	request.Session_Handle=session->Session_Handle;
	request.Sender_ContextL=session->Sender_ContextL;
	request.Sender_ContextH=session->Sender_ContextH;
	request.Command=EIP_LISTIDENTITY;
	if ((res=CipSendData(session->sock,(Encap_Header*)(&request)))<0) return(NULL);
	if ((reply=(ListIdentity_Reply*)CipRecvData(session->sock,session->timeout))==NULL)
	{
		CIPERROR(Internal_Error,E_TimeOut,0);
		return(NULL);
	}
	if (reply->Header.Command!=EIP_LISTIDENTITY)
	{
		CIPERROR(Internal_Error,E_UnsolicitedMsg,0);
		free(reply);
		return(NULL);
	}
	CIPERROR(EIP_Error,reply->Header.Status,0);
	if (reply->Header.Status!=0) {free(reply);return(NULL);}
		else return(reply);
}
ListInterface_Reply *_ListInterfaces(Eip_Session *session)
{	int res;
	ListInterface_Request request;
	ListInterface_Reply *reply=NULL;
  CIPERROR(0,0,0);
	memset(&request,0,sizeof(request));
	request.Session_Handle=session->Session_Handle;
	request.Sender_ContextL=session->Sender_ContextL;
	request.Sender_ContextH=session->Sender_ContextH;
	request.Command=EIP_LISTINTERFACES;
	if ((res=CipSendData(session->sock,(Encap_Header*)(&request)))<0) return(NULL);
	if ((reply=(ListInterface_Reply*)CipRecvData(session->sock,session->timeout))==NULL)
	{
		CIPERROR(Internal_Error,E_TimeOut,0);
		return(NULL);
	}
	if (reply->Header.Command!=EIP_LISTINTERFACES)
	{
		CIPERROR(Internal_Error,E_UnsolicitedMsg,0);
		free(reply);
		return(NULL);
	}
	CIPERROR(EIP_Error,reply->Header.Status,0);
	if (reply->Header.Status!=0) {free(reply);return(NULL);}
		else return(reply);
}

void _AddRR(Encap_Header *request,CIP_UDINT interface_handle,CIP_UINT timeout)
{
	_AddDINT2Header(request,interface_handle);
	_AddINT2Header(request,timeout);
}
int _GetItemSize(Eip_Item *item)
{
	switch (item->Type_Id)
	{
		case ItemId_Null :return(sizeof(Eip_NAI));
		//case ItemId_ListIdentityResponse :taille=sizeof();break;
		case ItemId_ConnectionBased :return(sizeof(Eip_CAI));
		case ItemId_ConnectedTP :return(sizeof(Eip_CDI));
		case ItemId_UCM :return(sizeof(Eip_UDI));
		//case ItemId_ListServiceResponse :taille=sizeof();break;
		case ItemId_OTSocketInfo :
		case ItemId_TOSocketInfo :return(sizeof(Eip_SII));
		case ItemId_Sequenced :return(sizeof(Eip_SAI));
		default:return(0);
	}
}
int _AddItem(Encap_Header *request,Eip_Item *item,void *data)
{ int taille=_GetItemSize(item);
	int datasize=0;
	if (taille==0)
	{
		CIPERROR(Internal_Error,E_ItemUnknow,0);
		return(E_Error);
	};
	datasize=item->Length;
	item->Length+=taille-sizeof(Eip_Item);
	if (data!=NULL)
	{
		_AddHeader(request,item,taille);
		if (item->Length>0) _AddHeader(request,data,datasize);
	} else _AddHeader(request,item,taille+datasize);
	return(item->Length);
}
void _AddCPF(Encap_Header *request,Eip_Item *adressitem,void *adress,
										Eip_Item *dataitem,void *data)
{
	_AddINT2Header(request,2);
	_AddItem(request,adressitem,adress);
	_AddItem(request,dataitem,data);
}
Encap_Header *_BuildRequest(Eip_Session *session,Eip_Item *adressitem,void *adress,
										Eip_Item *dataitem,void *data,int timeout)
{ int requestsize=0;
	Encap_Header *request=NULL;
	CIPERROR(0,0,0);
	LogCip(LogTrace,"->Entering BuildRequest \n");
	requestsize=sizeof(SendData_Request)+sizeof(CIP_UINT)+//sizeof(Eip_Common_Packet)+
	_GetItemSize(adressitem)+adressitem->Length+_GetItemSize(dataitem)+dataitem->Length;
	request=malloc(requestsize);
	if (request==NULL)
	{
		CIPERROR(Sys_Error,errno,0);
		LogCip(LogTrace,"!Exiting BuildRequest with error : %s\n",_cip_err_msg);
		return(NULL);
	}
	memset(request,0,requestsize);
	request->Length=sizeof(CIP_UDINT)+/*Interface_Handle*/
									sizeof(CIP_UINT);/*Timeout*/
	request->Session_Handle=session->Session_Handle;
	request->Sender_ContextL=session->Sender_ContextL;
	request->Sender_ContextH=session->Sender_ContextH;
	((SendData_Request*)request)->Timeout=timeout/1000;
	_AddCPF(request,adressitem,adress,dataitem,data);
	FlushCipBuffer(LogDebug,request,requestsize);
	LogCip(LogTrace,"<-Exiting BuildRequest : size=%d (%p)\n",requestsize,request);
	return(request);
}
int _SendData(Eip_Session *session,CIP_UINT command,
							Eip_Item *adressitem,void *adress,
							Eip_Item *dataitem,void *data)
{ Encap_Header *request=NULL;
	int res=0;
	CIPERROR(0,0,0);
	LogCip(LogTrace,"->Entering SendData \n");
	request=_BuildRequest(session,adressitem,adress,dataitem,data,session->timeout);
	if (request==NULL)
	{
		CIPERROR(Sys_Error,errno,0);
		LogCip(LogTrace,"!Exiting SendData with error : %s\n",_cip_err_msg);
		return(E_Error);
	}
	request->Command=command;
	res=CipSendData(session->sock,request);
	free(request);
	LogCip(LogTrace,"<-Exiting SendData\n");
	return(res);
}
Encap_Header *_SendData_WaitReply(Eip_Session *session,CIP_UINT command,
							Eip_Item *adressitem,void *adress,
							Eip_Item *dataitem,void *data)
{ Encap_Header *request=NULL;
	Encap_Header *reply=NULL;
	CIPERROR(0,0,0);
	LogCip(LogTrace,"->Entering SendData_WaitReply\n");
	request=_BuildRequest(session,adressitem,adress,dataitem,data,session->timeout);
	if (request==NULL)
	{
		CIPERROR(Sys_Error,errno,0);
		LogCip(LogTrace,"!Exiting SendData with error : %s\n",_cip_err_msg);
		return(NULL);
	}
	request->Command=command;
	reply=CipSendData_WaitReply(session->sock,request,session->timeout,session->timeout);
	free(request);
	LogCip(LogTrace,"<-Exiting SendData_WaitReply : %p\n",request);
	return(reply);
}
Encap_Header *_SendRRData(Eip_Session *session,
								Eip_Item *adressitem,void *adressdata,
								Eip_Item *dataitem,void *data)
{
	return(_SendData_WaitReply(session,EIP_SENDRRDATA,adressitem,adressdata,dataitem,data));
}
Encap_Header *_SendUnitData(Eip_Session *session,
									Eip_Item *adressitem,void *adressdata,
									Eip_Item *dataitem,void *data)
{
	return(_SendData_WaitReply(session,EIP_SENDUNITDATA,adressitem,adressdata,dataitem,data));
}
Eip_CDI *_ConnectedSend( Eip_Session *session,Eip_Connection *connection,
                    void *request,int size)
{ Eip_CAI adressitem;
	Eip_CDI dataitem;
	Eip_CDI *result=NULL;
	Encap_Header *reply=NULL;

	LogCip(LogTrace,"->Entering ConnectedSend \n");

	// adressitem
	adressitem.Type_Id=ItemId_ConnectionBased;
	adressitem.Length=0;
	adressitem.CID=connection->OT_ConnID;

	// dataitem
	dataitem.Type_Id=ItemId_ConnectedTP;
	dataitem.Packet=++(connection->packet);
	dataitem.Length=size;

	reply=_SendData_WaitReply(session,EIP_SENDUNITDATA,(void*)&adressitem,NULL,(void*)&dataitem,request);
	if (reply!=NULL)
		{
			Eip_CDI *respdataitem =(Eip_CDI *)_GetDataItem(reply);
			if (respdataitem==NULL)
			{
				CIPERROR(EIP_Error,reply->Status,0);
				free(reply);
				LogCip(LogTrace,"!Exiting ConnectedSend with error : %s\n",_cip_err_msg);
				return(NULL);
			}
			if ((reply->Command!=EIP_SENDUNITDATA)||(respdataitem->Packet!=connection->packet))
				{
					CIPERROR(Internal_Error,E_UnsolicitedMsg,0);
					free(reply);
					LogCip(LogTrace,"!Exiting ConnectedSend with error : %s\n",_cip_err_msg);
					return(NULL);
				}
			/* Result is OK */
      CIPERROR(EIP_Error,reply->Status,0);
			result=malloc(sizeof(Eip_CDI)+respdataitem->Length);
			memcpy(result,respdataitem,sizeof(Eip_Item)+respdataitem->Length);
			LogCip(LogTrace,"<-Exiting ConnectedSend : size=%d (%p)\n",sizeof(Eip_Item)+respdataitem->Length,respdataitem);
			free(reply);
			return(result);
		}
		else
		{
			LogCip(LogTrace,"!Exiting ConnectedSend with no response\n");
			return(NULL);
		}
}
void _InitHeader(Encap_Header *header,CIP_UDINT session_handle,
								CIP_DINT Sender_ContextL,CIP_DINT Sender_ContextH)
{
	memset(header,0,sizeof(*header));
	header->Session_Handle=session_handle;
	header->Sender_ContextL=Sender_ContextL;
	header->Sender_ContextH=Sender_ContextH;
}
void _FillHeader(Encap_Header *header,CIP_UDINT session_handle,
								CIP_DINT Sender_ContextL,CIP_DINT Sender_ContextH)
{
	header->Session_Handle=session_handle;
	header->Sender_ContextL=Sender_ContextL;
	header->Sender_ContextH=Sender_ContextH;
}

int _AddHeader(Encap_Header *header,void *buffer,int size)
{ char *pos; // was void
	if (size>(MAX_MSG_LEN-header->Length-sizeof(*header))) return(0);
	pos=((char*)header+sizeof(*header));
	pos+=header->Length;
	memcpy(pos,buffer,size);
	header->Length+=size;
	//LogCip(LogTrace,"AddHeader %d bytes at %p (new size is %d)\n",size,pos,header->Length);
	return(header->Length);
}

int _AddBYTE2Header(Encap_Header *header,BYTE value)
{
	return(_AddHeader(header,&value,sizeof(value)));
}
int _AddINT2Header(Encap_Header *header,CIP_INT value)
{
	return(_AddHeader(header,&value,sizeof(value)));
}
int _AddDINT2Header(Encap_Header *header,CIP_DINT value)
{
	return(_AddHeader(header,&value,sizeof(value)));
}
int _Addtab2Header(Encap_Header *header,BYTE *buff[],int size)
{
	return(_AddHeader(header,buff,size));
}

// Encap_Header *EipDispatcher(Encap_Header *request)
// { Encap_Header *reply=NULL;
// 	int res=0;
// 	int size=0;
// 
// 	reply=malloc(sizeof(reply));
// 	if (reply==NULL)
// 	{
// 		CIPERROR(Sys_Error,errno,0);
// 		return(NULL);
// 	}
// 	/* check common errors */	
// 	//@TODO move to upper level
// 	/*if (size!=(sizeof(Encap_Header)+request->Length))
// 		{
// 			LogCip(LogError,"ProcessEipRequest : EIP_INVALID_LENGTH\n");
// 			reply->Status=EIP_INVALID_LENGTH;
// 			memset(request,0,MAX_MSG_LEN);
// 			return(sizeof(Encap_Header));		
// 		}*/
// 	
// 	/* init reply header with request values */
// 	_InitHeader(reply,request->Session_Handle,
// 			request->Sender_ContextL,request->Sender_ContextH);
// 	reply->Command=request->Command;
// 
// 	/*  */
// 	switch (request->Command)
// 		{
// 		case EIP_NOP: // no response
// 			Log(LOG_INFO,"ProcessEipRequest : EIP_NOP\n");
// 			break;
// 		case EIP_LISTSERVICES:
// 			Log(LOG_INFO,"ProcessEipRequest : EIP_LISTSERVICES\n");
// 			ListServices_Reply *ListServices=(ListServices_Reply*)reply;
// 			ListServices->Count=1;//_Service_Information
// 			Service_Information *SI=(Service_Information*)(&ListServices+sizeof(Eip_Item));/*->Service_Information[0]);*/
// 			SI->ItemTypeCode=0x0100;
// 			SI->ItemLength=0x14;
// 			SI->Protocol_Version=EIP_VERSION;
// 			SI->Capability_Flags=0x20;
// 			strncpy(SI->Name,"Communications",16);
// 			ListServices->Header.Length=0x1a;
// 			res=(sizeof(Encap_Header)+ListServices->Header.Length);
// 			break;
// 		case EIP_LISTIDENTITY:
// 			Log(LOG_INFO,"ProcessEipRequest : EIP_LISTIDENTITY\n");
// 			ListIdentity_Reply *ListIdentity=(ListIdentity_Reply*)reply;
// 			ListIdentity->Count=0;
// 			ListIdentity->Header.Length=0x02;
// 			res=(sizeof(Encap_Header)+ListIdentity->Header.Length);
// 			break;
// 		case EIP_LISTINTERFACES:
// 			Log(LOG_INFO,"ProcessEipRequest : EIP_LISTINTERFACES\n");
// 			ListInterface_Reply *ListInterface=(ListInterface_Reply*)reply;
// 			ListInterface->Count=0;
// 			ListInterface->Header.Length=0x02;
// 			res=(sizeof(Encap_Header)+ListInterface->Header.Length);
// 			break;
// 		case EIP_REGISTERSESSION:
// 			Log(LOG_INFO,"ProcessEipRequest : EIP_REGISTERSESSION\n");
// 			{
// 				RegisterSession_Request *rep=(RegisterSession_Request*)reply;
// 				rep->Protocol_Version=EIP_VERSION;
// 				rep->Options_flags=0;	
// 				if (size!=sizeof(RegisterSession_Request))
// 					{
// 						reply->Status=EIP_INVALID_LENGTH;
// 						res=(sizeof(Encap_Header));
// 						break;
// 					}
// 				if (((RegisterSession_Request*)request)->Protocol_Version!=EIP_VERSION)
// 					{
// 						rep->Header.Status=EIP_UNSUPPORTED_PROTOCOL;
// 					} else
// 					{
// 						client->Session_Handle=rand();
// 						rep->Header.Session_Handle=client->Session_Handle;
// 						rep->Header.Status=EIP_SUCCESS;
// 					}
// 				res=(sizeof(*rep));
// 			}
// 			break;
// 		case EIP_UNREGISTERSESSION:
// 			Log(LOG_INFO,"ProcessEipRequest : EIP_UNREGISTERSESSION\n");
// 			if (size!=sizeof(Encap_Header))
// 				{
// 					reply->Status=EIP_INVALID_LENGTH;
// 					res=(sizeof(Encap_Header));		
// 				} else 
// 				{
// 					client->Session_Handle=0;
// 					//return(0);
// 				}
// 			break;
// 		case EIP_SENDRRDATA:
// 			if (request->Session_Handle!=client->Session_Handle)
// 			{
// 				Log(LOG_INFO,"ProcessEipRequest : EIP_INVALID_SESSION_HANDLE\n");
// 				reply->Status=EIP_INVALID_SESSION_HANDLE;
// 				res=(sizeof(Encap_Header));
// 				break;	
// 			}		
// 			Log(LOG_INFO,"ProcessEipRequest : EIP_SENDRRDATA\n");
// 			res=(ProcessMrRequest(request,size,reply));
// 			break;	
// 		case EIP_SENDUNITDATA:
// 			if (request->Session_Handle!=client->Session_Handle)
// 			{
// 				Log(LOG_INFO,"ProcessEipRequest : EIP_INVALID_SESSION_HANDLE\n");
// 				reply->Status=EIP_INVALID_SESSION_HANDLE;
// 				res=(sizeof(Encap_Header));
// 				break;
// 			}		
// 			Log(LOG_INFO,"ProcessEipRequest : EIP_SENDUNITDATA\n");
// 			res=(ProcessMrRequest(request,size,reply));
// 			break;
// 		case EIP_INDICATESTATUS: // optional
// 			Log(LOG_INFO,"ProcessEipRequest : EIP_INDICATESTATUS\n");
// 			//return(0);
// 			break;
// 		case EIP_CANCEL: // optional
// 			Log(LOG_INFO,"ProcessEipRequest : EIP_CANCEL\n");
// 			//return(0);
// 			break;
// 		case 0x01:
// 			Log(LOG_INFO,"ProcessEipRequest : 0x01\n");
// 			Answear01 *rep=(Answear01*)reply;
// 			rep->tab1[0]=0x01;
// 			rep->tab1[2]=0x01;
// 			rep->tab1[4]=0x24;
// 			rep->tab1[6]=0x01;
// 			rep->tab1[11]=0x02;
// 			rep->tab1[12]=0xaf;
// 			rep->tab1[13]=0x12;
// 			rep->tab1[14]=0x0a;
// 			rep->tab1[15]=0x8c;
// 			rep->tab1[16]=0xc8;
// 			rep->tab1[17]=0x2e;
// 			memcpy(rep->adress,&localadress,sizeof(localadress));
// 			res=sizeof(Answear01);
// 			break;
// 		default: // unknow command
// 			Log(LOG_INFO,"ProcessEipRequest : EIP_INVALID_COMMAND ( %d )\n",request->Command);
// 			_InitHeader(reply,request->Session_Handle,
// 				request->Sender_ContextL,request->Sender_ContextH);
// 			reply->Command=request->Command;
// 			reply->Status=EIP_INVALID_COMMAND;
// 			res=(sizeof(Encap_Header));
// 			break;
// 		}
// 	memset(request,0,MAX_MSG_LEN);
// 	return(res);
// }
