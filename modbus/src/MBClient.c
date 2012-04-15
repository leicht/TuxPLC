/***************************************************************************
 *   Copyright (C) 2006 http://www.foxinfo.fr                              *
 *   Author : Stephane JEANNE	stephane.jeanne@gmail.com                  *
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

#include "MBClient.h"

/******************************* Variables definition ***********************/
MB_UINT Transaction_Id=0;

int FTableSize=6;
mb_function DefTable[]={	{1,99999,0,READ_COILS,WRITE_SINGLE_COILS,WRITE_MULTIPLE_COILS,-1,IEC_BYTE},
													{100001,199999,0,READ_INPUTS,0,0,-100001,IEC_BYTE},
													{300001,399999,0,READ_INPUT_REGISTER,0,0,-300001,IEC_UINT},
													{400001,499999,0,READ_HOLDING_REGISTER,WRITE_SINGLE_REGISTER,WRITE_MULTIPLE_REGISTER,-400001,IEC_INT},
													{400001,499999,'D',READ_HOLDING_REGISTER,WRITE_MULTIPLE_REGISTER,WRITE_MULTIPLE_REGISTER,-400001,IEC_DINT},
													{400001,499999,'F',READ_HOLDING_REGISTER,WRITE_MULTIPLE_REGISTER,WRITE_MULTIPLE_REGISTER,-400001,IEC_REAL},
													};
mb_function *FTable=DefTable;

