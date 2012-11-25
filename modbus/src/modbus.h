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

#ifndef _MODBUS_H
#define _MODBUS_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef WIN32
    #define MIN(x,y) (x<y ? x : y)
    //#define errno WSAGetLastError
#endif

#ifndef DLLIMPORT
  #define DLLIMPORT
#endif
#ifndef DLLEXPORT
  #define DLLEXPORT
#endif

#ifndef VALGRING
	/* Comment this to use with valgrind */
	#define THREAD_VAR __thread
#else
	#define THREAD_VAR
#endif

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include "MBErrCodes.h"
#include "MBCommon.h"
#include "MBClient.h"


#ifdef INCLUDE_SRC
#include "./MBErrCodes.c"
#include "./MBCommon.c"
#include "./MBClient.c"

#endif

#ifdef __cplusplus
}
#endif

#endif /* _MODBUS_H */
