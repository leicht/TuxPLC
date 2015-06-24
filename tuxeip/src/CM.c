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
#include <errno.h>
#include <string.h>

#include "CM.h"
#include "CIP_Const.h"
#include "ErrCodes.h"
#include "CIP_IOI.h"

CIP_UINT _OriginatorVendorID=TuxPlcVendorId;
CIP_UDINT _OriginatorSerialNumber=0x12345678;
BYTE _Priority=0x0A;//0x07
CIP_USINT _TimeOut_Ticks=0x05;//0x3f
WORD _Parameters=0x43f8;
BYTE _Transport=0xa3;
BYTE _TimeOutMultiplier=0x01;

Unconnected_Send_Request *_Build_Unconnected_Send_Request(int *requestsize,BYTE Priority,CIP_USINT TimeOut_Ticks,
		MR_Request *MRrequest,int MRrequestsize,BYTE *routepath,CIP_USINT routepathsize)
{ char *pos=NULL; // was void*
  int taille=sizeof(Unconnected_Send_Request)
            +MRrequestsize
            +(MRrequestsize%2) // pad
            +routepathsize+routepathsize%2;
	Unconnected_Send_Request *request=malloc(taille);
	if (request==NULL)
	{
		CIPERROR(Sys_Error,errno,0);
		return(NULL);
	}
	pos=(char*)(&request->MRRequest);
	memset(request,0,taille);
	request->Priority=Priority;//used to calculate request timeout information
	request->TimeOut_Ticks=TimeOut_Ticks;//used to calculate request timeout information
  request->Request_size=MRrequestsize;
  memcpy(&(request->MRRequest),MRrequest,MRrequestsize);
	pos+=MRrequestsize+MRrequestsize%2;
  memcpy(pos,&routepathsize,sizeof(routepathsize));
	pos+=routepathsize+sizeof(CIP_USINT);
  memcpy(pos,routepath,routepathsize);
	if (requestsize!=NULL) *requestsize=taille;
	return(request);
}
Eip_Connection *_Forward_Open(Eip_Session *session,/*Eip_Connection *connection,*/BYTE Priority,
			CIP_USINT TimeOut_Ticks,CIP_UDINT TO_ConnID,CIP_UINT ConnSerialNumber,
			CIP_UINT OriginatorVendorID,CIP_UDINT OriginatorSerialNumber,
			CIP_USINT TimeOutMultiplier,CIP_UDINT RPI,CIP_UINT Parameters,CIP_USINT Transport,
			BYTE *path,CIP_USINT requestpathsize)
{ MR_Reply *reply=NULL;
	int taille=((requestpathsize%2)?(requestpathsize/2+1):(requestpathsize/2));
	Forward_Open_Request *request=malloc(sizeof(Forward_Open_Request)+2*taille);
	if (request==NULL)
	{
		CIPERROR(Sys_Error,errno,0);
		return(NULL);
	}
	memset(request,0,sizeof(*request)+2*taille);
	request->Priority=Priority;//used to calculate request timeout information
	request->TimeOut_Ticks=TimeOut_Ticks;//used to calculate request timeout information
	request->OT_ConnID=0;//TO_ConnID; //originator's CIP Produced session ID
	request->TO_ConnID=TO_ConnID; //originator's CIP consumed session ID
	request->ConnSerialNumber=ConnSerialNumber;// session serial number
	request->OriginatorVendorID=OriginatorVendorID;
	request->OriginatorSerialNumber=OriginatorSerialNumber;
	request->TimeOutMultiplier=TimeOutMultiplier;
	request->OT_RPI=1000*RPI;// originator to target packet rate in msec
	request->OT_Parameters=Parameters;
	request->TO_RPI=1000*RPI;// target to originator packet rate in msec
	request->TO_Parameters=Parameters;
	request->Transport=Transport;// class T3
	request->PathSize=taille;// size of session path in 16 bits words
	//memcpy(&(request->Path),path,requestpathsize);// padded EPath
	memcpy((char*)request+sizeof(*request),path,requestpathsize);// padded EPath
	reply=_SendMRRequest(session,FORWARD_OPEN,CM_PATH,sizeof(CM_PATH),request,sizeof(*request)+2*request->PathSize,NULL);
	free(request);
	if (reply!=NULL)
	{
	 //Success_Forward_Open_Reply *sfor=_GetCMReply(session->reply);
		Success_Forward_Open_Reply *sfor=_GetMRData(reply);
		if (sfor!=NULL)
		{ Eip_Connection *connection=malloc(sizeof(Eip_Connection)+2*requestpathsize);
			if (connection==NULL)
			{
				CIPERROR(Sys_Error,errno,0);
				free(reply);
				return(NULL);
			}
			connection->Session=session;
			connection->OriginatorVendorID=OriginatorVendorID;
			connection->OriginatorSerialNumber=OriginatorSerialNumber;
			connection->ConnectionSerialNumber=ConnSerialNumber;
			connection->OT_ConnID=sfor->OT_ConnID;
			connection->TO_ConnID=sfor->TO_ConnID;
			connection->packet=0;
			connection->Path_size=requestpathsize;
			memcpy(((char*)connection)+sizeof(Eip_Connection),path,requestpathsize);
			free(reply);
			return(connection);
		} else
		{
			free(reply);
			return(NULL);
		}
	} else return(NULL);
}
int _ExForward_Close(Eip_Session *session,CIP_USINT Priority,
			CIP_USINT TimeOut_Ticks,CIP_UINT ConnSerialNumber,CIP_UINT OriginatorVendorID,
			CIP_UDINT OriginatorSerialNumber,BYTE *path,CIP_USINT requestpathsize)
{ MR_Reply *reply=NULL;
	int status=0;
	int pathsize=((requestpathsize%2)?(requestpathsize/2+1):(requestpathsize/2));
	int requestsize=sizeof(Forward_Close_Request)+2*pathsize;
	int replysize=0;
	Forward_Close_Request *request=malloc(requestsize);
	if (request==NULL)
	{
		CIPERROR(Sys_Error,errno,0);
		return(E_Error);
	}
	memset(request,0,requestsize);
	request->Priority=Priority;//used to calculate request timeout information
	request->TimeOut_Ticks=TimeOut_Ticks;//used to calculate request timeout information
	request->ConnSerialNumber=ConnSerialNumber;// session serial number
	request->OriginatorVendorID=OriginatorVendorID;
	request->OriginatorSerialNumber=OriginatorSerialNumber;
	request->PathSize=pathsize;// size of session path in 16 bits words
	//memcpy((void*)(request->Path),path,requestpathsize);// padded EPath
	memcpy((char*)request+sizeof(*request),path,requestpathsize);// padded EPath
	reply=_SendMRRequest(session,FORWARD_CLOSE,CM_PATH,sizeof(CM_PATH),request,requestsize,&replysize);
	free(request);
	if (reply!=NULL)
	{
		LogCip(LogDebug,"_ExForward_Close : reply=%p (%d)\n",reply,replysize);
		CIPERROR(MR_Error,reply->General_Status,_GetMRExtendedStatus(reply));
		status=reply->General_Status;
		free(reply);
		return(status);
	} else
	{
		return(E_Error);
	}
}
EXPORT int _Forward_Close(Eip_Connection *connection)
{
	int res=_ExForward_Close(connection->Session,_Priority,_TimeOut_Ticks,connection->ConnectionSerialNumber,
	connection->OriginatorVendorID,connection->OriginatorSerialNumber,
	(BYTE*)((char*)connection+sizeof(Eip_Connection)),connection->Path_size);
	free(connection);
	return(res);
}
void *_GetCMReply(Encap_Header *reply)
{
	return(_GetMRReply(reply));//_GetMRData
}
MR_Reply *_UnconnectedSend(Eip_Session *session,BYTE Priority,
										CIP_USINT TimeOut_Ticks,
										MR_Request *MRrequest,int MRrequestsize,
										BYTE *routepath,CIP_USINT routepathsize,int *replysize)
{ Unconnected_Send_Request *UCRequest=NULL;
	MR_Reply *reply=NULL;
	char *pos=NULL;
	if ((MRrequest==NULL) && (MRrequestsize<=0))
	{
		CIPERROR(Internal_Error,E_NothingToSend,0);
		return(NULL);
	}
	if ((routepath!=NULL) && (routepathsize>0)) //
	{	int UCRequestsize=sizeof(Unconnected_Send_Request)
										+MRrequestsize
										+(MRrequestsize%2) // pad
										+routepathsize+routepathsize%2;

		UCRequest=malloc(UCRequestsize);
		if (UCRequest==NULL)
		{
			CIPERROR(Sys_Error,errno,0);
			return(NULL);
		};
		memset(UCRequest,0,UCRequestsize);
		pos=(char *)(&(UCRequest->MRRequest));

		UCRequest->Priority=Priority;//used to calculate request timeout information
		UCRequest->TimeOut_Ticks=TimeOut_Ticks;//used to calculate request timeout information
		UCRequest->Request_size=MRrequestsize;

		memcpy(pos,MRrequest,MRrequestsize);
		pos+=MRrequestsize+MRrequestsize%2;
		*((CIP_USINT *)pos)=routepathsize/2;
		pos+=2*sizeof(CIP_USINT);
		memcpy(pos,routepath,routepathsize);
		reply=_SendMRRequest(session,UNCONNECTED_SEND,CM_PATH,sizeof(CM_PATH),UCRequest,UCRequestsize,replysize);
		free(UCRequest);
		if (reply!=NULL)
		{
			if ((reply->Service!=(UNCONNECTED_SEND+0x80))&&(reply->Service!=(MRrequest->Service+0x80)))
			{
				CIPERROR(Internal_Error,E_UnsolicitedMsg,0);
				free(reply);
				return(NULL);
			} else
			{
				CIPERROR(MR_Error,reply->General_Status,_GetMRExtendedStatus(reply));
				return(reply);
			}
		} else return(NULL);
	} else
	{
		reply=_ExSendMRRequest(session,MRrequest,MRrequestsize,replysize);
		if (reply!=NULL)
		{
			if (reply->Service!=(MRrequest->Service+0x80))
			{
				CIPERROR(Internal_Error,E_UnsolicitedMsg,0);
				free(reply);
				return(NULL);
			} else
			{
				CIPERROR(MR_Error,reply->General_Status,_GetMRExtendedStatus(reply));
				return(reply);
			}
		} else return(NULL);
	}
}
