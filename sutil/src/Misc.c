/***************************************************************************
 *  Copyright (C) 2006 http://www.foxinfo.fr                               *
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

#ifndef _MISC_C
#define _MISC_C

#include <syslog.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>

#define MaxConnections 10

void Wait(int tps)
{ int i;
	if (!tps) return;
	printf("wait :\n");
	for(i=0;i<tps;i++)
	{
		printf("\b\b%d",tps-i);
		fflush(stdout);
		sleep(1);
	}
	printf("\n");
}
void _FlushBuffer(void *buffer,int size)
{	int i;
	if (buffer==NULL) return;
	for(i=0;i<size;i++) printf("%02X/",*((unsigned char*)(buffer+i)));printf("\n");
	fflush(stdout);
}
void _FlushAsciiBuffer(void *buffer,int size)
{	int i;
	if (buffer==NULL) return;
	for(i=0;i<size;i++) printf("%c",*((char*)(buffer+i)));printf("\n");
	fflush(stdout);
}
void _Log(int level,char *format,...)
{	va_list list;
	va_start(list,format);

	#ifndef DEAMON
		vprintf(format,list);
	#endif
	#ifdef DEAMON
		vsyslog(level,format,list);
	#endif
	va_end(list);
}
int OpenServerSock(char *port)
{	struct sockaddr_in monadr;
	int portnum = atoi(port);
	int opt=1;
	int sock = socket(PF_INET, SOCK_STREAM, 0);
	/* option de r�tilisation d'adresse */
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,(char *) &opt, sizeof(opt));
	/* init socket serveur */
	bzero( (char *)&monadr, sizeof monadr);
	monadr.sin_family = AF_INET;
	monadr.sin_port = htons(portnum);
	monadr.sin_addr.s_addr = INADDR_ANY;

	if( bind(sock, (struct sockaddr *)&monadr, sizeof(monadr)) == -1 )
		{
			//Log(LOG_CRIT,"bind :\n");
			return(-1);
		}
	/* mise en ecoute de notre socket */
	if( listen(sock, MaxConnections) == -1 ) {
		//Log(LOG_CRIT,"listen :");
		return(-1);
	}
	return(sock);
}
int OpenSock(char *serveur,char *port)
{
	int s, r, i, portnum;
	struct sockaddr_in sonadr ;

	portnum = atoi(port);
	/*printf("- Connection au serveur %s sur le port %d -\n",argv[1], port);*/

	bzero( (void *)&sonadr, sizeof sonadr );
	sonadr.sin_family = AF_INET ;
	sonadr.sin_port = htons(portnum) ;
	sonadr.sin_addr.s_addr = inet_addr(serveur);

	s = socket( PF_INET, SOCK_STREAM, 0 ) ;
	/* vrification du format de l'adresse donne */
	if( sonadr.sin_addr.s_addr != -1 )
		{	if (!connect(s,(struct sockaddr *)&sonadr, sizeof(sonadr)))
			return(s); else return(-1);
		}
	else
	{	/* Le serveur est dsign par nom, il faut
		 * alors lancer un requete dns */
		struct hostent *hp;
		hp = gethostbyname(serveur);
		if (hp==NULL) return(-1);
		for( i=0, r=-1; (r==-1) && (hp->h_addr_list[i] != NULL); i++)
		{	bcopy((char *) hp->h_addr_list[i],(char *)&(sonadr.sin_addr), sizeof(sonadr.sin_addr));
			r=(connect(s, (struct sockaddr *)&sonadr, sizeof(sonadr) ));
		}
		if (!r) return(s); else return(-1);
	}
}
int _opencom (char *comdev)
{	struct termios com;
	int file1=open(comdev,O_RDWR|O_NOCTTY|O_NONBLOCK);
	if (file1 < 0)
		{
		return (-1);
		}
	bzero (&com,sizeof(com));
	//com.c_cflag = CS8 | CREAD | CLOCAL;
	//com.c_iflag= IGNPAR;
	com.c_cflag = CS8 |  CREAD | CLOCAL | PARENB | PARODD;
	com.c_iflag= PARODD;
	com.c_cc[VTIME]=10;
	com.c_cc[VMIN]=0;
	cfsetispeed (&com,B9600);
	cfsetospeed (&com,B9600);
	tcflush (file1,TCIOFLUSH);
	tcsetattr(file1,TCSANOW,&com);
	return (file1);
}
#endif /*_MISC_C*/
