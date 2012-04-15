/***************************************************************************
 *   Copyright (C) 2006 by TuxPLC					                                 *
 *   Author Stephane JEANNE s.jeanne@tuxplc.net                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef _TUXDEF_H
#define _TUXDEF_H

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(_WINDLL)
	#define EXPORT __declspec(dllexport)
#else
	#define EXPORT
#endif

#ifdef _WIN32
    #define MIN(x,y) (x<y ? x : y)
    //#define errno WSAGetLastError
#endif

#ifdef VALGRING /* Define this to use with valgrind */
    #define THREAD_VAR
#else
  #ifdef __MINGW32__
    #define THREAD_VAR
    #define PACKED __attribute__((packed))
  #elif _MSC_VER
    #ifndef THREAD_VAR
			#define THREAD_VAR __declspec( thread )
		#endif
    #define PACKED
    #pragma pack (1)
  #elif __BORLANDC__
    #define THREAD_VAR __declspec( thread )
    #define PACKED
    #pragma pack (1)
  #else
    #define THREAD_VAR __thread
    #define PACKED __attribute__((packed))
  #endif

#endif

#ifndef __GNUC__ /*MSG_WAITALL*/
  #define MSG_WAITALL 0x100
#endif

#ifdef __cplusplus
}
#endif

#endif /* _TUXDEF_H */
