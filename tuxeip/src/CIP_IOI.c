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

#ifdef _WIN32
    #include <limits.h>
#else

#endif

#include <string.h>
#include <stdlib.h>

#include "CIP_IOI.h"
#include "CM.h"


BYTE CM_PATH[4]={0x20,CONNECTION_MANAGER,0x24,0x01};
BYTE ROUTER_PATH[4]={0x20,ROUTER,0x24,0x01};
BYTE BACKPLANE_DATA_PATH[4]={0x20,BACKPLANE_DATA,0x24,0x01};
BYTE PCCC_PATH[4]={0x20,OBJECT_PCCC,0x24,0x01};
BYTE DHPA_PROXY_PATH[6]={0x20,OBJECT_DHP,0x24,0x01,0x2C,0x01};
BYTE DHPB_PROXY_PATH[6]={0x20,OBJECT_DHP,0x24,0x02,0x2C,0x01};

int _AddSegmentValue(BYTE *IOI,int value);

int _BuildIOI(BYTE *IOI,char *adress)
{ char *b,*e,*p=adress,*l=adress+strlen(adress);
	int ioilength=0;
	while (p<l)
	{
		b=strchr(p,'[');
		e=strchr(p,'.');
		if ((b==NULL) && (e==NULL))
		{
			if (IOI!=NULL) ioilength+=_BuildIOIString(IOI+ioilength,p,l-p);
				else ioilength+=_BuildIOIString(NULL,p,l-p);
			p=l;
		}
		if ((b!=NULL)&&(((e!=NULL)&&(b<e))||(e==NULL))) // "[" before "."
		{ e=strchr(p,']');
			if (b>p)
				{
					if (IOI!=NULL) ioilength+=_BuildIOIString(IOI+ioilength,p,b-p);
						else ioilength+=_BuildIOIString(NULL,p,b-p);	
				}	
				
			if (IOI!=NULL) ioilength+=_BuildIOIArray(IOI+ioilength,b+1,e-b);
				else ioilength+=_BuildIOIArray(NULL,b+1,e-b);
			p=e+1;
		}
		if ((e!=NULL)&&(((b!=NULL)&&(b>e))||(b==NULL))) // "." before "["
		{ 
			if (IOI!=NULL) ioilength+=_BuildIOIString(IOI+ioilength,p,e-p);
				else ioilength+=_BuildIOIString(NULL,p,e-p);	
			p=e+1;
		}
	}
	return(ioilength);	
}
int _AddSegmentValue(BYTE *IOI,int value)
{ int valuelength=0;
	//int ioilength=0;
	
	if (value>0xFFFF) valuelength=4;
		else {if (value>0xFF) valuelength=2;else valuelength=1;}
	switch (valuelength)
	{
		case 1:
			if (IOI!=NULL)
			{
				IOI[0]=0x28;
				IOI[1]=(BYTE)value;
			}
			return(2);
		case 2:
			if (IOI!=NULL)
			{
				IOI[0]=0x29;
				IOI[1]=0x00;
				*((CIP_UINT*)(&IOI[2]))=value;
			}
			return(4);
		case 4:
			if (IOI!=NULL)
			{
				IOI[0]=0x2A;
				IOI[1]=0x00;
				*((CIP_UINT*)(&IOI[2]))=value;
			}
			return(6);
		default:return(0);
	}
	//return(ioilength);
}

int _BuildIOIArray(BYTE *IOI,char *adress,int size)
{ char *b,*p=adress,*l=adress+(strlen(adress)<size ? strlen(adress):size);// MIN(strlen(adress),size);
	int ioilength=0;
	int value=0;

	while (p<l)
	{ b=strchr(p,',');
		value=atoi(p);
		if (IOI!=NULL) ioilength+=_AddSegmentValue(IOI+ioilength,value);
			else ioilength+=_AddSegmentValue(NULL,value);
		if (b==NULL) break;
		p=++b;	
	}
	return(ioilength);
}
int _BuildIOIString(BYTE *IOI,char *adress,int size)
{ int pathlen=0;
	int adresslen=(strlen(adress)<size? strlen(adress):size);//MIN(strlen(adress),size);
	if(adresslen<=0) return(0);
	pathlen=2+adresslen+adresslen%2;
	if (IOI==NULL) return(pathlen);
		
	memset(IOI,0,pathlen);
	//if (IOI==NULL){CIPERROR(Sys_Error,errno,0);return(Error);}
	IOI[0]=0x91;
	IOI[1]=adresslen;
	memcpy(&(IOI[2]),adress,adresslen);
	return(pathlen);
}
