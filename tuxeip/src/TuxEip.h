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

#ifndef _TUXEIP_H
#define _TUXEIP_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef WIN32
    #define MIN(x,y) (x<y ? x : y)
    #define errno WSAGetLastError
#endif

#include "ErrCodes.h"
#include "CIP_Types.h"
#include "CIP_Const.h"
#include "CIP_IOI.h"
#include "CIP_Objects.h"
#include "EIP_Const.h"
#include "Ethernet_IP.h"
#include "SendData.h"
#include "MR.h"
#include "CM.h"
#include "AB.h"
#include "LGX.h"
#include "PLC.h"

#ifdef INCLUDE_SRC
#include "ErrCodes.c"
#include "SendData.c"
#include "Ethernet_IP.c"
#include "CIP_IOI.c"
#include "MR.c"
#include "CM.c"
#include "AB.c"
#include "LGX.c"
#include "PLC.c"
#endif

#ifdef __cplusplus
}
#endif

#endif /* _TUXEIP_H */
