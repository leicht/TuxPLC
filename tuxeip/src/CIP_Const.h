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
 
#ifndef _CIP_CONST_H
#define _CIP_CONST_H

#ifdef __cplusplus
extern "C"
{
#endif

#define TuxPlcVendorId 1030
#define CIP_Success 0x0000

/************************ Item Id numbers  see vol 2/ch 2 *************************/

#define ItemId_Null 			0x0000 // used for UCMM messages
#define ItemId_ListIdentityResponse 	0x000C
#define ItemId_ConnectionBased 		0x00A1 // used for connected messages
#define ItemId_ConnectedTP 		0x00B1 // connected transport packet
#define ItemId_UCM 			0x00B2 // unconnected message
#define ItemId_ListServiceResponse 	0x0100
#define ItemId_OTSocketInfo 		0x8000// originator to target socket info
#define ItemId_TOSocketInfo 		0x8001// target to originator socket info
#define ItemId_Sequenced 		0x8002// sequenced adress item

/**************************** Cip Path ********************************/

#define CIP_PortSegment		0x00	// Port segment
#define CIP_LogicalSegment	0x20	// Logical segment
#define CIP_NetworkSegment	0x40	// Network segment
#define CIP_SymbolicSegment	0x60	// Symbolic segment
#define CIP_DataSegment		0x80	// Data segment
#define CIP_DataTypeC		0xA0	// Data type constructed (see C-2.2)
#define CIP_DataTypeE		0xC0	// Data type elementary (see C-2.1)

#define CIP_SegmentTypeMask	0xE0	// segment Mask

 //Port Segment
#define CIP_PS_ExtendedLinkMask	0x10	// ExtendedLink Mask
#define CIP_PS_PortIDMask	0x0F	// Port Identifier Mask

 //Logical Segment
#define CIP_LS_LogicalTypeMask	0x1C	// LogicalType Mask
#define CIP_LS_ClassId		0x00    // LogicalType Class Id
#define CIP_LS_InstanceId	0x04    // LogicalType Instance Id
#define CIP_LS_MemberId		0x08    // LogicalType Member Id
#define CIP_LS_ConnectionPoint	0x0C    // LogicalType ConnectionPoint
#define CIP_LS_AttributeId		0x10    // LogicalType Attribute Id
#define CIP_LS_Special			0x14    // LogicalType Special *
#define CIP_LS_ServiceId		0x18    // LogicalType Service ID *
#define CIP_LS_Reserved			0x1C    // futur
#define CIP_LS_LogicalFormatMask	0x03	// LogicalFormat Mask
#define CIP_LS_8BitLogicalAdress	0x00	// LogicalFormat Mask 8 Bit Logical Adress
#define CIP_LS_16BitLogicalAdress	0x01	// LogicalFormat Mask 16 Bit Logical Adress
#define CIP_LS_32BitLogicalAdress	0x02	// LogicalFormat Mask 32 Bit Logical Adress
#define CIP_LS_ReservedBitLogicalAdress	0x03	// Reserved for futur use

 //Symbolic Segment
#define CIP_SS_SymbolicTypeMask	0x60; // S

#define CIP_Segment	0x00..0x9F; // encoding for segment
#define CIP_Data	0xA0..0xDF;  // encoding for data type reporting
#define CIP_Reserved	0xE0..0xFF; // futur use

	
#ifdef __cplusplus
}
#endif

#endif /* _CIP_CONST_H */
