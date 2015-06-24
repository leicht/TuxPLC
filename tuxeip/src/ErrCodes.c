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

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "ErrCodes.h"

THREAD_VAR int _cip_debuglevel=LogNone;
THREAD_VAR unsigned int _cip_errno;
THREAD_VAR unsigned int _cip_ext_errno;
THREAD_VAR Error_type _cip_err_type;
THREAD_VAR char _cip_err_msg[MAX_ERR_MSG_LEN+1];

/**
 *
 * @param level Level of debugging messages
 * @param buffer
 * @param size
 * This function is used to print buffer
 */
EXPORT void FlushCipBuffer(int level,void *buffer,int size)
{	if (level<=_cip_debuglevel)
	{
		int i;
		if (buffer==NULL) return;
		for(i=0;i<size;i++)
		{
			if (!(i % 10))
			{
				if (i>0)
				{
					fprintf(stdout,"\n");
				}
				fprintf(stdout,"%p (%04d) :\t",(char*)buffer+i,i);
			}
			fprintf(stdout,"%02X ",*((unsigned char*)((char*)buffer+i)));
		}
		fprintf(stdout,"\n");
		fflush(stdout);
	}
}
EXPORT void LogCip(int level,char *format,...)
{	va_list list;
	va_start(list,format);//NULL
	if (level<=_cip_debuglevel)
	{
		vfprintf(stdout,format,list);
		/*if (format!="\n")
		{
			//fprintf("",time(NULL));
		} else vfprintf(stdout,format,list);*/
	}
	va_end(list);
}
#ifdef _WINDLL
EXPORT unsigned int cip_errno()
{
	return(_cip_errno);
};
EXPORT unsigned int cip_ext_errno()
{
	return(_cip_ext_errno);
};
EXPORT Error_type cip_err_type()
{
	return(_cip_err_type);
};
EXPORT char *cip_err_msg()
{
	return(_cip_err_msg);
};
#endif
EXPORT char *CIPGetInternalErrMsg(unsigned int ErrorCode)
{
	switch (ErrorCode){
		case E_Success:return("Success");
		case E_Error:return("Error");
		case E_SeeErrorno:return("System error");
		case E_PendingBufferFull:return("Pending Buffer Full");
		case E_NoMoreData:return("No More Data");
		case E_TimeOut:return("TimeOut");
		case E_ConnectionFailed:return("Connection Failed");
		case E_UnsolicitedMsg:return("Unsolicited Message");
		case E_NothingToSend:return("Nothing to send");
		case E_NoReply:return("There is no reply");
		case E_ItemUnknow:return("Item type unknow");
		case E_InvalidReply:return("Invalid reply");
		case E_OutOfRange:return("Index out of range");
		case E_UnsupportedService:return("Unsupported Service");
		case E_UnsupportedDataType:return("Unsupported DataType");
		case E_InternalMemory:return("Internal Memory");
		case E_BufferTooSmall:return("Buffer Too Small");
		case E_SendTimeOut:return("Send TimeOut");
    case E_RcvTimeOut:return("Receive TimeOut");
    case E_SendError:return("Sending Error");
    case E_RcvError:return("Receive Error");
		/*case :return("");
		case :return("");
		case :return("");
		case :return("");
		case :return("");	*/
		default :return("Reserved for future expansion");
	}
}
EXPORT char *CIPGetEipErrMsg(unsigned int ErrorCode)
{
	switch (ErrorCode){
		case 0x0000:return("Success");
		case 0x0001:return("The sender issued an invalid or unsupported encapsulation command");
		case 0x0002:return("Insufficient memory");
		case 0x0003:return("Poorly formed or incorrect data in the data portion");
		//case 0x0004-0x0063:return("Reserved for legacy RA");
		case 0x0064:return("an originator used an invalid session handle when sending an encapsulation message to the target");
		case 0x0065:return("The target received a message of invalide length");
		case 0x0066:
		case 0x0067:
		case 0x0068:return("Reserved for legacy RA");
		case 0x0069:return("unsupported encapsulation protocol revision ");
		//case 0x006A-0xFFFF:return("Reserved for future expansion");
		default :{if ((ErrorCode>=0x0004)&(ErrorCode<=0x0063)) return("Reserved for legacy RA");
							else if (ErrorCode>=0x006A) return("Reserved for future expansion");else return("unknow error code");}
	}
}
EXPORT char *CIPGetMRErrMsg(unsigned int ErrorCode,unsigned int Ext_ErrorCode)
{
	switch (ErrorCode){
		case 0x0000:return("Success");
		case 0x0001:
         {switch (Ext_ErrorCode)
          {
		        case 0x0100:return("Connection in use or Duplicate Forward Open");
		        case 0x0103:return("Transport Class and Trigger combination not supported");
		        case 0x0106:return("Ownership conflict");
		        case 0x0107:return("Connection not found at target application");
		        case 0x0108:return("Invalid session type");
		        case 0x0109:return("Invalid session size");
		        case 0x0110:return("Device not configured");
		        case 0x0111:return("RPI not supported");
		        case 0x0113:return("Connection managercannot support any more connections");
		        case 0x0114:return("Vendor Id or product code in the key segment did not match the device");
		        case 0x0115:return("Product type in the key segment did not match the device");
		        case 0x0116:return("Major or minor revision information in the key segment did not match the device");
		        case 0x0117:return("Invalid session point");
		        case 0x0118:return("Invalid configuration format");
		        case 0x0119:return("Connection request fails since there is no controlling session currently open");
            //
		        default :return ("Connection failure");
		       }
		     }
		case 0x0002:return("Resource unavailable");
		case 0x0003:return("Invalid parameters value");
		case 0x0004:return("Path segment error");
		case 0x0005:return("Path destination unknow");
		case 0x0006:return("Partial transfert");
		case 0x0007:return("Connection lost");
		case 0x0008:return("Service not supported");
		case 0x0009:return("Invalid attribute value");
		case 0x000A:return("Attribute list error");
		case 0x000B:return("Already in requested mode/state");
		case 0x000C:return("Object state conflict");
		case 0x000D:return("Object already exist");
		case 0x000E:return("Attribute not settable");
		case 0x000F:return("Privilege violation");
		case 0x0010:return("Device state conflict");
		case 0x0011:return("Reply data too large");
		case 0x0012:return("Fragmentation of a primitive value");
		case 0x0013:return("Not enough data");
		case 0x0014:return("Attribute not supported");
		case 0x0015:return("Too much data");
		case 0x0016:return("Object does not exist");
		case 0x0017:return("Service fragmentation sequence not in progress");
		case 0x0018:return("No stored attibute data");
		case 0x0019:return("Store operation failure");
		case 0x001A:return("Routing failure,request packet too large");
		case 0x001B:return("Routing failure,response packet too large");
		case 0x001C:return("Missing attribute list entry data");
		case 0x001D:return("Invalid attribute value list");
		case 0x001E:return("Embendded service error");
		case 0x001F:return("Vendor specific");
		case 0x0020:return("Invalid parameter");
		case 0x0021:return("Write once value or medium already written");
		case 0x0022:return("Invalid reply received");
		case 0x0025:return("Key failure in path");
		case 0x0026:return("Path size invalid");
		case 0x0027:return("Unexpected attribute in list");
		case 0x0028:return("Invalid member ID");
		case 0x0029:return("Member not settable");
		case 0x002A:return("Group 2 only server general failure");

		default :{if ((ErrorCode>=0x002B)&(ErrorCode<=0x00CF)) return("Reserved for future extension");
							else if (ErrorCode>=0x00D0) return("Reserved Object/Class/services errors");else return("unknow error code");}
	}
}
EXPORT char *CIPGetABErrMsg(unsigned int ErrorCode,unsigned int Ext_ErrorCode)
{
	switch (ErrorCode&0xff){
		case 0x0000:return("Success");
		case 0x01FF:
         {switch (Ext_ErrorCode)
          {
		        case 0x2105:return("You have tried to access beyond the end of the data object");
		        case 0x2107:return("The abbreviated type does not match the data type of the data object");
		        case 0x2104:return("The beginning offset was beyond the end of the template");
            //
		        default :return ("Unknow Extended AB Error type");
		       }
		     }
		case 0x0004:return("The IOI could not be deciphered");
		case 0x0005:return("The particular item referenced could not be found");
		case 0x0006:return("The amount of data requested would not fit into the response buffer.Partial data transfer has occured");
		case 0x000a:return("An error has occured trying to process one of the attributes");
		case 0x0013:return("Not enougth command / data were spplied in the command to execute the service requested");
		case 0x001c:return("An insufficient number of attributes were provided compared to the attributes count");
		case 0x0026:return("The IOI WORD length did not match the amount of IOI witch was processed");
		default :return("Unknow AB Error type");
		}
}
EXPORT char *CIPGetPCCCErrMsg(unsigned int ErrorCode,unsigned int Ext_ErrorCode)
{
	switch (ErrorCode&0xff){
		case 0x00:return("Success");
		case 0x01:return("DST node is out of buffer space");
		case 0x02:return("Cannot guarantee delivery:link layer");
		case 0x03:return("Duplicate token holder detected");
		case 0x04:return("Local port is disconnected");
		case 0x05:return("Application layer timed out waiting for a response");
		case 0x06:return("Duplicate node detected");
		case 0x07:return("Station is offline");
		case 0x08:return("Hardware fault");
		case 0x10:return("Illegal command or format");
		case 0x20:return("Host has a problem and will not communicate");
		case 0x30:return("Remote node host is missing,disconnected,or shut down");
		case 0x40:return("Host could not complete function due to hardware fault");
		case 0x50:return("Adressing problem or memory protect rungs");
		case 0x60:return("Function not allowed due to command protection selection");
		case 0x70:return("Processor is in program mode");
		case 0x80:return("Compatibility mode file missing or communicate zone problem");
		case 0x90:return("Remote node cannot buffer command");
		case 0xA0:return("Wait ACK");
		case 0xB0:return("Remote node problem due to download");
		case 0xC0:return("Wait ACK");
		case 0xD0:return("not used");
		case 0xE0:return("not used");
		case 0xF0:
         {switch (Ext_ErrorCode)
          {
		        case 0x00:return("not used");
						case 0x01:return("A field has an illegal value");
						case 0x02:return("Less levels specified in adress than minimum for any adress");
						case 0x03:return("More levels specified in adress than system supports");
						case 0x04:return("Symbol not found");
						case 0x05:return("Symbol is of improper format");
						case 0x06:return("Adress doesn't point to something usable");
						case 0x07:return("File is wrong size");
						case 0x08:return("Cannot complete request,situation has changed..");
						case 0x09:return("Data or file is too large");
						case 0x0A:return("Transaction size plus word adress is too large");
						case 0x0B:return("Access denied,improper privilege");
						case 0x0C:return("Condition cannot be generated-resource is not available");
						case 0x0D:return("Condition already exists-resource is already available");
						case 0x0E:return("Command cannot be executed");
						case 0x0F:return("Histogram overflow");
						case 0x10:return("No access");
						case 0x11:return("Illegal data type");
						case 0x12:return("Invalid parameter or invalid data");
						case 0x13:return("Adress reference exist to deleted area");
						case 0x14:return("Command execution failure for unknow reason");
						case 0x15:return("Data conversion error");
						case 0x16:return("Scanner not able to communicate with 1771 rack adapter");
						case 0x17:return("Type mismatch");
						case 0x18:return("1771 module response was not valid");
						case 0x19:return("Duplicate label");
						case 0x22:return("Remote rack fault (passthru from a DH+ link)");
						case 0x23:return("Timeout (passthru from a DH+ link)");
						case 0x24:return("Unknow error (passthru from a DH+ link)");
						case 0x1A:return("File is open;another node owns it");
						case 0x1B:return("Another node is the program owner");
						case 0x1C:return("Reserved");
						case 0x1D:return("Reserved");
						case 0x1E:return("Data table element protection violation");
            case 0x1F:return("Temporary internal problem");
		        default :return ("Unknow PCCC Error type");
		       }
		     }
		default :return ("Unknow PCCC Error type");
		}
}
EXPORT char *CIPGetErrMsg(Error_type s_err_type,unsigned int s_errno,unsigned int Ext_ErrorCode)
{
 switch (s_err_type){
  case Internal_Error:return(CIPGetInternalErrMsg(s_errno));
  case Sys_Error:return(strerror(s_errno));
  case EIP_Error:return(CIPGetEipErrMsg(s_errno));
	case MR_Error:return(CIPGetMRErrMsg(s_errno,Ext_ErrorCode));
	case AB_Error:return(CIPGetABErrMsg(s_errno,Ext_ErrorCode));
	case PCCC_Error:return(CIPGetPCCCErrMsg(s_errno,Ext_ErrorCode));
  default :return("Unknow Error type");
 }
}
