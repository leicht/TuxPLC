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

#ifndef _MISC_H
#define _MISC_H

#ifdef __cplusplus
extern "C"
{
#endif

void Wait(int tps);
void _FlushBuffer(void *buffer,int size);
void _FlushAsciiBuffer(void *buffer,int size);
void _Log(int level,char *format,...);
int OpenServerSock(char *port);
int OpenSock(char *serveur,char *port);
int _opencom (char *comdev);

#ifdef INCLUDE_SRC
#include "Misc.c"
#endif

#ifdef __cplusplus
}
#endif

#endif /* _MISC_H */
