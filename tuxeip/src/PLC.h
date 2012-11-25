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

#ifndef _PLC_H
#define _PLC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "TuxDef.h"
#include "CIP_Types.h"
#include "AB.h"
#include "Ethernet_IP.h"

#define PCCC_TYPED_READ_CMD	0x0F
#define PCCC_TYPED_READ_FNC	0x68
#define PCCC_TYPED_WRITE_CMD	0x0F
#define PCCC_TYPED_WRITE_FNC	0x67

/* TAF -> Three Address Field */
#define TAF_STATUS	0x84
#define TAF_BIT			0x85
#define TAF_TIMER		0x86
#define TAF_COUNTER	0x87
#define TAF_CONTROL	0x88
#define TAF_INTEGER	0x89
#define TAF_FLOATING	0x8A
#define TAF_INPUT	0x8C
#define TAF_OUTPUT	0x8b

/* Timers / Counter */
#define TIMER_CTL	0
#define TIMER_PRE 1
#define TIMER_ACC 2

/******************    Data access ***********************************/

typedef struct _PCCC_Header{
		BYTE CMD;
		BYTE STS; // must be zero in request
		CIP_INT TNS;
		}PACKED PCCC_Header;

typedef struct _DHP_Header{
		CIP_UINT Dest_link;
		CIP_UINT Dest_adress;
		CIP_UINT Src_link;
		CIP_UINT Src_adress;
		//BYTE data; // the PCCC request
		}PACKED DHP_Header;

typedef struct _PCCC_Requestor_ID{
		CIP_USINT Length;
		CIP_UINT VendorID;
		CIP_UDINT Serial_Number;
		}PACKED PCCC_Requestor_ID;

typedef struct _Logical_Binary_Address{
        BYTE size;
				BYTE address[10];
				unsigned int mask;
        }PACKED Logical_Binary_Address;

typedef struct _Three_Address_Fields{
        BYTE size;
				BYTE address[10];
				unsigned int mask;
        }PACKED Three_Address_Fields;

typedef struct _PCCC_SLCRW_Request{
		BYTE CMD;
		BYTE STS; // must be zero in request
		CIP_UINT TNS;
		BYTE FNC;
		CIP_UINT Offset;
		CIP_UINT Trans;
		// adress
		// BYTE Size;
		}PACKED PCCC_SLCRW_Request;

typedef struct _PCCC_SLCRW_Reply{
		BYTE CMD;
		BYTE STS; // must be zero in request
		CIP_UINT TNS;
		BYTE Ext_STS;
		//BYTE Data[];
		}PACKED PCCC_SLCRW_Reply;

typedef struct _PLC_Read{
								PLC_Data_Type type;
								int Varcount;
								int totalsize;
								int elementsize;
								unsigned int mask;}PACKED PLC_Read;

/********************* Global var *****************************************/

extern PCCC_Requestor_ID _PCCC_RequestorID;

/**********************                        *****************************/

int _BuildLogicalBinaryAddress(char* address,Logical_Binary_Address *lba);

int _BuildThreeAddressField(char* address,Three_Address_Fields *taf);

PCCC_Header *_BuildPLCReadDataRequest(Plc_Type type,CIP_UINT tns,
																		Logical_Binary_Address *lba,CIP_UINT number,int *requestsize);

PCCC_Header *_BuildPLCWriteDataRequest(Plc_Type type,CIP_UINT tns,Logical_Binary_Address *lba,
		CIP_UINT number,PLC_Data_Type datatype,void *data,int *requestsize);

PCCC_Header *_SendUnConnectedPCCCRequest(Eip_Session *session,
																					DHP_Header *dhp,
																					PCCC_Header *PCCCRequest,
																					int PCCCrequestsize,
																					BYTE *routepath,CIP_USINT routepathsize,
																						int *replysize);

#define SendUnConnectedPCCCRequest(Dhp,PCCCRequest,PCCCrequestsize,routepath,routepathsize,timeout)  _SendUnConnectedPCCCRequest(&_session,Dhp,PCCCRequest,PCCCrequestsize,routepath,routepathsize)

PCCC_Header *_SendConnectedPCCCRequest(
										Eip_Session *session,
										Eip_Connection *connection,
										DHP_Header *dhp,
										PCCC_Header *PCCCRequest,
										int PCCCrequestsize,
										int *replysize);
#define SendConnectedPCCCRequest(Connection,Dhp,PCCCRequest,PCCCrequestsize)  _SendUnConnectedPCCCRequest(&_session,Connection,Dhp,PCCCRequest,PCCCrequestsize)

EXPORT PLC_Read *_ReadPLCData(
												Eip_Session *session,
												Eip_Connection *connection,
												DHP_Header *dhp,
												BYTE *routepath,CIP_USINT routepathsize,
												Plc_Type type,CIP_UINT tns,
												char *address,CIP_UINT number);
#define ReadPLCData(session,connection,dhp,routepath,routepathsize,type,tns,address,number) \
				_ReadPLCData(session,connection,dhp,routepath,routepathsize,type,tns,address,number)

PLC_Read *_ReadPLCData_Ex(
														Eip_Session *session,
														Eip_Connection *connection,
														DHP_Header *dhp,
														BYTE *routepath,CIP_USINT routepathsize,
														Plc_Type type,CIP_UINT tns,
														Logical_Binary_Address *lba,CIP_UINT number);
#define ReadPLCData_Ex(session,connection,dhp,routepath,routepathsize,type,tns,lba,number) \
				_ReadPLCData_Ex(session,connection,dhp,routepath,routepathsize,type,tns,lba,number)

EXPORT int _WritePLCData(
														Eip_Session *session,
														Eip_Connection *connection,
														DHP_Header *dhp,
														BYTE *routepath,CIP_USINT routepathsize,
														Plc_Type type,CIP_UINT tns,
														char *address,
														PLC_Data_Type datatype,
														void *data,
														CIP_UINT number);
#define WritePLCData(session,connection,dhp,routepath,routepathsize,type,tns,address,datatype,data,number) \
		_WritePLCData(session,connection,dhp,routepath,routepathsize,type,tns,address,datatype,data,number)

int _GetPLCDataSize(PLC_Data_Type DataType);

PLC_Data_Type _PLCDataType(Data_Type DataType);
#define PLCDataType _PLCDataType

void *_DecodePLCDataType(PCCC_Header *reply,PLC_Data_Type *type,int *tsize,int *esize);
#define DecodePLCDataType _DecodePLCDataType

int _EncodePLCDataType(void **data,PLC_Data_Type type,int number);

PLC_Read *_DecodePCCC(PCCC_Header *reply);

EXPORT int _PCCC_GetValueAsBoolean(PLC_Read *reply,int index);
#define PCCC_GetValueAsBoolean(reply,index) _PCCC_GetValueAsBoolean(reply,index)

EXPORT int _PCCC_GetValueAsInteger(PLC_Read *reply,int index);
#define PCCC_GetValueAsInteger(reply,index) _PCCC_GetValueAsInteger(reply,index)

EXPORT float _PCCC_GetValueAsFloat(PLC_Read *reply,int index);
#define PCCC_GetValueAsFloat(reply,index) _PCCC_GetValueAsFloat(reply,index)

EXPORT void _FreePLCRead(PLC_Read *Data);

#ifdef __cplusplus
}
#endif

#endif /* _PLC_H */
