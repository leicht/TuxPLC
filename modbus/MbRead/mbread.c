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

#define INCLUDE_SRC

#include "mbread.h"
#include <unistd.h>

#define MAXALIASSIZE 50
#define LOG_TITLE "Read/Write Modbus"

int main (int argc,char *argv[])
{
	int num=1;
	int c,i,w=0,rep=0;
	float value;
	char TagName[MAXALIASSIZE]="\0";
	char *ip;
	int repeat=1,wait=1000;
	int port=MODBUS_PORT;

	while ((c=getopt(argc,argv,"r:s:p:t:d:n:w:?h"))!=EOF)
		switch(c)
		{
			case 'r': //Repeat
			{
				repeat=atoi(optarg);
			};break;
			case 's': //Sleep
			{
				wait=atoi(optarg);
			};break;
			case 'p': //Port
				{
					port=atoi(optarg);
				};break;
			case 't': //Timeout
				{
					MB_TIMEOUT=1000*atoi(optarg);
				};break;
			case 'd': //Device Id
				{
					MB_UNIT_IDENTIFIER=atoi(optarg);
				};break;
			case 'n': //Number of value to read
				{
					num=atoi(optarg);
				};break;
			case 'w': //Write value
				{
					w=1;
					value=atof(optarg);
				};break;
			case '?':
			case 'h':
				{
					printf("%s (Build on %s %s)\n",LOG_TITLE,__DATE__,__TIME__);
					printf("usage: %s:[-?,h] [-p1~65535] [-t1...] [-d0~255] [-n1..] [-w Value] node_adress var\n",argv[0]);
					printf("-r\tRepeat (Default : %d)\n",repeat);
					printf("-s\tRepeat interval (Default : %d ms)\n",wait);
					printf("-p\tPort (Default : %d)\n",port);
					printf("-t\tTimeout (Default : %d s)\n",MB_TIMEOUT/1000);
					printf("-n\tNumber of value to read(Default : 1)\n");
					printf("-d\tDevice Id(Default : 1)\n");
					printf("-w\tValue to write\n");
					return(0);
				}break;
			default:break;
		}
	if (optind < argc)
	{
		ip=argv[optind];
		strcpy(TagName,argv[optind+1]);
	}

	int sock=MBOpenSock(ip,port);
	if (sock==MBError)
	{
		MBLog("opensock : %s\n",mb_err_msg);
		return(1);
	}
	for(rep=0;rep<repeat;rep++)
	{
		if (w)
		{
			mb_write_rsp *rsp=MBWrite(sock,TagName,1,&value);
			//mb_write_rsp *rsp=_MBWrite_Ext(sock,timeout,MBGet_Transaction_Id(),UNIT_IDENTIFIER,TagName,1,&value);
			if(rsp==NULL)
			{
				MBLog("Write error : %s\n",mb_err_msg);
			}
			MBLog("%s Write %s \n",TagName,mb_err_msg);
			free(rsp);
		}

		mb_read_rsp *rsp=MBRead(sock,TagName,num);
		//mb_read_rsp *rsp=_MBRead_Ext(sock,timeout,MBGet_Transaction_Id(),UNIT_IDENTIFIER,TagName,num);

		if(rsp==NULL)
		{
			MBLog("Read error ! : %s\n",mb_err_msg);
			return(1);
		}
		MBLog("Read %s \n",mb_err_msg);
		for(i=0;i<rsp->Number;i++)
		{
			MBLog("Value (%s + %d) : %.8X  (%d = %g) (%s)\n",TagName,i,MBGetValueAsInteger(rsp,i),MBGetValueAsInteger(rsp,i),MBGetValueAsReal(rsp,i),mb_err_msg);
		}
		free(rsp);
		if (rep+1<repeat) sleep(wait/1000);
	}
	close(sock);
  return(0);

	exit(0);
}
