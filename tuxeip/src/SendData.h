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

#ifndef _SENDDATA_H
#define _SENDDATA_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "TuxDef.h"
#include "Ethernet_IP.h"

extern char _CIPEmptyBuff[512];

#ifdef _WIN32
    int _InitWSA(); //(WORD version);
#endif

	void _CipFlushBuffer(void *buffer,int size);
	EXPORT int _CipOpenSock(char *serveur,int port);

/********** Time-out unit is msec !!! ********************/

	int _CipSendData(int sock,Encap_Header *header);
	Encap_Header *_CipRecvData(int sock,int timeout);
	Encap_Header *_CipSendData_WaitReply(int sock,Encap_Header *header,int sendtimeout,int rcvtimeout);
#ifdef QT
	#define _CipSendData CipSendData
	#define _CipRecvData CipRecvData
	#define _CipSendData_WaitReply CipSendData_WaitReply
#else
	extern int (*CipSendData)(int sock,Encap_Header *header);
	extern Encap_Header *(*CipRecvData)(int sock,int timeout);
	extern Encap_Header *(*CipSendData_WaitReply)(int sock,Encap_Header *header,int sendtimeout,int rcvtimeout);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _SENDDATA_H */
