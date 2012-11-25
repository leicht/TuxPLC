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

/* I used this lines for compiling library+example (and debugging)*****/
#include "../src/TuxEip.h"
/**********************************************************************/
/* Use this one instead of the previous if you install TuxEip library */
//#include <tuxeip/TuxEip.h>
/**********************************************************************/
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#define space printf("\n")
#define flush(header) _CipFlushBuffer(header,24+header->Length);

int Connected_Mode=0; // 0 if you want to try the unconnected mode
		
int main(int argc,char *argv[])
{
	//cip_debuglevel=LogDebug; // You can uncomment this line to see the data exchange between TuxEip and your Logic controller

	char *IP="192.168.1.5\0";
	BYTE path[]={1,5,1,1,2,3};
// 	char *IP="10.140.200.48\0";
// 	BYTE path[]={1,2,2,3,1,2,2,1};
	int res;
	int count=1;
	char *var;
	int tns=getpid();
	Eip_Connection *connection=NULL;
	PLC_Read *data;
	
	printf("Starting, Pid = %d\n",getpid());

	if (argc<2)
	{
		printf("You have to provide a tag name\n");
		return(1);
	}
	var=argv[1];
			
	/* Openning a session */	
	printf("entering OpenSession \n");

	Eip_Session *session=OpenSession(IP);
	if (session==NULL)
	{
		printf("Error : OpenSession %s (%d:%d)\n",cip_err_msg,cip_errno,cip_ext_errno);
		return(1);
	}
	printf("OpenSession Ok\n");
	printf("entering RegisterSession \n");
	res=RegisterSession(session);

	if (res!=Error)
	{ int i,val_int,val_bool;
	float val_float;
	if (Connected_Mode)
	{
		printf("RegisterSession Ok\n");
		printf("entering ConnectPLCOverCNET\n");
		if (Connected_Mode)
			connection=ConnectPLCOverCNET(
			session, // session whe have open
			PLC5, // plc type
			0x12345678, // Target To Originator connection ID
			0x6789, // Connection Serial Number
			5000, // Request Packet Interval
			path, // Path to the ControlLogix
			sizeof(path) // path size
																									);
		if (connection!=NULL) printf("ConnectPLCOverCNET Ok, \n");
	}

	if ((connection!=NULL)||(!Connected_Mode))
	{
		printf("Reading %s\n",var);
		if (Connected_Mode)	data=ReadPLCData(session,connection,NULL,NULL,0,PLC5,tns++,var,count);
		else data=ReadPLCData(session,NULL,NULL,path,sizeof(path),PLC5,tns++,var,count);
		if (data!=NULL)
		{
			if (data->Varcount>0)
			{
				printf("ReadPLCData Ok :\n");
				printf("\telements :%d\n\tDatatype : %d\n\ttotalsize : %d\n\tElement size : %d\n",data->Varcount,data->type,data->totalsize,data->elementsize,data->mask);
				for (i=0;i<data->Varcount;i++)
				{
					val_bool=PCCC_GetValueAsBoolean(data,i);
					val_int=PCCC_GetValueAsInteger(data,i);
					val_float=PCCC_GetValueAsFloat(data,i);
					printf("Value [%d] = %f (%d)\n",i,val_float,val_int,val_bool);
				}
			} else printf("Error ReadPLCData : %s\n",cip_err_msg);
			free(data); // You have to free the data return by ReadLgxData
		} else
		{
			printf("Error : ReadPLCData %s (%d:%d)\n",cip_err_msg,cip_errno,cip_ext_errno);
		}
		if (connection!=NULL)
		{
			printf("entering Forward_Close\n");
			res=Forward_Close(connection);
			if (res!=Error)	printf("Forward_Close %s\n",cip_err_msg);
			else printf("Error : Forward_Close %s (%d:%d)\n",cip_err_msg,cip_errno,cip_ext_errno);
		}
	} else
	{
		printf("Error : ConnectPLCOverCNET %s (%d:%d)\n",cip_err_msg,cip_errno,cip_ext_errno);
	}	
	UnRegisterSession(session);	
	printf("UnRegister : %s\n",cip_err_msg);
	}else 
	{
		printf("Error : RegisterSession %s (%d:%d)\n",cip_err_msg,cip_errno,cip_ext_errno);
	}
	printf("entering CloseSession\n");
	CloseSession(session);

	return(0);
}
