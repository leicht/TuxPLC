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

#ifndef _CM_H
#define _CM_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "TuxDef.h"
#include "CIP_Types.h"
#include "Ethernet_IP.h"
#include "MR.h"
#pragma pack (1)

#define MAX_PATH_SIZE 512

/***************************************************************

	Object Class Attributes	: 1~7
	Object Class Services : 0x01,0x0e
	Object Instance Attributes : 1~13
	Object Instance Services : 0x01,0x0e,0x10


****************************************************************/

#define FORWARD_CLOSE 0x4e
#define UNCONNECTED_SEND 0x52
#define FORWARD_OPEN 0x54
#define GET_CONNECTION_DATA 0x56
#define SEARCH_CONNECTION_DATA 0x57
#define EX_FORWARD_OPEN 0x59
#define GET_CONNECTION_OWNER 0x5a

/************* See Cip common spec Vol.1/Ch.3 *****************/

typedef struct _Forward_Open_Request{
		BYTE Priority;//used to calculate request timeout information
		CIP_USINT TimeOut_Ticks;//used to calculate request timeout information
		CIP_UDINT OT_ConnID; //originator's CIP Produced session ID
		CIP_UDINT TO_ConnID; //originator's CIP consumed session ID
		CIP_UINT ConnSerialNumber;// session serial number
		CIP_UINT OriginatorVendorID;
		CIP_UDINT OriginatorSerialNumber;
		CIP_USINT TimeOutMultiplier;
		BYTE Reserved[3];
		CIP_UDINT OT_RPI;// originator to target packet rate in msec
		WORD OT_Parameters;
		CIP_UDINT TO_RPI;// target to originator packet rate in msec
		WORD TO_Parameters;
		BYTE Transport;
		CIP_USINT PathSize;// size of session path in 16 bits words
		//EPath Path;// padded EPath
		}PACKED Forward_Open_Request;

typedef struct _Success_Forward_Open_Reply{
		CIP_UDINT OT_ConnID; //originator's CIP Produced session ID
		CIP_UDINT TO_ConnID; //originator's CIP consumed session ID
		CIP_UINT ConnSerialNumber;// session serial number
		CIP_UINT OriginatorVendorID;
		CIP_UDINT OriginatorSerialNumber;
		CIP_UDINT OT_API;// originator to target packet rate in msec
		CIP_UDINT TO_API;// target to originator packet rate in msec
		CIP_USINT ReplySize; // size of application reply in 16 bits words
		CIP_USINT Reserved;
		//ByteArray Reply;
		}PACKED Success_Forward_Open_Reply;

typedef struct _UnSuccess_Forward_Open_Reply{
		CIP_UINT ConnSerialNumber;// session serial number
		CIP_UINT OriginatorVendorID;
		CIP_UDINT OriginatorSerialNumber;
		CIP_USINT RemainPathSize;//only with routing type errors
		CIP_USINT Reserved;// shall be 0
		}PACKED UnSuccess_Forward_Open_Reply;

typedef struct _Forward_Close_Request{
		BYTE Priority;//used to calculate request timeout information
		CIP_USINT TimeOut_Ticks;//used to calculate request timeout information
		CIP_UINT ConnSerialNumber;// session serial number
		CIP_UINT OriginatorVendorID;
		CIP_UDINT OriginatorSerialNumber;
		CIP_USINT PathSize;// size of session path in 16 bits words
		CIP_USINT Reserved;
		//EPath Path;// padded EPathfloat
		}PACKED Forward_Close_Request;

typedef struct _Success_Forward_Close_Reply{
		CIP_UINT ConnSerialNumber;// session serial number
		CIP_UINT OriginatorVendorID;
		CIP_UDINT OriginatorSerialNumber;
		CIP_USINT ReplySize; // size of application reply in 16 bits words
		CIP_USINT Reserved;
		//ByteArray Reply;
		}PACKED Success_Forward_Close_Reply;

typedef struct _UnSuccess_Forward_Close_Reply{
		CIP_UINT ConnSerialNumber;// session serial number
		CIP_UINT OriginatorVendorID;
		CIP_UDINT OriginatorSerialNumber;
		CIP_USINT RemainPathSize;//only with routing type errors
		CIP_USINT Reserved;// shall be 0
		}PACKED UnSuccess_Forward_Close_Reply;

typedef struct _Unconnected_Send_Request{
		BYTE Priority;//used to calculate request timeout information
		CIP_USINT TimeOut_Ticks;//used to calculate request timeout information
		CIP_UINT Request_size;
		MR_Request MRRequest;
		//
   }PACKED Unconnected_Send_Request;