/********************************* Functions *******************************/
MB_UINT MBGet_Transaction_Id(void)
{
	return(++Transaction_Id);
}
int _MBGet_F_Index(int Address)
{int i;
	int Table_Size=sizeof(FTable);
	for(i=0;i<Table_Size;i++)
	{
		if ((Address>=FTable[i].AddressL)&&(Address<=FTable[i].AddressH))
			return(i);
	}
	return(MBError);
}
char _MBGetPrefix(char *Address)
{
	if ((strlen(Address)>1)&& (isalpha(Address[0]))) return (toupper(Address[0]));
		else return(0);
}
int _MBGetAddress(char *Address)
{
	if ((strlen(Address)>0)&& (isdigit(Address[0]))) return (atoi(Address));
		else if (strlen(Address)>1) return(atoi(&Address[1]));
			else return(0);
}
int _MBGetC_F_Index(char *Address)
{	int i;
	char c=toupper(_MBGetPrefix(Address));
	int add=_MBGetAddress(Address);
	for(i=0;i<FTableSize;i++)
	{
		if ((add>=FTable[i].AddressL)&&(add<=FTable[i].AddressH)&&(c==FTable[i].Prefix))
			return(i);
	}
	return(MBError);
}
MBAP *_MBBuild_Request(MB_UINT Transaction_Id,MB_UINT Unit_Id,int Datasize,void *Data)
{
	MBAP *Header=NULL;
	if (Datasize>MAX_PDU_SIZE)
	{
		MBERROR(MB_InternalError,MBPDUTooLarge,0);
		return(Header);
	};
	Header=malloc(sizeof(MBAP)+Datasize);
	if (Header==NULL)
	{
		MBERROR(MB_SysError,errno,0);
		return(NULL);
	};
	Header->Id=_mbswap_16(Transaction_Id);
	Header->Protocol=MODBUS_PROTOCOL;
	Header->Length=_mbswap_16(Datasize+sizeof(Header->Unit_Id));
	Header->Unit_Id=Unit_Id;
	void *Request=(void *)Header+sizeof(MBAP);
	memcpy(Request,Data,Datasize);
	return(Header);
}
MBAP *_MBBuild_Read_Request(MB_UINT Transaction_Id,MB_UINT Unit_Id,MB_BYTE Function,MB_UINT Address,int number)
{
	MBAP *Header;
	mb_read_req *Request;
	int Reqlen=0;
	switch (Function)
	{
		case READ_COILS:
			Reqlen=sizeof(mb_read_req);
			if (number>MAX_READ_COILS_NUMBER)
			{
				number=MAX_READ_COILS_NUMBER;
				MBERROR(MB_InternalError,MBSizeClamped,0);
			}
			break;
		case READ_INPUTS:
			Reqlen=sizeof(mb_read_req);
			if (number>MAX_READ_COILS_NUMBER)
			if (number>MAX_READ_COILS_NUMBER)
			{
				number=MAX_READ_COILS_NUMBER;
				MBERROR(MB_InternalError,MBSizeClamped,0);
			}
			break;
		case READ_HOLDING_REGISTER:
			Reqlen=sizeof(mb_read_req);
			if (number>MAX_READ_REGISTERS_NUMBER)
			{
				number=MAX_READ_REGISTERS_NUMBER;
				MBERROR(MB_InternalError,MBSizeClamped,0);
			}
			break;
		case READ_INPUT_REGISTER:
			Reqlen=sizeof(mb_read_req);
			if (number>MAX_READ_REGISTERS_NUMBER)
			{
				number=MAX_READ_REGISTERS_NUMBER;
				MBERROR(MB_InternalError,MBSizeClamped,0);
			}
			break;
		default:
			MBERROR(MB_InternalError,MBBadParameter,0);
			return(NULL);
		break;
	}
	Header=malloc(sizeof(MBAP)+Reqlen);
	if (Header==NULL)
	{
		MBERROR(MB_SysError,errno,0);
		printf("!Exiting _Build_Read_Request with error : %s\n",_mb_err_msg);
		return(NULL);
	};
	Header->Id=_mbswap_16(Transaction_Id);
	Header->Protocol=MODBUS_PROTOCOL;
	Header->Length=_mbswap_16(Reqlen+sizeof(Header->Unit_Id));
	Header->Unit_Id=Unit_Id;
	Request=(void *)Header+sizeof(MBAP);
	Request->Function=Function;
	Request->Address=_mbswap_16(Address);
	Request->Number=_mbswap_16(number);
	return(Header);
}
mb_read_rsp *_MBRead_Raw(int socket,int Timeout,MB_UINT Transaction_Id,MB_UINT Unit_Id,char *Address,int number)
{
	MBAP *Req,*Rsp;
	mb_rsp_pdu *Resp;
	mb_read_rsp *ret;

	MBERROR(0,0,0);
	int i=MBGetC_F_Index(Address);
	MB_BYTE func=0;
	int add=0;
	if (i>=0)
	{
		func=FTable[i].Read_Function;
		add=_MBGetAddress(Address)+FTable[i].Offset;
	} else
	{
		MBERROR(MB_InternalError,MBBadParameter,0);
		return(NULL);
	}
 Req=_MBBuild_Read_Request(Transaction_Id,Unit_Id,func,add,number);
	if (Req==NULL) return(NULL);
	Rsp=_MBSendData_WaitReply(socket,Req,Timeout,Timeout);
	free(Req);
	if (Rsp==NULL)
	{
		if (_mb_errno==0) MBERROR(MB_InternalError,MBTimeOut,0);
		return(NULL);
	}
	if (_mbswap_16(Rsp->Id)!=Transaction_Id) {MBERROR(MB_InternalError,MBUnsolicitedMsg,0);free(Rsp);return(NULL);}; // Transaction_Id error
	Resp=(void*)Rsp+sizeof(MBAP);
	if (Resp->Function==func+0x80) {MBERROR(MB_Error,Resp->Data,0);free(Rsp);return(NULL);}; // Modbus exception
	if (Resp->Function!=func) {MBERROR(MB_InternalError,MBUnsolicitedMsg,0);free(Rsp);return(NULL);}; // UnsolicitedMsg
	//ret=malloc(_mbswap_16(Rsp->Length)-sizeof(MB_BYTE)); //sizeof(MBAP.Unit_Id)
	ret=malloc(sizeof(mb_read_rsp)+_mbswap_16(Rsp->Length)-sizeof(mb_rsp_pdu)-sizeof(MB_BYTE)); //sizeof(MBAP.Unit_Id)
	if (ret==NULL)
	{
		MBERROR(MB_SysError,errno,0);
		free(Rsp);
		printf("!Exiting _Build_Read_Request with error : %s\n",_mb_err_msg);
		return(NULL);
	};
	ret->Function=Resp->Function;
	ret->DataType=IEC_UNK;
	ret->BaseAdress=add;
	ret->Number=number;
	memcpy((void*)(&ret->Data),(void*)(Resp)+sizeof(mb_rsp_pdu),_mbswap_16(Rsp->Length)-3); // 3=sizeof(MBAP.Unit_Id)+sizeof(mb_rsp_pdu)
	free(Rsp);
	return(ret);
}		
mb_read_rsp *_MBRead_Ext(int socket,int Timeout,MB_UINT Transaction_Id,MB_UINT Unit_Id,char *Address,int number)
{
	MBAP *Req,*Rsp;
	mb_rsp_pdu *Resp;
	mb_read_rsp *ret;

	MBERROR(0,0,0);
	int i=MBGetC_F_Index(Address);
	MB_BYTE func=0;
	int add=0;
	IEC1131 DataType=0;
	printf("i=%d",i);
	if (i>=0)
	{
		func=FTable[i].Read_Function;
		add=_MBGetAddress(Address)+FTable[i].Offset;
		DataType=FTable[i].DataType;
	} else
	{
		MBERROR(MB_InternalError,MBBadParameter,0);
		return(NULL);
	}
	if (MBGetDataSize(DataType)>2) Req=_MBBuild_Read_Request(Transaction_Id,Unit_Id,func,add,2*number);
		else Req=_MBBuild_Read_Request(Transaction_Id,Unit_Id,func,add,number);
	if (Req==NULL) return(NULL);
	Rsp=_MBSendData_WaitReply(socket,Req,Timeout,Timeout);
	free(Req);
	if (Rsp==NULL)
	{
		if (_mb_errno==0) MBERROR(MB_InternalError,MBTimeOut,0);
		return(NULL);
	}
	if (_mbswap_16(Rsp->Id)!=Transaction_Id) {MBERROR(MB_InternalError,MBUnsolicitedMsg,0);free(Rsp);return(NULL);}; // Transaction_Id error
	Resp=(void*)Rsp+sizeof(MBAP);
	if (Resp->Function==func+0x80) {MBERROR(MB_Error,Resp->Data,0);free(Rsp);return(NULL);}; // Modbus exception
	if (Resp->Function!=func) {MBERROR(MB_InternalError,MBUnsolicitedMsg,0);free(Rsp);return(NULL);}; // UnsolicitedMsg
	//ret=malloc(_mbswap_16(Rsp->Length)-sizeof(MB_BYTE)); //sizeof(MBAP.Unit_Id)
	ret=malloc(sizeof(mb_read_rsp)+_mbswap_16(Rsp->Length)-sizeof(mb_rsp_pdu)-sizeof(MB_BYTE)); //sizeof(MBAP.Unit_Id)
	if (ret==NULL)
	{
		MBERROR(MB_SysError,errno,0);
		free(Rsp);
		printf("!Exiting _Build_Read_Request with error : %s\n",_mb_err_msg);
		return(NULL);
	};
	ret->Function=Resp->Function;
	ret->DataType=DataType;
	ret->BaseAdress=add;
	ret->Number=number;
	memcpy((void*)(&ret->Data),(void*)(Resp)+sizeof(mb_rsp_pdu),_mbswap_16(Rsp->Length)-3); // 3=sizeof(MBAP.Unit_Id)+sizeof(mb_rsp_pdu)
	free(Rsp);
	return(ret);
}

