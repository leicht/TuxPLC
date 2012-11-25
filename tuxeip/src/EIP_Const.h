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

#ifndef _EIP_CONST_H
#define _EIP_CONST_H

#ifdef __cplusplus
extern "C"
{
#endif
		
/* Misc Ctes */
#define MAX_MSG_LEN 1024 // a verifier

#define EIP_PORT	0xAF12 //44818
#define ENCAP_PROTOCOL 0x0001

/* Ethernet IP commands */
#define EIP_NOP	0x0000
#define EIP_LISTTARGETS	0x0001 // Reserved for legacy RA
#define EIP_LISTSERVICES	0x0004
#define EIP_LISTIDENTITY	0x0063
#define EIP_LISTINTERFACES	0x0064
#define EIP_REGISTERSESSION	0x0065
#define EIP_UNREGISTERSESSION	0x0066
#define EIP_SENDRRDATA		0x006F
#define EIP_SENDUNITDATA	0x0070
#define EIP_INDICATESTATUS	0x0072
#define EIP_CANCEL		0x0073

// Ethernet IP status code
#define EIP_SUCCESS 0x0000
#define EIP_INVALID_COMMAND 0x0001
#define EIP_MEMORY 0x0002
#define EIP_INCORRECT_DATA 0x0003
#define EIP_INVALID_SESSION_HANDLE 0x0064
#define EIP_INVALID_LENGTH 0x0065
#define EIP_UNSUPPORTED_PROTOCOL 0x0069

// Ethernet IP Services Class
#define EIP_SC_COMMUNICATION 0x0100
#define EIP_VERSION 0x01

#ifdef __cplusplus
}
#endif

#endif /* _EIP_CONST_H */
