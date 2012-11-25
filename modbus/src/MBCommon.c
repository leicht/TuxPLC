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

#ifndef _MBCOMMON_C
#define _MBCOMMON_C


#include "MBCommon.h"
#include "MBErrCodes.h"

#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

int MB_UNIT_IDENTIFIER=MB_DEFAULT_UNIT_IDENTIFIER;
int MB_TIMEOUT=MB_DEFAULT_TIMEOUT;

int (*MBSendData)(int sock,MBAP *header)=&_MBSendData;
MBAP *(*MBRecvData)(int sock,int timeout)=&_MBRecvData;
MBAP *(*MBSendData_WaitReply)(int sock,MBAP *header,int sendtimeout,int rcvtimeout)=&_MBSendData_WaitReply;
int _MBEmptyBuffer(int sock);
char _MBEmptyBuff[512];

#ifdef WIN32
DLLEXPORT int _InitWSA(WORD version)
{
	WORD wVersionRequested;
	WSADATA wsaData;

	wVersionRequested = MAKEWORD( 2, 0 );
	return(WSAStartup( wVersionRequested, &wsaData ));
}
#endif

DLLEXPORT int _MBOpenSock(char *serveur,int port)
{
	int s, r, i, portnum;
	int opt=1;
	struct sockaddr_in sonadr ;

	portnum = port;

	memset( (void *)&sonadr,0, sizeof sonadr );
	sonadr.sin_family = AF_INET ;
	sonadr.sin_port = _mbswap_16(portnum) ;
	sonadr.sin_addr.s_addr = inet_addr(serveur);

	s = socket( PF_INET, SOCK_STREAM, 0 );//|SO_KEEPALIVE
	if (s<0)
	{
		MBERROR(MB_SysError,errno,0);
		return(MBError);
	}
	/*********** Options Modbus ****************************************/
	setsockopt(s,SOL_SOCKET,SO_KEEPALIVE,&opt,sizeof(int));
	setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(int));

	/* verification du format de l'adresse donnee */
	if( sonadr.sin_addr.s_addr != -1 )
	{	r=connect(s,(struct sockaddr *)&sonadr, sizeof(sonadr));
		if (!r) return(s);
			else
			{
				MBERROR(MB_SysError,errno,0);
				close(s);
				return(MBError);
			};
	}
	else
	{	/* Le serveur est designe par nom, il faut
		* alors lancer un requete dns */
		struct hostent *hp;
		hp = gethostbyname(serveur);
		if (hp==NULL) return(MBConnectionFailed);
		for( i=0, r=-1; (r==-1) && (hp->h_addr_list[i] != NULL); i++)
		{	memmove((char *)&(sonadr.sin_addr),(char *) hp->h_addr_list[i], sizeof(sonadr.sin_addr));
		r=connect(s, (struct sockaddr *)&sonadr, sizeof(sonadr));
		}
		if (!r) return(s);
			else
			{
				MBERROR(MB_SysError,errno,0);
				close(s);
				return(MBError);
			};
	}
}
DLLEXPORT void _MBFlushBuffer(void *buffer,int size)
{	int i;
	if (buffer==NULL) return;
	for(i=0;i<size;i++) printf("%02X/",*((unsigned char*)(buffer+i)));printf("\n");
	fflush(stdout);
}
void _MBLog(char *format,...)
{	va_list list;
	va_start(list,format);//NULL
	vfprintf(stdout,format,list);
	va_end(list);
}
int _MBGetDataSize(IEC1131 Type)
{
	switch (Type)
	{
		case  IEC_BYTE:return(1);break;
		case  IEC_INT:case  IEC_UINT:return(2);break;
		case  IEC_DINT:case  IEC_UDINT:case  IEC_REAL:return(4);break;
		default:return(0);break;
	}
}
int _MBEmptyBuffer(int sock)
{	fd_set rfds;
	struct timeval to;
	int rcv=0;

	if (sock<0)
	{
		MBERROR(MB_InternalError,MBBadSocket,0);
		return(MBError);
	}
	FD_ZERO(&rfds);
	FD_SET(sock,&rfds);
	to.tv_sec=0;
	to.tv_usec=0;

	MBERROR(MB_InternalError,0,0);
	switch (select(MAXFILEDESCRIPTORS,&rfds,NULL,NULL,&to))
	{
		case 0 ://timeout ;
			return(MBSuccess);
			break;
			case -1 ://err	;
				MBERROR(MB_SysError,errno,0);
				return(MBError);
				break;
				default://data;
					if (FD_ISSET(sock,&rfds))
					{	rcv=recv(sock,&_MBEmptyBuff,sizeof(_MBEmptyBuff),0);
					return(rcv);
					} else return(MBSuccess);
					break;
	}
}
int _MBSendData(int sock,MBAP *header)
{
	int res;
	if (sock<0)
	{
		MBERROR(MB_InternalError,MBBadSocket,0);
		return(MBError);
	}
	_MBEmptyBuffer(sock);
	MBERROR(0,0,0);
	int len=_mbswap_16(header->Length);
	int tosend=sizeof(MBAP)+len-sizeof(header->Unit_Id);
	res=send(sock,header,tosend,0);
	if (res!=tosend)
	{
		MBERROR(MB_SysError,errno,0);
		return(_mb_errno);
	} else return(MBSuccess);
}
DLLEXPORT MBAP *_MBRecvData(int sock,int timeout)
{	fd_set rfds;
	struct timeval to;
	int rcv=0,len=0;
	int MsgOk=0;

	MBERROR(0,0,0);
	if (sock<0)
	{
		MBERROR(MB_InternalError,MBBadSocket,0);
		return(NULL);
	}
	MBAP *header=malloc(sizeof(MBAP));
	if (header==NULL)
	{
		MBERROR(MB_SysError,errno,0);
		return(NULL);
	} //else MBERROR(MB_InternalError,0,0);

	to.tv_sec=(timeout/1000);
	to.tv_usec=timeout*1000-to.tv_sec*1000000;
	while (!MsgOk)
	{
		FD_ZERO(&rfds);
		FD_SET(sock,&rfds);
		switch (select(MAXFILEDESCRIPTORS,&rfds,NULL,NULL,&to))
		{
			case 0 ://timeout ;
				MBERROR(MB_InternalError,MBTimeOut,0);
				free(header);
				return(NULL);
				break;
				case -1 ://err	;
					MBERROR(MB_SysError,errno,0);
					free(header);
					return(NULL);
					break;
				default://data;
					if (FD_ISSET(sock,&rfds))
					{
						rcv=recv(sock,header,sizeof(MBAP),MSG_WAITALL);
						if (rcv<=0)
						{
							MBERROR(MB_SysError,errno,0);
							free(header);
							return(NULL);
						}
						len=sizeof(MBAP)+_mbswap_16(header->Length)-sizeof(header->Unit_Id);
						header=realloc(header,len);
						if (header==NULL)
						{
							MBERROR(MB_SysError,errno,0);
							return(NULL);
						}
						if (header->Length>sizeof(header->Unit_Id))
							rcv+=recv(sock,(void*)header+sizeof(MBAP),len-sizeof(MBAP),MSG_WAITALL);
						return(header);
					} else {MBERROR(MB_InternalError,MBRcvError,0);free(header);return(NULL);};
					break;
		}
	}
	return(header);
}
DLLEXPORT MBAP *_MBSendData_WaitReply(int sock,MBAP *header,int sendtimeout,int rcvtimeout)
{
	_MBEmptyBuffer(sock);
	if (_MBSendData(sock,header)!=MBSuccess) return(NULL);
	return(_MBRecvData(sock,rcvtimeout));
}

#endif /* _MBCOMMON_C */
