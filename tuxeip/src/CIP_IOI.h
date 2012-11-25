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

#ifndef _CIP_IOI_H
#define _CIP_IOI_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "TuxDef.h"
#include "CIP_Types.h"
#pragma pack (1)

/************* Classes : See CIP Common spec Vol.1/Ch 5. *****************/

#define IDENTITY 0x01
#define ROUTER 0x02
#define ASSEMBLY 0x04
#define CONNECTION 0x05
#define CONNECTION_MANAGER 0x06
#define REGISTER 0x07

#define PORT_OBJECT 0xf4
#define TCPIP_INTERFACE_OBJECT 0xf5
#define ETHERNET_LINK_OBJECT 0xf6

/* Rockwell Classes */

#define BACKPLANE_DATA 0x66
#define OBJECT_PCCC 0x67
#define OBJECT_DETAILS 0x68
#define OBJECT_PROP 0x6a
#define OBJECT_LIST 0x6b
#define STRUCT_LIST 0x6c
#define OBJECT_DHP	0xA6

/********* Common services : See CIP Common spec Vol.1/Ch 5.& Appendix A. **********/

#define GET_ATTRIBUTE_ALL 0x01
#define SET_ATTRIBUTE_ALL 0x02
#define GET_ATTRIBUTE_LIST 0x03
#define SET_ATTRIBUTE_LIST 0x04
#define RESET 0x05
#define START 0x06
#define STOP 0x07
#define CREATE 0x08
#define DELETE 0x09
#define MULTIPLE_SERVICE_PACKET 0x09
#define APPLY_ATTRIBUTES 0x0d
#define GET_ATTRIBUTE_SINGLE 0x0e
#define SET_ATTRIBUTE_SINGLE 0x10
#define FIND_NEXT_OBJECT_INSTANCE 0x11
#define RESTORE 0x15
#define SAVE 0x16
#define NOP 0x17
#define GET_MEMBER 0x18
#define SET_MEMBER 0x19
#define INSERT_MEMBER 0x1a
#define REMOVE_MEMBER 0x1b


#define GET_ALL_INSTANCES 0x4b
#define GET_SINGLE_PROP 0x4f
#define PUT_SINGLE_PROP 0x50

#define EXECUTE_PCCC 0x4b
/*
#define CONN_KICK_TIMER 0x4b
#define CONN_OPEN 0x4c
#define CONN_CLOSE 0x4d
#define CONN_STOP 0x4e
#define CONN_CHANGE_START 0x4f
#define CONN_GET_STATUS 0x50
#define CONN_CHANGE_COMPLETE 0x51
#define CONN_AUDIT_CHANGE 0x52
#define GET_ALL_INSTANCES 0x4b
#define GET_SINGLE_PROP 0x4f
#define PUT_SINGLE_PROP 0x50*/

/************* Commons Class attributes : See CIP Common spec Vol.1/Ch 4. *****************/

#define REVISION 											0x01
#define MAX_INSTANCE 									0x02
#define NUM_INSTANCE 									0x03
#define OPT_ATTRIBUTE_LIST 						0x04
#define OPT_SERVICE_LIST 							0x05
#define MAX_ID_NUM_CLASS_ATTRIBUTE 		0x06
#define MAX_ID_NUM_INSTANCE_ATTRIBUTE 0x07

/********************************************************************************/

extern BYTE CM_PATH[4];//={0x20,CONNECTION_MANAGER=0x06,0x24,0x01};
extern BYTE ROUTER_PATH[4];//={0x20,ROUTER=0x02,0x24,0x01};
extern BYTE BACKPLANE_DATA_PATH[4];//={0x20,BACKPLANE_DATA=0x66,0x24,0x01};
extern BYTE PCCC_PATH[4];//={0x20,OBJECT_PCCC=0x67,0x24,0x01};
extern BYTE DHPA_PROXY_PATH[6];//={0x20,OBJECT_DHP=0xA6,0x24,Channel A=0x01,0x2C,0x01};
extern BYTE DHPB_PROXY_PATH[6];//={0x20,OBJECT_DHP=0xA6,0x24,Channel B=0x02,0x2C,0x01};


/********************************************************************************/

 typedef struct _Cip_PortSegmentR{
		CIP_UINT LinkAdressSize;
		CIP_UINT PortId;
		//BYTE LinkAdress[0];
		}PACKED Cip_PortSegmentR;
typedef Cip_PortSegmentR *PCip_PortSegmentR;

typedef struct _Cip_Key{
		CIP_UINT VendorId;
		CIP_UINT DeviceType;
		CIP_UINT ProductCode;
		BYTE MajorRev;
		CIP_USINT MinorRev;
		CIP_INT Compatibility;
		}PACKED Cip_Key;
typedef Cip_Key *PCip_Key;

typedef struct _Cip_LogicalSegmentR{   // Unused value = $FFFF
		CIP_UINT ClassId;
		EPath_LogicalAdress F_ClassId;
		CIP_UINT InstanceId;
		EPath_LogicalAdress F_InstanceId;
		CIP_UINT MemberId;
		EPath_LogicalAdress F_MemberId;
		CIP_UINT Connection;
		EPath_LogicalAdress F_Connection;
		CIP_UINT AttibuteId;
		CIP_UINT ServiceId;
		PCip_Key Key;
		}PACKED Cip_LogicalSegmentR;
typedef  Cip_LogicalSegmentR *PCip_LogicalSegmentR;

typedef struct _Cip_SymbolicSegmentR{
		BYTE StringDepth;
		char StringValue[31];
		BYTE NumericDepth;
		CIP_UDINT NumericValue;
		}PACKED Cip_SymbolicSegmentR;
typedef Cip_SymbolicSegmentR *PCip_SymbolicSegmentR;

/*********************************                   ********************************/

int _BuildIOIString(BYTE *IOI,char *adress,int size);
int _BuildIOIArray(BYTE *IOI,char *adress,int size);
int _BuildIOI(BYTE *IOI,char *adress);

#ifdef __cplusplus
}
#endif

#endif /* _CIP_IOI_H */
