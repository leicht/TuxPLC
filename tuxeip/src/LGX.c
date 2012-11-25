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

#include "LGX.h"
#include "ErrCodes.h"
#include "CIP_Const.h"
#include "CIP_IOI.h"

ReadDataService_Request *_BuildLgxReadDataRequest(char *adress,CIP_UINT number,int *requestsize)
{ char *Path=NULL; //was void*
	int pathlen=_BuildIOI(NULL,adress);
	int rsize=sizeof(ReadDataService_Request)+pathlen;
	ReadDataService_Request *request=malloc(rsize);
	if (request==NULL)
	{
		CIPERROR(Sys_Error,errno,0);
		if (requestsize!=NULL) *requestsize=0;
		return(NULL);
	}
	Path=(char*)request+sizeof(request->Service)+sizeof(request->PathSize);
	memset(request,0,rsize);
	request->Service=CIP_DATA_READ;
	request->PathSize=pathlen/2;// size in WORD
	_BuildIOI((BYTE*)(Path),adress);
	memcpy((void*)(Path+pathlen),&number,sizeof(number));
	if (requestsize!=NULL) *requestsize=rsize;
	return(request);
}
WriteDataService_Request *_BuildLgxWriteDataRequest(char *adress,LGX_Data_Type datatype,
										void *data,int datasize,CIP_UINT number,int *requestsize)
{ char *Path=NULL; // was Void
	int pathlen=_BuildIOI(NULL,adress);
	int rsize=sizeof(WriteDataService_Request)+pathlen+datasize;
	WriteDataService_Request *request=malloc(rsize);
	if (request==NULL)
	{
		CIPERROR(Sys_Error,errno,0);
		if (requestsize!=NULL) *requestsize=0;
		return(NULL);
	}
	Path=(char*)request+sizeof(request->Service)+sizeof(request->PathSize);
	memset(request,0,rsize);
	request->Service=CIP_DATA_WRITE;
	request->PathSize=pathlen/2;// size in WORD
	_BuildIOI((BYTE*)(Path),adress);
	memcpy((void*)(Path+pathlen),&datatype,2); // sizeof datatype is 2
	memcpy((void*)(Path+pathlen+2),&number,sizeof(number));
	memcpy((void*)(Path+pathlen+2+sizeof(number)),data,datasize);
	if (requestsize!=NULL) *requestsize=rsize;
	return(request);
}
EXPORT LGX_Read *_ReadLgxData( Eip_Session *session,Eip_Connection *connection,
                            char *adress,CIP_UINT number)
{ int requestsize;
	CommonDataService_Reply *rep=NULL;
	ReadDataService_Request *request=_BuildLgxReadDataRequest(adress,number,&requestsize);
	Eip_CDI *DataItem=_ConnectedSend(session,connection,request,requestsize);
	free(request);
	if (DataItem==NULL) return(NULL);
	rep=_GetService_Reply((Eip_Item*)DataItem,0);
	if (rep==NULL) return(NULL);
	if (rep->Status!=0)
	{
		CIPERROR(AB_Error,rep->Status,_GetExtendedStatus(rep));
		free(DataItem);
		return(NULL);
	}	else
	{
		LGX_Read *result=_DecodeLGX(rep,DataItem->Length-sizeof(DataItem->Packet));
		free(DataItem);
		return(result);
	}
}
EXPORT int _WriteLgxData(Eip_Session *session,Eip_Connection *connection,
                            char *adress,LGX_Data_Type datatype,void *data,
                            /*int datasize,*/CIP_UINT number)
{ int requestsize;
	CommonDataService_Reply *rep=NULL;
	int datasize=number*_GetLGXDataSize(datatype);
	WriteDataService_Request *request=_BuildLgxWriteDataRequest(adress,datatype,data,datasize,number,&requestsize);
	Eip_CDI *DataItem=_ConnectedSend(session,connection,request,requestsize);
	free(request);
	if (DataItem==NULL) return(Error);
	rep=_GetService_Reply((Eip_Item*)DataItem,0);
	if (rep==NULL) return(Error);
	if (rep->Status!=0)
	{
		CIPERROR(AB_Error,rep->Status,_GetExtendedStatus(rep));
		free(DataItem);
		return(Error);
	}	else
	{
		int result=_GetService_ReplyNumber((Eip_Item*)DataItem);
		free(DataItem);
		return(result);
	}
	//return(DataItem->Length);
}
int _GetService_ReplyNumber(Eip_Item *dataitem)
{ CommonDataService_Reply *rep=NULL;
	if (dataitem==NULL) {CIPERROR(Internal_Error,E_InvalidReply,0);return(0);}//E_OutOfRange
    switch (dataitem->Type_Id)
    {
			case (ItemId_UCM):rep=(CommonDataService_Reply *)((char*)dataitem+sizeof(Eip_UDI));
												break;
			case (ItemId_ConnectedTP):rep=(CommonDataService_Reply *)((char*)dataitem+sizeof(Eip_CDI));
												break;
			default:CIPERROR(Internal_Error,E_ItemUnknow,0);return(0);
    }
	switch (rep->Service)
		{
			case (CIP_MULTI_REQUEST+0x80) :
			{
				MultiService_Reply *temp=(MultiService_Reply*)rep;
				return(temp->Count);
			}
			case (CIP_DATA_READ+0x80) :
			case (CIP_DATA_WRITE+0x80) :
      case (EXECUTE_PCCC+0x80) :
				return(1);
			default:CIPERROR(Internal_Error,E_UnsupportedService,0);return(0);
		}
}
CommonDataService_Reply *_GetService_Reply(Eip_Item *dataitem,unsigned int index)
{ CommonDataService_Reply *rep=NULL;
	CIP_UINT *Offsets=NULL;
	if (dataitem==NULL)
	{
		CIPERROR(Internal_Error,E_InvalidReply,0);
		return(NULL);
	}//E_OutOfRange

	switch (dataitem->Type_Id)
	{
		case (ItemId_UCM):rep=(CommonDataService_Reply *)((char*)dataitem+sizeof(Eip_UDI));
											break;
		case (ItemId_ConnectedTP):rep=(CommonDataService_Reply *)((char*)dataitem+sizeof(Eip_CDI));
											break;
		default:CIPERROR(Internal_Error,E_ItemUnknow,0);return(NULL);
	}
	switch (rep->Service)
	{
		case (CIP_MULTI_REQUEST+0x80) :
		{
			MultiService_Reply *temp=(MultiService_Reply*)rep;
			if (index+1>temp->Count) {CIPERROR(Internal_Error,E_OutOfRange,0);return(NULL);};
			//return((CommonDataService_Reply *)((char*)(&temp->Count)+temp->Offsets[temp->Count]));
			Offsets=(CIP_UINT *)((char*)(temp)+sizeof(MultiService_Reply));
			return((CommonDataService_Reply *)((char*)(&temp->Count)+Offsets[temp->Count]));
		}
		case (CIP_DATA_READ+0x80) :
		case (CIP_DATA_WRITE+0x80) :
		case (EXECUTE_PCCC+0x80) :
			if (index>0)
			{
				CIPERROR(Internal_Error,E_OutOfRange,0);
				return(NULL);
			};
			return((CommonDataService_Reply *)rep);
		default:CIPERROR(Internal_Error,E_UnsupportedService,0);return(NULL);
	}
}
LGX_Read *_DecodeLGX(CommonDataService_Reply *reply,int replysize)
{ void *data=NULL;
	LGX_Read *result=malloc(sizeof(LGX_Read));
	if (result==NULL)
	{
		CIPERROR(Sys_Error,errno,0);
		return(NULL);
	};
	memset(result,0,sizeof(LGX_Read));
	result->type=_GetLGXDataType(reply);
	result->elementsize=_GetLGXDataSize(result->type);
	data=_GetData(reply);
	if ((result->elementsize<=0)||(data==NULL))
	{
		CIPERROR(Internal_Error,E_LGX,__LINE__);
		free(result);
		return(NULL);
	}
	result->totalsize=replysize-((int)data-(int)reply);
	result->Varcount=result->totalsize/result->elementsize;
	if(result->Varcount<1)
	{
		CIPERROR(Internal_Error,E_LGX,__LINE__);
		free(result);
		return(NULL);
	}
	result=realloc(result,sizeof(LGX_Read)+result->totalsize);
	if (result==NULL)
	{
		CIPERROR(Sys_Error,errno,0);
		return(NULL);
	};
	memcpy((char*)result+sizeof(LGX_Read),data,result->totalsize);
	return(result);

}
LGX_Data_Type _GetLGXDataType(CommonDataService_Reply *reply)
{	if (reply==NULL) {CIPERROR(Internal_Error,E_InvalidReply,0);return(0);}
	if (reply->Status!=0) {CIPERROR(AB_Error,reply->Status,_GetExtendedStatus(reply));return(0);}
	switch (reply->Service)
	{
		case (CIP_DATA_READ+0x80):
		{	CIP_INT *type;
			type=(CIP_INT*)(((char*)reply)+sizeof(CommonDataService_Reply));
			return(*type);
		}
		default:CIPERROR(Internal_Error,E_UnsupportedDataType,0);return(0);
	}
}
CIP_INT _GetExtendedStatus(CommonDataService_Reply *reply)
{	if (reply==NULL) {CIPERROR(Internal_Error,E_InvalidReply,0);return(0);}
	if (reply->Status!=0x01FF) return(0);
		else return(*((CIP_INT*)((char*)&(reply->Status)+2)));
}
EXPORT float _GetLGXValueAsFloat(LGX_Read *reply,int index)
{ float value=0;
	unsigned int mask=-1;
	if ((reply->Varcount==0)||(reply->totalsize==0))
	{
		CIPERROR(Internal_Error,E_InvalidReply,0);
		return(0);
	}

	CIPERROR(Internal_Error,Success,0);
	if (index>reply->Varcount-1) CIPERROR(Internal_Error,E_OutOfRange,0);

	if (reply->mask) mask=reply->mask;
	switch (reply->type)
	{
		case DT_BOOL:value=(*((BYTE*)(((char*)reply)+sizeof(LGX_Read)+index*reply->elementsize)));break;
		case DT_BITARRAY:value=(*((CIP_DINT*)(((char*)reply)+sizeof(LGX_Read)+index*reply->elementsize)));break;
		case DT_SINT:value=(*((CIP_SINT*)(((char*)reply)+sizeof(LGX_Read)+index*reply->elementsize)));break;
		case DT_INT:value=(*((CIP_INT*)(((char*)reply)+sizeof(LGX_Read)+index*reply->elementsize)));break;
		case DT_DINT:value=(*((CIP_DINT*)(((char*)reply)+sizeof(LGX_Read)+index*reply->elementsize)));break;
		case DT_REAL:value=(*((float*)(((char*)reply)+sizeof(LGX_Read)+index*reply->elementsize)));break;
		default:CIPERROR(Internal_Error,E_UnsupportedDataType,0);return(Error);
	}
	if (mask!=-1)
	{
		if (((unsigned int)value) & (mask)) return(1); else return(0);
	} else return(value);
}

