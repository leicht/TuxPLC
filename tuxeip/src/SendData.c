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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#ifdef _MSC_VER
	#include <io.h>
#else
	#include <unistd.h>
#endif

#ifdef _WIN32
	#include <winsock2.h>
	#define MAXFILEDESCRIPTORS FD_SETSIZE
	#ifdef __MINGW32__
		#define MSG_WAITALL 8
	#else
		//#define MSG_WAITALL 0
	#endif
#else
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#define MAXFILEDESCRIPTORS getdtablesize()
#endif

#include "SendData.h"
#include "ErrCodes.h"

#ifndef QT
	int (*CipSendData)(int sock,Encap_Header *header)=&_CipSendData;
	Encap_Header *(*CipRecvData)(int sock,int timeout)=&_CipRecvData;
	Encap_Header *(*CipSendData_WaitReply)(int sock,Encap_Header *header,int sendtimeout,int rcvtimeout)=&_CipSendData_WaitReply;
#endif
int _CIPEmptyBuffer(int sock);
char _CIPEmptyBuff[512];

#ifdef _WIN32
int _InitWSA() //(WORD version)
{
  WORD wVersionRequested;
  WSADATA wsaData;

  wVersionRequested = MAKEWORD( 2, 2 );
  return(WSAStartup( wVersionRequested, &wsaData ));
}
#endif

EXPORT int _CipOpenSock(char *serveur,int port)
{
	int s, r, i, portnum;
	struct sockaddr_in sonadr ;

	portnum = port;

	memset( (void *)&sonadr,0, sizeof sonadr );
	sonadr.sin_family = AF_INET ;
	sonadr.sin_port = htons(portnum) ;
	sonadr.sin_addr.s_addr = inet_addr(serveur);

#ifdef _WIN32
	if (_InitWSA() != 0) return(-1);
#endif

	s = socket( PF_INET, SOCK_STREAM, 0 );//|SO_KEEPALIVE
	if (s<0) return(-1);

	/* verification du format de l'adresse donnee */
	if( sonadr.sin_addr.s_addr != -1 )
		{r=connect(s,(struct sockaddr *)&sonadr, sizeof(sonadr));
			if (!r)
				return(s);
			else
			{
				close(s);
#ifdef _WIN32
				WSACleanup();
#endif
				return(E_ConnectionFailed);
			}
		}
	else
	{	/* Le serveur est designe par nom, il faut
		 * alors lancer un requete dns */
		struct hostent *hp;
		hp = gethostbyname(serveur);
		if (hp==NULL) return(E_ConnectionFailed);
		for( i=0, r=-1; (r==-1) && (hp->h_addr_list[i] != NULL); i++)
		{	memmove((char *)&(sonadr.sin_addr),(char *) hp->h_addr_list[i], sizeof(sonadr.sin_addr));
			r=connect(s, (struct sockaddr *)&sonadr, sizeof(sonadr));
		}
		if (!r)
			return(s);
		else
		{
			close(s);
#ifdef _WIN32
			WSACleanup();
#endif
			return(E_ConnectionFailed);
		}
	}
}
void _CipFlushBuffer(void *buffer,int size)
{	int i;
	if (buffer==NULL) return;
	for(i=0;i<size;i++) printf("%02X/",*((unsigned char*)((char*)buffer+i)));printf("\n");
	fflush(stdout);
}
int _CIPEmptyBuffer(int sock)
{	fd_set rfds;
	struct timeval to;
	int rcv=0;

	FD_ZERO(&rfds);
	FD_SET(sock,&rfds);
	to.tv_sec=0;
	to.tv_usec=0;

	CIPERROR(Internal_Error,0,0);
	switch (select(MAXFILEDESCRIPTORS,&rfds,NULL,NULL,&to))
	{
		case 0 ://timeout ;
			return(0);
		case -1 ://err	;
			CIPERROR(Sys_Error,errno,0);
			return(E_Error);
		default://data;
			if (FD_ISSET(sock,&rfds))
				{	rcv=recv(sock,(void *)(&_CIPEmptyBuff),sizeof(_CIPEmptyBuff),0);
					return(rcv);
				} else return(0);
	}
}
int _CipSendData(int sock,Encap_Header *header)
{
	LogCip(LogTrace,".CipSendData : %d\n",header->Length);
	FlushCipBuffer(LogDebug,header,24+header->Length);
	_CIPEmptyBuffer(sock);
	return(send(sock,(void *)(header),sizeof(Encap_Header)+header->Length,0));
}
Encap_Header *_CipRecvData(int sock,int timeout)
{	fd_set rfds;
	struct timeval to;
	int rcv=0;
	int MsgOk=0;
	Encap_Header *header=NULL;
	CIPERROR(0,0,0);
	header=malloc(sizeof(Encap_Header));
	if (header==NULL)
	{
		CIPERROR(Sys_Error,errno,0);
		return(NULL);
	} else CIPERROR(Internal_Error,0,0);

	to.tv_sec=(timeout/1000);
	to.tv_usec=timeout*1000-to.tv_sec*1000000;
	while (!MsgOk)
	{
		FD_ZERO(&rfds);
		FD_SET(sock,&rfds);
		switch (select(MAXFILEDESCRIPTORS,&rfds,NULL,NULL,&to))
		{
			case 0 ://timeout ;
				CIPERROR(Internal_Error,E_RcvTimeOut,0);
				free(header);
				return(NULL);
			case -1 ://err	;
				CIPERROR(Sys_Error,errno,0);
				free(header);
				return(NULL);
			default://data;
				if (FD_ISSET(sock,&rfds))
				{
					rcv=recv(sock,(void *)(header),sizeof(Encap_Header),MSG_WAITALL);
					header=realloc(header,sizeof(Encap_Header)+header->Length);
					if (header==NULL)
					{
						CIPERROR(Sys_Error,errno,0);
						return(NULL);
					}
					if (header->Length>0) rcv+=recv(sock,(char*)header+sizeof(Encap_Header),header->Length,MSG_WAITALL);
					LogCip(LogTrace,".CipRecvData : %d\n",rcv);
					FlushCipBuffer(LogDebug,header,rcv);
					return(header);
				} else {CIPERROR(Internal_Error,E_RcvError,0);free(header);return(NULL);};
		}
	}
	return(header);
}
Encap_Header *_CipSendData_WaitReply(int sock,Encap_Header *header,int sendtimeout,int rcvtimeout)
{
	_CIPEmptyBuffer(sock);
	_CipSendData(sock,header);
	return(_CipRecvData(sock,rcvtimeout));
}

