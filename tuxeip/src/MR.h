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

#ifndef _MR_H
#define _MR_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "TuxDef.h"
#include "CIP_Types.h"
#include "Ethernet_IP.h"

/************* See Cip common spec Vol.1/Ch.2-4 *****************/

typedef struct _MR_Request{
		CIP_USINT Service; //service code of the request
		CIP_USINT Request_Path_Size; //number of 16 bit WORD in the request path field (next element)
		//ByteArray Request_Path; // PADDED EPath
		//ByteArray Request_Data; // specific data
		}PACKED MR_Request;
typedef MR_Request *PMR_Request;

typedef struct _MR_Reply{
		CIP_USINT Service; // Reply service code
		CIP_USINT Reserved; // shall be 0
		CIP_USINT General_Status; // One of the general Status code listed in App}ix B (Status codes)
		CIP_USINT Add_Status_Size; // number of 16 bit WORD in Additional status Array (next element)
		//CIP_UINT  Add_Status[0]; // Additional status
		//ByteArray Response_Data; // Response Data from request or additional error data if
		                            // General status indicate an error
		}PACKED MR_Reply;
typedef MR_Reply *PMR_Reply;

/******************                             ***********************/

typedef struct _CommonSendUnitRequest{
		CIP_UINT Packet;
		CIP_USINT Service; //service code of the request
		CIP_USINT PathSize; //number of 16 bit WORD in the request path field (next element)
		//ByteArray Path; // PADDED EPath
		}PACKED CommonSendUnitRequest;

typedef struct _CommonSendUnitReply{
		CIP_UINT Packet;
		CIP_USINT Service;
		CIP_SINT Reserved;
		CIP_UINT Status;
		//ByteArray Data;
		}PACKED CommonSendUnitReply;

/****************** Declaration of global var *********************/

//extern BYTE *_mrbuffer[];

/*********************** Function *******************************/

#define _GetMRRequestSize(pathsize,datasize) (sizeof(MR_Request)+pathsize+datasize)

	MR_Reply *_GetMRReply(Encap_Header *header);
	int _GetMRExtendedStatus(MR_Reply *MRReply);
	void *_GetMRData(MR_Reply *MRReply);
	int _GetMRDataSize(Encap_Header *header);
	int _GetMRReplySize(Encap_Header *header);

	MR_Request *_BuildMRRequest(
											CIP_USINT service,
											BYTE *path,
											CIP_USINT requestpathsize,
											void *requestdata,
											int requestdatasize,int *requestsize);

	MR_Reply *_SendMRRequest(	Eip_Session *session,
                        		CIP_USINT service,BYTE *path,
														CIP_USINT requestpathsize,void *requestdata,
														int requestdatasize,int *replysize);
	#define SendMRRequest _SendMRRequest

	MR_Reply *_ExSendMRRequest(	Eip_Session *session,
															MR_Request *request,int size,
															int *replysize);
	#define ExSendMRRequest(session,request,size)	_SendMRRequest(session,request,size,NULL)

#ifdef __cplusplus
}
#endif

#endif /* _MR_H */