typedef struct _Search_Connection_Data_Request{
		CIP_UINT ConnSerialNumber;// session serial number
		CIP_UINT OriginatorVendorID;
		CIP_UDINT OriginatorSerialNumber;
}PACKED Search_Connection_Data_Request;

typedef struct _Connection{
	  /* Session parameters */
		Eip_Session *Session;
		int References;
		void *Data;
		/* Connection parameters */
		CIP_USINT State;
		CIP_USINT Instance_type;
		BYTE TransportClass_trigger;
		/* DeviceNet params to add here */
		CIP_UINT Produced_connection_size;
		CIP_UINT Consumed_connection_size;
		CIP_UINT Expected_packet_rate;
		CIP_UDINT Produced_connection_Id;
		CIP_UDINT Consumed_connection_Id;
		CIP_USINT Watchdog_timeout_action;
		CIP_UINT Produced_connection_path_length;
		BYTE *Produced_connection_path;
		CIP_UINT Consumed_connection_path_length;
		BYTE *Consumed_connection_path;
		CIP_UINT Production_inhibit_time;
		CIP_USINT Connection_timeout_multiplier;
	}PACKED Connection;

/****************** Declaration of global var *********************/

extern CIP_UINT _OriginatorVendorID;//=0xFFFE
extern CIP_UDINT _OriginatorSerialNumber;//=0x12345678
extern BYTE _Priority;//=0x07
extern CIP_USINT _TimeOut_Ticks;//=0x3f
extern WORD _Parameters;/*=0x43f8,_TO_Parameters;=0x43f8*/
extern BYTE _Transport;//=0xa3
extern BYTE _TimeOutMultiplier;//=0x01

/*********************** Function *******************************/

	Unconnected_Send_Request *_Build_Unconnected_Send_Request(
		int *requestsize,
		BYTE Priority,
		CIP_USINT TimeOut_Ticks,//used to calculate request timeout information
		MR_Request *MRrequest,
		int MRrequestsize,
		BYTE *routepath,CIP_USINT routepathsize);

	void *_GetCMReply(Encap_Header *reply);
	#define GetCMReply _GetCMReply(_reply)

	Eip_Connection *_Forward_Open(
		Eip_Session *session,
		BYTE Priority,
		CIP_USINT TimeOut_Ticks,//used to calculate request timeout information
		CIP_UDINT TO_ConnID, //originator's CIP consumed session ID
		CIP_UINT ConnSerialNumber,// session serial number
		CIP_UINT OriginatorVendorID,
		CIP_UDINT OriginatorSerialNumber,
		CIP_USINT TimeOutMultiplier,
		CIP_UDINT RPI,// originator to target packet rate in msec
		CIP_UINT Parameters,
		CIP_USINT Transport,
		BYTE *path,CIP_USINT requestpathsize);
	#define Forward_Open(session,TO_ConnID,ConnSerialNumber,\
		RPI,path,requestpathsize) \
		_Forward_Open(session,_Priority,_TimeOut_Ticks,TO_ConnID,ConnSerialNumber,\
		_OriginatorVendorID,_OriginatorSerialNumber,_TimeOutMultiplier,RPI,_Parameters,\
		_Transport,path,requestpathsize)

	EXPORT int _Forward_Close(Eip_Connection *connection);
	#define Forward_Close _Forward_Close

	int _ExForward_Close(
			Eip_Session *session,
			BYTE Priority,
			CIP_USINT TimeOut_Ticks,//used to calculate request timeout information
			CIP_UINT ConnSerialNumber,// session serial number
			CIP_UINT OriginatorVendorID,
			CIP_UDINT OriginatorSerialNumber,
			BYTE *path,CIP_USINT requestpathsize);

	MR_Reply *_UnconnectedSend(Eip_Session *session,
										BYTE Priority,
										CIP_USINT TimeOut_Ticks,//used to calculate request timeout information
										MR_Request *MRrequest,
										int MRrequestsize,
										BYTE *routepath,CIP_USINT routepathsize,
										int *replysize);
	#define UnconnectedSend(Session,MRrequest,MRrequestsize,routepath,routepathsize,replysize) \
				_UnconnectedSend(Session,_Priority,_TimeOut_Ticks,MRrequest,MRrequestsize,routepath,routepathsize,replysize)

#ifdef __cplusplus
}
#endif

#endif /* _CM_H */