float _MBGetValueAsReal(mb_read_rsp *Reply,int index)
{
	float Value=0;
	if (index>=Reply->Number)
	{
		MBERROR(MB_InternalError,MBOutOfRange,0);
		return(0);
	} else	MBERROR(0,0,0);
	switch (Reply->Function)
	{
		case READ_COILS:
		case READ_INPUTS:
			{ 	int x,y;
				MB_BYTE *byte;
				x=index%8;
				y=(index-x)/8;
				byte=(&(Reply->Data)+y);
				Value=(*byte)&(1<<x);
				if (Value) Value=1;
			}
			break;
		case READ_HOLDING_REGISTER:
		case READ_INPUT_REGISTER:
			{
				switch (Reply->DataType)
				{
					case  IEC_UNK: // if DataType is IEC_UNK then it is a _MBRead_Raw request
						{
							MB_DINT word;
							memcpy(&word,(&(Reply->Data)+/*sizeof(word)**/2*index),sizeof(word));
							word=_mbswap_32(word);
							memcpy(&Value,&word,sizeof(word));
						}
						break;
					case  IEC_INT:
						{
							MB_INT word;
							memcpy(&word,(&(Reply->Data)+sizeof(word)*index),sizeof(word));
							Value=_mbswap_16(word);
						}
						break;
					case  IEC_UINT:
						{
							MB_UINT word;
							memcpy(&word,(&(Reply->Data)+sizeof(word)*index),sizeof(word));
							Value=_mbswap_16(word);
						}
						break;
					case  IEC_DINT:
						{
							MB_DINT word;
							memcpy(&word,(&(Reply->Data)+sizeof(word)*index),sizeof(word));
							Value=_mbswap_32(word);mb_maskwrite_rsp *_Mask_Write(int socket,int Timeout,MB_UINT Transaction_Id,MB_UINT Unit_Id,char *Address,MB_UINT And_Mask,MB_UINT Or_Mask);
						}
						break;
					case  IEC_UDINT:
						{
							MB_UDINT word;
							memcpy(&word,(&(Reply->Data)+sizeof(word)*index),sizeof(word));
							Value=_mbswap_32(word);
						}
						break;
					case  IEC_REAL:
						{
							MB_DINT word;
							memcpy(&word,(&(Reply->Data)+sizeof(word)*index),sizeof(word));
							word=_mbswap_32(word);
							memcpy(&Value,&word,sizeof(word));
						}
						break;
					default:
						MBERROR(MB_InternalError,MBBadParameter,0);
						return(0);
						break;
				}
			}
			break;
		default:
			MBERROR(MB_InternalError,MBBadParameter,0);
			return(0);
		break;
	}
	return(Value);
}
int _MBGetValueAsInteger(mb_read_rsp *Reply,int index)
{
	int Value=0;
	if (index>=Reply->Number)
	{
		MBERROR(MB_InternalError,MBOutOfRange,0);
		return(0);
	} else	MBERROR(0,0,0);
	switch (Reply->Function)
	{
		case READ_COILS:
		case READ_INPUTS:
			{ 	int x,y;
				MB_BYTE *byte;
				x=index%8;
				y=(index-x)/8;
				byte=(&(Reply->Data)+y);
				Value=(*byte)&(1<<x);
				if (Value) Value=1;
			}
			break;
		case READ_HOLDING_REGISTER:
		case READ_INPUT_REGISTER:
			{
				switch (Reply->DataType)
				{
					case  IEC_UNK: // if DataType is IEC_UNK then it is a _MBRead_Raw request
						{
							MB_UINT word;
							memcpy(&word,(&(Reply->Data)+/*sizeof(word)**/index),sizeof(word));
							Value=_mbswap_16(word);
						}
						break;
					case  IEC_INT:
						{
							MB_INT word;
							memcpy(&word,(&(Reply->Data)+sizeof(word)*index),sizeof(word));
							Value=_mbswap_16(word);
						}
						break;
					case  IEC_UINT:
						{
							MB_UINT word;
							memcpy(&word,(&(Reply->Data)+sizeof(word)*index),sizeof(word));
							Value=_mbswap_16(word);
						}
						break;
					case  IEC_DINT:
						{
							MB_DINT word;
							memcpy(&word,(&(Reply->Data)+sizeof(word)*index),sizeof(word));
							Value=_mbswap_32(word);
						}
						break;
					case  IEC_UDINT:
						{
							MB_UDINT word;
							memcpy(&word,(&(Reply->Data)+sizeof(word)*index),sizeof(word));
							Value=_mbswap_32(word);
						}
						break;
					case  IEC_REAL:
						{
							MB_DINT word;
							memcpy(&word,(&(Reply->Data)+sizeof(word)*index),sizeof(word));
							word=_mbswap_32(word);
							memcpy(&Value,&word,sizeof(word));
							//Value=(float)word;
						}
						break;
					default:
						MBERROR(MB_InternalError,MBBadParameter,0);
						return(0);
						break;
				}
			}
			break;
		default:
			MBERROR(MB_InternalError,MBBadParameter,0);
			return(0);
		break;
	}
	return(Value);
}
MBAP *_MBBuild_Write_Request(MB_UINT Transaction_Id,MB_UINT Unit_Id,MB_BYTE Function,IEC1131 DataType,int Address,int number,float *Data)
{
	MBAP *Header;
	int Reqlen=0,coil=0;
	int datasize=0; // number of bytes to send
	switch (Function)
	{
		case WRITE_SINGLE_COILS:
			Reqlen=sizeof(mb_write_single_req);
			number=0;
			coil=1;
			break;
		case WRITE_SINGLE_REGISTER:
			Reqlen=sizeof(mb_write_single_req);
			number=0;
			break;
		case WRITE_MULTIPLE_COILS:
			if (number>MAX_WRITE_COILS_NUMBER)
			{
				number=MAX_WRITE_COILS_NUMBER;
				MBERROR(MB_InternalError,MBSizeClamped,0);
			}
			datasize=number/8;
			if ((number-8*datasize)>0) datasize+=1;
			Reqlen=sizeof(mb_write_multiple_req)+datasize;
			coil=1;
			break;
		case WRITE_MULTIPLE_REGISTER:
			if (MBGetDataSize(DataType)>2)
			{
				if (number>MAX_WRITE_REGISTERS_NUMBER/2)
				{
					number=MAX_WRITE_REGISTERS_NUMBER/2;
					MBERROR(MB_InternalError,MBSizeClamped,0);
				}
				datasize=4*number;
			}else
			{
				if (number>MAX_WRITE_REGISTERS_NUMBER)
				{
					number=MAX_WRITE_REGISTERS_NUMBER;
					MBERROR(MB_InternalError,MBSizeClamped,0);
				}
				datasize=2*number;
			}
			Reqlen=sizeof(mb_write_multiple_req)+datasize;
			break;
		default:
			MBERROR(MB_InternalError,MBBadParameter,0);
			return(NULL);
		break;
	}
	Header=malloc(sizeof(MBAP)+Reqlen);
	if (Header==NULL)
	{
		MBERROR(MB_SysError,errno,0);
		printf("!Exiting _Build_Read_Request with error : %s\n",_mb_err_msg);
		return(NULL);
	};
	memset(Header,0,sizeof(MBAP)+Reqlen);
	Header->Id=_mbswap_16(Transaction_Id);
	Header->Protocol=MODBUS_PROTOCOL;
	Header->Length=_mbswap_16(Reqlen+sizeof(Header->Unit_Id));
	Header->Unit_Id=Unit_Id;
	if (number==0) // single write
	{
		mb_write_single_req *Request;
		Request=(void *)Header+sizeof(MBAP);
		Request->Function=Function;
		Request->Address=_mbswap_16(Address);
		MB_UINT Value=Data[0];
		if (coil)
		{
			if (Value) Request->Value=_mbswap_16(DEFAULT_ON_COIL_VALUE);
				else Request->Value=_mbswap_16(DEFAULT_OFF_COIL_VALUE);
		} else	Request->Value=_mbswap_16(Value);

	}else // Multiple write
	{
		mb_write_multiple_req *Request;
		Request=(void *)Header+sizeof(MBAP);
		Request->Function=Function;
		Request->Address=_mbswap_16(Address);
		Request->Count=datasize;
		if (coil)
		{
			int x,y,i;
			Request->Number=_mbswap_16(number);
			MB_BYTE *ReqData=(void *)Header+sizeof(MBAP)+sizeof(mb_write_multiple_req);
			for(i=0;i<number;i++)
			{
				x=i%8;
				y=(i-x)/8;
				if (Data[i]) ReqData[y]|=1<<x;
			}
		} else // Register
		{
			int i;
			switch (DataType)
			{
				case IEC_INT: case IEC_UINT:
					{
						Request->Number=_mbswap_16(number);
						MB_UINT *ReqData=(void *)Header+sizeof(MBAP)+sizeof(mb_write_multiple_req);
						for(i=0;i<number;i++)
						{
							ReqData[i]=_mbswap_16((MB_UINT)(Data[i]));
						}
					}
					break;
				case  IEC_DINT: case IEC_UDINT:
					{
						Request->Number=_mbswap_16(2*number);
						MB_UDINT *ReqData=(void *)Header+sizeof(MBAP)+sizeof(mb_write_multiple_req);
						for(i=0;i<number;i++)
						{
							ReqData[i]=_mbswap_32((MB_UDINT)(Data[i]));
						}
					}
					break;
				case  IEC_REAL:
					{
						Request->Number=_mbswap_16(2*number);
						MB_REAL *ReqData=(void *)Header+sizeof(MBAP)+sizeof(mb_write_multiple_req);
						MB_UDINT *Req=(MB_UDINT*)ReqData;
						for(i=0;i<number;i++)
						{
							ReqData[i]=Data[i];
							Req[i]=_mbswap_32(Req[i]);
						}
					}
					break;
				default:
					;
					break;
			}
		}
	}
	return(Header);
}
mb_write_rsp *_MBWrite_Ext(int socket,int Timeout,MB_UINT Transaction_Id,MB_UINT Unit_Id,char *Address,int number,float *Data)
{
	MBAP *Req,*Rsp;
	mb_rsp_pdu *Resp;
	mb_write_rsp *ret;

	MBERROR(0,0,0);
	int i=MBGetC_F_Index(Address);
	MB_BYTE func=0;
	int add=0;
	IEC1131 DataType=0;
	if (i<0)
	{
		MBERROR(MB_InternalError,MBBadParameter,0);
		return(NULL);
	}
	if (number>1)	func=FTable[i].MWrite_Function;
		else func=FTable[i].Write_Function;
	add=_MBGetAddress(Address)+FTable[i].Offset;
	DataType=FTable[i].DataType;

	Req=_MBBuild_Write_Request(Transaction_Id,Unit_Id,func,DataType,add,number,Data);
	if (Req==NULL) return(NULL);
	Rsp=_MBSendData_WaitReply(socket,Req,Timeout,Timeout);
	free(Req);
	if (Rsp==NULL) return(NULL);
	if (_mbswap_16(Rsp->Id)!=Transaction_Id) {MBERROR(MB_InternalError,MBUnsolicitedMsg,0);free(Rsp);return(NULL);}; // Transaction_Id error
	Resp=(void*)Rsp+sizeof(MBAP);
	if (Resp->Function==func+0x80) {MBERROR(MB_Error,Resp->Data,0);free(Rsp);return(NULL);}; // Modbus exception
	if (Resp->Function!=func) {MBERROR(MB_InternalError,MBUnsolicitedMsg,0);free(Rsp);return(NULL);}; // UnsolicitedMsg
	ret=malloc(sizeof(mb_write_rsp));
	if (ret==NULL)
	{
		MBERROR(MB_SysError,errno,0);
		free(Rsp);
		printf("!Exiting _Build_Read_Request with error : %s\n",_mb_err_msg);
		return(NULL);
	};
	ret->Function=Resp->Function;
	ret->Address=_mbswap_16(((mb_write_rsp*)Resp)->Address);
	ret->Number=_mbswap_16(((mb_write_rsp*)Resp)->Number);
	free(Rsp);
	return(ret);
}
mb_maskwrite_rsp *_MBMask_Write(int socket,int Timeout,MB_UINT Transaction_Id,MB_UINT Unit_Id,char *Address,MB_UINT And_Mask,MB_UINT Or_Mask)
{
	MBAP *MReq,*MRsp;
	mb_rsp_pdu *Rsp;
	mb_maskwrite_req Req;
	mb_maskwrite_rsp *ret;

	MBERROR(0,0,0);
	int i=MBGetC_F_Index(Address);
	int add=0;
	IEC1131 DataType=0;
	if (i>=0)
	{
		add=_MBGetAddress(Address)+FTable[i].Offset;
		DataType=FTable[i].DataType;
	} else
	{
		MBERROR(MB_InternalError,MBBadParameter,0);
		return(NULL);
	}
	if ((DataType!=IEC_INT)&&(DataType!=IEC_DINT))
	{
		MBERROR(MB_InternalError,MBBadParameter,0);
	}
	Req.Function=MASK_WRITE_REGISTER;
	Req.Address=_mbswap_16(add);
	Req.And_Mask=_mbswap_16(And_Mask);
	Req.Or_Mask=_mbswap_16(Or_Mask);
	MReq=_MBBuild_Request(Transaction_Id,Unit_Id,sizeof(Req),&Req);
	if (MReq==NULL) return(NULL);
	MRsp=_MBSendData_WaitReply(socket,MReq,Timeout,Timeout);
	free(MReq);
	if (MRsp==NULL) return(NULL);
	if (_mbswap_16(MRsp->Id)!=Transaction_Id) {MBERROR(MB_InternalError,MBUnsolicitedMsg,0);free(MRsp);return(NULL);}; // Transaction_Id error
	Rsp=(void*)MRsp+sizeof(MBAP);
	if (Rsp->Function==MASK_WRITE_REGISTER+0x80) {MBERROR(MB_Error,Rsp->Data,0);free(MRsp);return(NULL);}; // Modbus exception
	if (Rsp->Function!=MASK_WRITE_REGISTER) {MBERROR(MB_InternalError,MBUnsolicitedMsg,0);free(MRsp);return(NULL);}; // UnsolicitedMsg
	ret=malloc(sizeof(mb_maskwrite_rsp));
	if (ret==NULL)
	{
		MBERROR(MB_SysError,errno,0);
		free(Rsp);
		return(NULL);
	};
	ret->Function=Rsp->Function;
	ret->Address=((mb_maskwrite_rsp*)Rsp)->Address;
	ret->And_Mask=_mbswap_16(((mb_maskwrite_rsp*)Rsp)->And_Mask);
	ret->Or_Mask=_mbswap_16(((mb_maskwrite_rsp*)Rsp)->Or_Mask);
	free(Rsp);
	return(ret);
}
