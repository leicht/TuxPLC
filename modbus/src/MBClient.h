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

#ifndef _MBCLIENT_H
#define _MBCLIENT_H

//#include <modbus/MBCommon.h>
#include "MBCommon.h"
#include <stdlib.h>
#include <errno.h>

#ifdef __cplusplus
extern "C"
{
#endif

//#define FTable_Size 5

/********* Generic structures as defined by Modbus Application Protocol Specification V1.1a ***************/
typedef struct _mb_req_pdu {
	MB_BYTE Function; // Function ID
	MB_BYTE Data; //
	}__attribute__((packed)) mb_req_pdu;

typedef struct _mb_rsp_pdu {
	MB_BYTE Function; // Function ID
	MB_BYTE Data; //
	}__attribute__((packed)) mb_rsp_pdu;

typedef struct _mb_excep_rsp_pdu {
	MB_BYTE Function; // Function ID
	MB_BYTE Code; //
	}__attribute__((packed)) mb_excep_rsp_pdu;

/********************************************************************************************/
typedef struct _mb_read_req{
	MB_BYTE Function; // Function ID
	MB_UINT Address; // Address of first variable
	MB_UINT Number; // Number of variables
	}__attribute__((packed)) mb_read_req;

typedef struct _mb_read_rsp{
	MB_BYTE Function; // Function ID
	MB_UINT Number; // Number of variables
	IEC1131 DataType;
	int BaseAdress;
	MB_BYTE Data;
	}__attribute__((packed)) mb_read_rsp;

typedef struct _mb_write_single_req{
	MB_BYTE Function; // Function ID
	MB_UINT Address; // Address of first variable
	MB_INT Value; // Value to write
	}__attribute__((packed)) mb_write_single_req;

typedef struct _mb_write_multiple_req{
	MB_BYTE Function; // Function ID
	MB_UINT Address; // Address of first variable
	MB_UINT Number; // Number of variables
	MB_BYTE Count; // Byte count of data to write
	}__attribute__((packed)) mb_write_multiple_req;

typedef struct _mb_write_rsp{
	MB_BYTE Function; // Function ID
	MB_UINT Address; // Address of first variable
	MB_UINT Number; // Number / Value of variable(s)
	}__attribute__((packed)) mb_write_rsp;

typedef struct _mb_maskwrite_req{
	MB_BYTE Function; // Function ID
	MB_UINT Address; // Address of first variable
	MB_UINT And_Mask;
	MB_UINT Or_Mask;
	}__attribute__((packed)) mb_maskwrite_req;

typedef struct _mb_maskwrite_req mb_maskwrite_rsp ;

typedef struct _mb_function{
	int AddressL;
	int AddressH;
	char Prefix;
	int Read_Function;
	int Write_Function;
	int MWrite_Function;
	int Offset;
	IEC1131 DataType;
	}__attribute__((packed)) mb_function;

/****************************** Global Variables *****************************************/
extern MB_UINT Transaction_Id;
extern mb_function *FTable;
extern int FTableSize;

/******************************* Function Declaration *************************************/

MB_UINT MBGet_Transaction_Id(void);

int _MBGet_F_Index(int Address);
#define MBGet_F_Index(Address) _MBGet_F_Index(Address)

int _MBGetC_F_Index(char *Address);
#define MBGetC_F_Index(Address) _MBGetC_F_Index(Address)

char _MBGetPrefix(char *Address);
int _MBGetAddress(char *Address);

MBAP *_MBBuild_Request(MB_UINT Transaction_Id,MB_UINT Unit_Id,int Datasize,void *Data);

MBAP *_MBBuild_Read_Request(MB_UINT Transaction_Id,MB_UINT Unit_Id,MB_BYTE Function,MB_UINT Address,int number);

mb_read_rsp *_MBRead_Raw(int socket,int Timeout,MB_UINT Transaction_Id,MB_UINT Unit_Id,char *Address,int number);
#define MBRead_Raw(socket,unit_id,Address,number) _MBRead_Raw(socket,MB_TIMEOUT,MBGet_Transaction_Id(),unit_id,Address,number)

mb_read_rsp *_MBRead_Ext(int socket,int Timeout,MB_UINT Transaction_Id,MB_UINT Unit_Id,char *Address,int number);
#define MBRead_Ext(socket,unit_id,Address,number) _MBRead_Ext(socket,MB_TIMEOUT,MBGet_Transaction_Id(),unit_id,Address,number)
#define MBRead(socket,Address,number) _MBRead_Ext(socket,MB_TIMEOUT,MBGet_Transaction_Id(),MB_UNIT_IDENTIFIER,Address,number)

int _MBGetValueAsInteger(mb_read_rsp *Reply,int index);
#define MBGetValueAsInteger _MBGetValueAsInteger

float _MBGetValueAsReal(mb_read_rsp *Reply,int index);
#define MBGetValueAsReal _MBGetValueAsReal

MBAP *_MBBuild_Write_Request(MB_UINT Transaction_Id,MB_UINT Unit_Id,MB_BYTE Function,IEC1131 DataType,int Address,int number,float *Data);

mb_write_rsp *_MBWrite_Ext(int socket,int Timeout,MB_UINT Transaction_Id,MB_UINT Unit_Id,char *Address,int number,float *Data);
#define MBWrite_Ext(socket,Function,Address,number,Data) _MBWrite_Ext(socket,MB_TIMEOUT,MBGet_Transaction_Id(),MB_UNIT_IDENTIFIER,Address,number,Data)
#define MBWrite(socket,Address,number,Data) _MBWrite_Ext(socket,MB_TIMEOUT,MBGet_Transaction_Id(),MB_UNIT_IDENTIFIER,Address,number,Data)

mb_maskwrite_rsp *_MBMask_Write(int socket,int Timeout,MB_UINT Transaction_Id,MB_UINT Unit_Id,char *Address,MB_UINT And_Mask,MB_UINT Or_Mask);
#define MBMask_Write(socket,Address,And_Mask,Or_Mask) _MBMask_Write(socket,MB_TIMEOUT,MBGet_Transaction_Id(),MB_UNIT_IDENTIFIER,Address,And_Mask,Or_Mask)

#ifdef __cplusplus
}
#endif

#endif /* _MBCLIENT_H */