EXPORT int _GetLGXValueAsInteger(LGX_Read *reply,int index)
{
	return(_GetLGXValueAsFloat(reply,index));
}
void *_GetData(CommonDataService_Reply *reply)
{	if (reply==NULL)
	{
		CIPERROR(Internal_Error,E_InvalidReply,0);
		return(NULL);
	}
	if (reply->Status!=0)
	{
		CIPERROR(AB_Error,reply->Status,_GetExtendedStatus(reply));
		return(NULL);
	}
	CIPERROR(Internal_Error,Success,0);
	switch (_GetLGXDataType(reply))
	{
		case LGX_BOOL:
		case LGX_BITARRAY:
		case LGX_SINT:
		case LGX_INT:
		case LGX_DINT:
		case LGX_REAL:return(((char*)reply)+sizeof(CommonDataService_Reply)+sizeof(CIP_INT));
		default:CIPERROR(Internal_Error,E_UnsupportedDataType,0);return(NULL);
	}
}
int _GetLGXDataSize(LGX_Data_Type DataType)
{
	switch (DataType)
	{
		case LGX_BOOL:
		case LGX_SINT:return(sizeof(CIP_SINT));
		case LGX_INT:return(sizeof(CIP_INT));
		case LGX_BITARRAY:
		case LGX_DINT:
		case LGX_REAL:return(sizeof(CIP_DINT));
		default:CIPERROR(Internal_Error,E_UnsupportedDataType,0);return(Error);
	}
}
LGX_Data_Type _LGXDataType(Data_Type DataType)
{
	switch (DataType)
	{
	case AB_BIT:return(LGX_BOOL);
	case AB_SINT:return(LGX_SINT);
	case AB_INT:return(LGX_INT);
	case AB_DINT:return(LGX_DINT);
	case AB_REAL:return(LGX_REAL);
	default:
			return(Error);

	}
}
EXPORT void _FreeLGXRead(LGX_Read *Data)
{
		free(Data);
}
IdList *_GetIdList(Eip_Session *session,BYTE routepath[],int routepathsize)
{
	int replysize=0,MRrequestsize=0;
	int mrdatasize=0;int count=0;
	void *MRData=NULL;
	IdList *result=NULL;
	BYTE path[4]={0x20,OBJECT_PROP,0x24,0x00};
	//BYTE routepath[2]={0x01,0x00};
	
	MR_Request *MRrequest=_BuildMRRequest(CIP_LIST_ALL_OBJECTS,path,sizeof(path),NULL,0,&MRrequestsize);
	MR_Reply *reply=UnconnectedSend(session,MRrequest,MRrequestsize,routepath,routepathsize,&replysize);
	free(MRrequest);
	if (reply!=NULL)
	{	
		//_CipFlushBuffer(reply,replysize);
		MRData=_GetMRData(reply);
		mrdatasize=replysize-sizeof(MR_Reply)-reply->Add_Status_Size;
		if (mrdatasize>0)
		{
			count=mrdatasize/4;
			result=malloc(sizeof(result->IdCount)+mrdatasize);
			if (result!=NULL)
			{
				result->IdCount=count;
				memcpy(&(result->Id[0]),MRData,mrdatasize);
			} else
			{
				CIPERROR(Sys_Error,errno,0);
			}
		}
		//printf("replysize : %d : %d\n",replysize,mrdatasize);
		free(reply);
		return(result);
	} else return(NULL);
}
