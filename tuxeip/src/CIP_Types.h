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

#ifndef _CIP_TYPES_H
#define _CIP_TYPES_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "TuxDef.h"

#ifndef WINDOWS
	typedef unsigned char BYTE;
	typedef unsigned short WORD;
#else
		 #include <windef.h>
#endif

typedef unsigned int LONGWORD;

typedef char CIP_SINT;		//128..127	8 bits signed
typedef short CIP_INT;		//32768..32767	16 bits signed
typedef int CIP_DINT;			//2147483648..2147483647	32 bits signed
typedef long int CIP_LINT;//2^0..2^63 64 bits signed
typedef BYTE CIP_USINT;		//0..255	8 bits non signed
typedef WORD CIP_UINT;		//0..65535	16 bits non signed
typedef LONGWORD CIP_UDINT;//0..4294967295	32 bits non signed

typedef enum _EPath_Format{FPadded,FPacked} EPath_Format;
typedef enum _EPath_LogicalAdress{_8Bits,_16Bits,_32Bits} EPath_LogicalAdress;
//typedef BYTE Path_Segment;

#ifdef BCC32
	//typedef Path_Segment EPath[0];
	//typedef Path_Segment EPath;
	//typedef EPath *PEPath;
#endif

#ifdef __cplusplus
}
#endif

#endif /* _CIP_TYPES_H */
