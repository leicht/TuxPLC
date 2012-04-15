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

#ifndef _MBERRCODES_C
#define _MBERRCODES_C

#include "MBErrCodes.h"

#include <string.h>
#include <errno.h>

THREAD_VAR unsigned int _mb_errno;
THREAD_VAR unsigned int _mb_ext_errno;
THREAD_VAR MB_ErrorType _mb_err_type;
THREAD_VAR char _mb_err_msg[MAX_ERR_MSG_LEN+1];

DLLEXPORT char *MBGetInternalErrMsg(unsigned int ErrorCode)
{
	switch (ErrorCode){
		case MBSuccess:return("Success");
		case MBError:return("Error");
		case MBSeeErrorno:return("System error");
		case MBPError:return("MBPError");
		case MBConnectionFailed:return("MBConnectionFailed");
		case MBTimeOut:return("MBTimeOut");
		case MBRcvError:return("MBRcvError");
		case MBBadParameter:return("Bad Parameters in request");
		case MBOutOfRange:return("Index Out of Range");
		case MBSizeClamped:return("Size have been clamped");
		case MBUnsolicitedMsg:return("Unsolicited Message");
		case MBTypeMismatch:return("Data type mismatch");
		case MBPDUTooLarge:return("PDU Too large");
		case MBBadSocket:return("Bad socket descriptor");
		/*case :return("");
		case :return("");
		case :return("");	*/
		default :return("Reserved for future expansion");
	}
}
DLLEXPORT char *MBGetMBErrMsg(unsigned int ErrorCode,unsigned int Ext_ErrorCode)
{
	switch (ErrorCode){
		case 0x00:return("Success");
		case 0x01:return("Illegal function");
		case 0x02:return("Illegal data address");
		case 0x03:return("Illegal data value");
		case 0x04:return("Slave device failure");
		case 0x05:return("Acknowledge");
		case 0x06:return("Slave device busy");
		case 0x08:return("Memory parity error");
		case 0x0A:return("Gateway path unavailable");
		case 0x0B:return("Gateway target device failed to respond");
		//case 0x006A-0xFFFF:return("Reserved for future expansion");
		default :
		{
			return("unknow error code");
		}
	}
}

DLLEXPORT char *MBGetErrMsg(MB_ErrorType _mb_err_type,unsigned int _mb_errno,unsigned int _mb_ext_errno)
{
	switch (_mb_err_type){
		case MB_InternalError:return(MBGetInternalErrMsg(_mb_errno));
		case MB_SysError:return(strerror(_mb_errno));
		case MB_Error:return(MBGetMBErrMsg(_mb_errno,_mb_ext_errno));
  default :return("Unknow Error type");
 }
}

#endif /* _MBERRCODES_C */
