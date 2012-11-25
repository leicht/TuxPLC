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

#ifndef _MBCOMMON_H
#define _MBCOMMON_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "MBErrCodes.h"

#include <string.h>
#include <ctype.h>

#ifndef WIN32
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#define MAXFILEDESCRIPTORS getdtablesize()
	#define DLLEXPORT
#endif

#ifdef WIN32
	#include <winsock2.h>
	#include <windef.h>
	#define MAXFILEDESCRIPTORS FD_SETSIZE
	DLLEXPORT int _InitWSA(WORD version);
#endif

#ifndef _IEC1131_DATATYPE
	#define _IEC1131_DATATYPE
	typedef enum _IEC1131{IEC_UNK,IEC_BYTE,IEC_INT,IEC_UINT,IEC_DINT,IEC_UDINT,IEC_REAL} IEC1131;
#endif

//#define SWAPPED_MODBUS

	#ifdef SWAPPED_MODBUS
		#define _mbswap_32(x) ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) | (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))
	#else
		#define _mbswap_32(x) ((((x) & 0xff000000) >> 8) | (((x) & 0x00ff0000) <<  8) | (((x) & 0x0000ff00) >>  8) | (((x) & 0x000000ff) << 8))
	#endif

#define _mbswap_16(x) ((((x)&0xff00)>>8)|(((x)&0x00ff)<<8))

/****************************************************/
#define MODBUS_PORT 502
#define MAX_PDU_SIZE 253
#define MODBUS_PROTOCOL 0x00
#define MB_DEFAULT_UNIT_IDENTIFIER 0x01 //0xFF
#define DEFAULT_ON_COIL_VALUE 0xFF00
#define DEFAULT_OFF_COIL_VALUE 0x0000

#define MAX_READ_COILS_NUMBER	0x07d0 // 1-2000
#define MAX_READ_REGISTERS_NUMBER	0x007d // 1-125
#define MAX_WRITE_COILS_NUMBER 0x07b0 // 1-1968
#define MAX_WRITE_REGISTERS_NUMBER 0x0078 // 1-120

#define MB_DEFAULT_TIMEOUT 1000

/************ Functions Codes ***************/
#define READ_COILS 0x01
#define READ_INPUTS 0x02
#define READ_HOLDING_REGISTER 0x03
#define READ_INPUT_REGISTER 0x04
#define WRITE_SINGLE_COILS 0x05
#define WRITE_SINGLE_REGISTER 0x06
#define WRITE_MULTIPLE_COILS 0x0F
#define WRITE_MULTIPLE_REGISTER 0x10
#define MASK_WRITE_REGISTER 0x16

/************** Global Variables ************************************************/
extern int MB_UNIT_IDENTIFIER;
extern int MB_TIMEOUT;

/************* Types Declaration ************************************************/
typedef unsigned char MB_BYTE; // 0..255	8 bits non sign�
typedef short MB_INT; // 16 bits sign�
typedef unsigned short MB_UINT; // 0..65535	16 bits non sign�
typedef long MB_DINT; // 32 bits sign�
typedef unsigned long MB_UDINT; // 0..4294967295	32 bits non sign�
typedef float MB_REAL;

typedef struct _MBAP{	//ModBus Application Protocol;
	MB_UINT Id; // Transaction Identifier
	MB_UINT Protocol; // Protocol Identifier (Modbus=0)
	MB_UINT Length; // Number of following bytes (including Unit_Id)
	MB_BYTE Unit_Id; // Identifier of a remote slave (recommended: 0xFF)
}__attribute__((packed)) MBAP;



/********** Function Declaration (Time-out unit is msec !!!) ********************/

	DLLEXPORT void _MBFlushBuffer(void *buffer,int size);
	#define MBFlushBuffer(header) _MBFlushBuffer(header,header->Length+6)

	DLLEXPORT void _MBLog(char *format,...);
	#define MBLog _MBLog

	DLLEXPORT int _MBGetDataSize(IEC1131 Type);
	#define MBGetDataSize _MBGetDataSize

	DLLEXPORT int _MBOpenSock(char *serveur,int port);
	#define MBOpenSock _MBOpenSock

	DLLEXPORT int _MBSendData(int sock,MBAP *header);
	DLLEXPORT MBAP *_MBRecvData(int sock,int timeout);
	DLLEXPORT MBAP *_MBSendData_WaitReply(int sock,MBAP *header,int sendtimeout,int rcvtimeout);

	DLLEXPORT int (*MBSendData)(int sock,MBAP *header);
	DLLEXPORT MBAP *(*MBRecvData)(int sock,int timeout);
	DLLEXPORT MBAP *(*MBSendData_WaitReply)(int sock,MBAP *header,int sendtimeout,int rcvtimeout);


#ifdef __cplusplus
}
#endif

#endif /* _MBCOMMON_H */
