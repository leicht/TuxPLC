/***************************************************************************
 *  Copyright (C) 2006 http://www.foxinfo.fr                              *
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

#ifndef _MBERRCODES_H
#define _MBERRCODES_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef VALGRING
	#define THREAD_VAR __thread
#else
	#define THREAD_VAR
#endif

#ifndef DLLIMPORT
  #define DLLIMPORT
#endif
#ifndef DLLEXPORT
  #define DLLEXPORT
#endif

#define MAX_ERR_MSG_LEN 255

/********** Internal codes ******************/
#define MBSuccess 0
#define MBError -1
#define MBSeeErrorno -2
#define MBPError -3
#define MBConnectionFailed -4
#define MBTimeOut -5
#define MBRcvError -6
#define MBBadParameter -7
#define MBOutOfRange -8
#define MBSizeClamped -9
#define MBUnsolicitedMsg -10
#define MBTypeMismatch -11
#define MBPDUTooLarge -12
#define MBBadSocket -13

/******************** AB error codes *********************/

typedef enum _MB_ErrorType{
	MB_InternalError, // Error in library
	MB_SysError, // System Error
	MB_Error // Modbus Error
} MB_ErrorType;

extern THREAD_VAR MB_ErrorType _mb_err_type;
extern THREAD_VAR unsigned int _mb_errno;
extern THREAD_VAR unsigned int _mb_ext_errno;
extern THREAD_VAR char _mb_err_msg[MAX_ERR_MSG_LEN+1];

DLLEXPORT char *MBGetInternalErrMsg(unsigned int ErrorCode);
DLLEXPORT char *MBGetMBErrMsg(unsigned int ErrorCode,unsigned int Ext_ErrorCode);

DLLEXPORT char *MBGetErrMsg(MB_ErrorType _mb_err_type,unsigned int _mb_errno,unsigned int _mb_ext_errno);

#define MBERROR(type,no,ext_no) {_mb_err_type=type;_mb_errno=no;_mb_ext_errno=ext_no;memcpy(&_mb_err_msg,MBGetErrMsg(_mb_err_type,_mb_errno,_mb_ext_errno),MAX_ERR_MSG_LEN);}

#define mb_err_type _mb_err_type
#define mb_errno _mb_errno
#define mb_ext_errno _mb_ext_errno
#define mb_err_msg _mb_err_msg

#ifdef __cplusplus
}
#endif

#endif /* _MBERRCODES_H */
