/***************************************************************************
 *  Copyright (C) 2006 http://www.foxinfo.fr                               *
 *  Author : Stephane JEANNE    stephane.jeanne@gmail.com                  *
 *           Stephane LEICHT    stephane.leicht@foxinfo.fr                 *
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

#include "df1.h"

int file;
word tns;
struct timeval tv;
int Terminated;

/***********************************************************************************/
void Df1_close_device(int Df1_device)
{
  if (tcsetattr (Df1_device,TCSANOW,&saved_tty_parameters) < 0)
    MyLog("Can't restore terminal parameters ");
  close(Df1_device);
}

/************************************************************************************
		Df1_open_device : open the device
*************************************************************************************
input :
-------
Df1_port   : string with the device to open (/dev/ttyS0, /dev/ttyS1,...)
Df1_speed  : speed (baudrate)
Df1_parity : 0=don't use parity, 1=use parity EVEN, -1 use parity ODD
Df1_bit_l  : number of data bits : 7 or 8 	USE EVERY TIME 8 DATA BITS
Df1_bit_s  : number of stop bits : 1 or 2    ^^^^^^^^^^^^^^^^^^^^^^^^^^

answer  :
---------
device descriptor
************************************************************************************/
int Df1_open_device(char Df1_port[20], int Df1_speed, int Df1_parity, int Df1_bit_l, int Df1_bit_s)
{
  int fd;
MyLog("Port : %s,%d\n",Df1_port,Df1_speed);
  /* open port */
  fd = open(Df1_port,O_RDWR | O_NOCTTY  | O_NDELAY) ;//| O_NONBLOCK
  if(fd<0)
  {
    //MyLog("Open device failure\n") ;
    return ERROR_COM1 ;
  }

  /* save olds settings port */
  if (tcgetattr (fd,&saved_tty_parameters) < 0)
  {
    //MyLog("Can't get terminal parameters ");
    return ERROR_COM2 ;
  }

  /* settings port */
  bzero(&Df1_tio,sizeof(&Df1_tio));

  switch (Df1_speed)
  {
     case 0:
        Df1_tio.c_cflag = B0;
        break;
     case 50:
        Df1_tio.c_cflag = B50;
        break;
     case 75:
        Df1_tio.c_cflag = B75;
        break;
     case 110:
        Df1_tio.c_cflag = B110;
        break;
     case 134:
        Df1_tio.c_cflag = B134;
        break;
     case 150:
        Df1_tio.c_cflag = B150;
        break;
     case 200:
        Df1_tio.c_cflag = B200;
        break;
     case 300:
        Df1_tio.c_cflag = B300;
        break;
     case 600:
        Df1_tio.c_cflag = B600;
        break;
     case 1200:
        Df1_tio.c_cflag = B1200;
        break;
     case 1800:
        Df1_tio.c_cflag = B1800;
        break;
     case 2400:
        Df1_tio.c_cflag = B2400;
        break;
     case 4800:
        Df1_tio.c_cflag = B4800;
        break;
     case 9600:
        Df1_tio.c_cflag = B9600;
        break;
     case 19200:
        Df1_tio.c_cflag = B19200;
        break;
     case 38400:
        Df1_tio.c_cflag = B38400;
        break;
     case 57600:
        Df1_tio.c_cflag = B57600;
        break;
     case 115200:
        Df1_tio.c_cflag = B115200;
        break;
     case 230400:
        Df1_tio.c_cflag = B230400;
        break;
     default:
        Df1_tio.c_cflag = B9600;
  }
  switch (Df1_bit_l)
  {
     case 7:
        Df1_tio.c_cflag = Df1_tio.c_cflag | CS7;
        break;
     case 8:
     default:
        Df1_tio.c_cflag = Df1_tio.c_cflag | CS8;
        break;
  }
  switch (Df1_parity)
  {
     case 1:
        Df1_tio.c_cflag = Df1_tio.c_cflag | PARENB;
        break;
     case -1:
        Df1_tio.c_cflag = Df1_tio.c_cflag | PARENB | PARODD;
        break;
     case 0:
     default:
        Df1_tio.c_iflag = IGNPAR;
        break;
  }

  if (Df1_bit_s==2)
     Df1_tio.c_cflag = Df1_tio.c_cflag | CSTOPB;
     
  Df1_tio.c_cflag = Df1_tio.c_cflag | CLOCAL | CREAD ;
  Df1_tio.c_oflag = 0;
  Df1_tio.c_lflag = 0; /*1=ICANON;*/
  Df1_tio.c_cc[VMIN]=0;
  Df1_tio.c_cc[VTIME]=0;

  /* clean port */
  tcflush(fd, TCIFLUSH);

  fcntl(fd, F_SETFL, FASYNC);
  /* activate the settings port */
  if (tcsetattr(fd,TCSANOW,&Df1_tio) <0)
  {
    //MyLog("Can't set terminal parameters ");
    return ERROR_COM3;
  }
  
  /* clean I & O device */
  tcflush(fd,TCIOFLUSH);
  /* init tns for DF1 */
  tns = (word) time((time_t *)0);
  
  return fd ;
}


/***********************************************************************/
word calc_crc (word crc, word buffer) 
{
 word temp1, y;
 //MyLog ("buffer[x] = %04X\n",buffer);
 temp1 = crc ^ buffer;
 crc = (crc & 0xff00) | (temp1 & 0xff);
 for (y = 0; y < 8; y++)
  {
   if (crc & 1)
    {	  
		crc = crc >> 1;
		crc ^= 0xa001;
    }
   else
		crc = crc >> 1;
  }
 return crc;
}

/***********************************************************************/
word compute_crc (TBuffer * buffer) 
{
	byte x;
	word crc = 0;
	for (x=0;x<buffer->size;x++) { 
		crc = calc_crc (crc, buffer->data[x]);
	}
	crc = calc_crc (crc,ETX);
	//MyLog("CRC : %04X\n",crc);
	return (crc);
}


/***********************************************************************/
int send_DF1(TMsg Df1Data)
{
	int error=ERROR_SENDDF1;
	int flag;
	byte c;
	TBuffer crcBuffer,dataSend,dataRcv;
	time_t time_start;
	int nbr_NAK=0;
	int nbr_ENQ=0;
	//int x;
	
	//build message to send
	bzero(&crcBuffer,sizeof(crcBuffer));
	bzero(&dataSend,sizeof(dataSend));
	bzero(&dataRcv,sizeof(dataRcv));
	add_byte2buffer(&dataSend,DLE);	
	add_byte2buffer(&dataSend,STX);
	add_data2bufferWithDLE(&dataSend,Df1Data);
	add_byte2buffer(&dataSend,DLE);	
	add_byte2buffer(&dataSend,ETX);	
	add_data2buffer(&crcBuffer,&Df1Data,Df1Data.size+6);
	add_word2buffer(&dataSend, compute_crc(&crcBuffer));
	//for (x=0;x<dataSend.size;x++) {
	//	printf("datasend %i=%2X\n",x,dataSend.data[x]);
	//}
	//ready to send
	do 
	{	
		if (write (file, &dataSend, dataSend.size)!=dataSend.size)
			return error;
		time_start=time((time_t *) 0);
		tv.tv_sec = TIME_OUT;
      tv.tv_usec = 0;
		//Read for ACK or NAK
		do
		{
			if (is_timeout(time_start)==ERROR_TIMEOUT)
			{
				if (++nbr_ENQ>3) 
					return ERROR_TIMEOUT;
				else
				{
					sendResponse(ENQ);
					time_start=time((time_t *) 0);
					tv.tv_sec = TIME_OUT;
					tv.tv_usec = 0;
				}
			}
			flag=get_symbol(&c);
		} while ((flag!=CONTROL_FLAG) & ((c!=ACK) | (c!=NAK)));
		if (c==ACK)
			 return SUCCESS;
		if (c==NAK) 
		{
			error=ERROR_NAK;
			++nbr_NAK;
		}
	} while (nbr_NAK<=3);
	return error;
}	

/***********************************************************************/
int rcv_DF1(TMsg * Df1Data) 
{
	byte c,crcb1,crcb2;
	byte last_response;
	word crc;
	int error = ERROR_RCVDF1;
	TBuffer dataRcv;
	int flag;
	int crcOK=FALSE;
	
	do
	{
		bzero(&dataRcv,sizeof(dataRcv));
		last_response = NAK;
		do
		{
			flag=get_symbol(&c);
			if (flag<0) 
				return flag;
		} while (flag!=CONTROL_FLAG);
		switch (c)
		{	
		case ENQ : 
			sendResponse(last_response); 
			break;
		case STX : 
			while ((flag=get_symbol(&c))!=CONTROL_FLAG)
			{
				add_byte2buffer(&dataRcv,c);
			}
			if (c==ETX)
			{	
            if (read_byte(&crcb1)!=SUCCESS)
					return error;
            if (read_byte(&crcb2)!=SUCCESS)
					return error;

				crc=bytes2word(crcb1,crcb2);
				if (crc==compute_crc(&dataRcv)) 
				{
					sendResponse(ACK);
					crcOK=TRUE;
					error=SUCCESS;
				}	
				else
				{
					sendResponse(NAK);
					crcOK=FALSE;	
				}
			}
			else
			{
				sendResponse(NAK);
			} 
			break;
		default :	sendResponse(NAK);
		}
	} while (crcOK != TRUE);
	memcpy(Df1Data,dataRcv.data,dataRcv.size);
	return error;			
}

/***********************************************************************/
int get_symbol(byte * b)
{
	byte c1,c2;
	int error;
	if ((error=read_byte(&c1))!=SUCCESS)
		return error;
	if (c1==DLE)
	{
		if ((error=read_byte(&c2))!=SUCCESS) 
			return error;
		*b = c2;
		switch (c2)
		{
			case DLE: return DATA_FLAG;
			case ETX: return CONTROL_FLAG;
			case STX: return CONTROL_FLAG;
			case ENQ: return CONTROL_FLAG;
			case ACK: return CONTROL_FLAG;
			case NAK: return CONTROL_FLAG;	
		}	
	}
	*b = c1;
	return DATA_FLAG;	
}	

/***********************************************************************/
void sendResponse(byte response) // used to send NAK or ACK
{
	word w;
	byte dle=DLE;
	w=bytes2word(dle,response);
	write (file, &w, 2);
}	

/***********************************************************************/
char *Df1_GetDf1ErrMsg(int ErrorCode)
{
	switch (ErrorCode){
		case SUCCESS :				return("Success");
		case ERROR :				return("General error");
		case ERROR_NAK :			return("DF1 Error : ERROR_NAK");
		case ERROR_SENDDF1 :		return("DF1 Error : ERROR_SENDDF1");
		case ERROR_RCVDF1 :			return("DF1 Error : ERROR_RCVDF1");
		case ERROR_TIMEOUT :		return("DF1 Error : ERROR_TIMEOUT");
		case ERROR_BAD_QUERY :		return("DF1 Error : ERROR_BAD_QUERY");
		case ERROR_TNS_MISMATCH :	return("DF1 Error : ERROR_TNS_MISMATCH");
		case ERROR_READ_A2 :		return("DF1 Error : ERROR_READ_A2");
		case ERROR_READ_INTEGER :	return("DF1 Error : ERROR_READ_INTEGER");
		case ERROR_CALC_ADDRESS :	return("DF1 Error : ERROR_CALC_ADDRESS");
		case ERROR_WRITE_AA :		return("DF1 Error : ERROR_WRITE_AA");
		case ERROR_WRITE :			return("DF1 Error : ERROR_WRITE");
		case ERROR_WRITE_AB :		return("DF1 Error : ERROR_WRITE_AB");
		case ERROR_COM1 :			return("DF1 Error : Open device failure");
		case ERROR_COM2 :			return("DF1 Error : Can't get terminal parameters");
		case ERROR_COM3 :			return("DF1 Error : Can't set terminal parameters");
		default :
		{
			return("DF1 Error : unknow error code");
		}
	}
}

/***********************************************************************/
int read_byte(byte * b)
{
	fd_set readfs;
	int ndfs;
   int retval;
   FD_ZERO(&readfs);
   FD_SET(file, &readfs);
   ndfs = 1025;
	retval=select(ndfs, &readfs, NULL, NULL, &tv);
   switch (retval)
	{
		case 0: 	return ERROR_TIMEOUT;
		case -1: return retval;
		default :
		   if (FD_ISSET(file,&readfs))
			{	
				if (read(file, b, 1)!=1)
					return ERROR;
				else
				{
					//MyLog("Read :%02X\n",*b);
					return SUCCESS;
				}
			}
			else
				return ERROR;
	}

}

int calc_address(char *straddress,TThree_Address_Fields *address)
{
	int error=ERROR_CALC_ADDRESS;
	int x,l;
	bzero(address,sizeof(*address));
	address->size=0;
	address->fileNumber=0;
	address->fileType=0;
	address->eleNumber=0;
	address->s_eleNumber=0;
	for (x=0;x<strlen(straddress);x++)
		{
		switch (straddress[x])
			{
			case 'O':
				address->fileType = 0x8b;
				address->fileNumber = 0;
				address->size = 2;
				break;
			case 'I':
				address->fileType = 0x8c;
				address->fileNumber = 1;
				address->size = 2;
				break;
			case 'S':
				x++;
				address->fileType = 0x84;
				address->fileNumber = atoi(&straddress[x]);
				address->size = 2;
				break;
			case 'B':
				x++;
				address->fileType = 0x85;
				address->fileNumber = atoi(&straddress[x]);
				address->size = 2;
				break;
			case 'T':
				x++;
				address->fileType = 0x86;
				address->fileNumber = atoi(&straddress[x]);
				address->size = 2;
				break;
			case 'C':
				x++;
				address->fileType = 0x87;
				address->fileNumber = atoi(&straddress[x]);
				address->size = 2;
				break;
			case 'R':
				x++;
				address->fileType = 0x88;
				address->fileNumber = atoi(&straddress[x]);
				address->size = 2;
				break;
			case 'N':
				x++;
				address->fileType = 0x89;
				address->fileNumber = atoi(&straddress[x]);
				address->size=2;
				break;
			case 'F':
				x++;
				address->fileType = 0x8a;
				address->fileNumber = atoi(&straddress[x]);
				address->size = 4;
				break;
			case ':':
				address->eleNumber = atoi(&straddress[++x]);
				break;
			case '.':
			case '/':
				x++;
				if (isdigit(straddress[x])) {
					address->s_eleNumber = atoi(&straddress[x]);
				}
				l=strlen(straddress) - x;
				if (strncasecmp (&straddress[x],"acc",l) == 0 )
					address->s_eleNumber = 2;
				if (strncasecmp (&straddress[x],"pre",l) == 0 )
					address->s_eleNumber = 1;
				if (strncasecmp (&straddress[x],"len",l) == 0 )
					address->s_eleNumber = 1;
				if (strncasecmp (&straddress[x],"pos",l) == 0 )
					address->s_eleNumber = 2;
				if (strncasecmp (&straddress[x],"en",l) == 0 )
					address->s_eleNumber = 13;
				if (strncasecmp (&straddress[x],"tt",l) == 0 )
					address->s_eleNumber = 14;
				if (strncasecmp (&straddress[x],"dn",l) == 0 )
					address->s_eleNumber = 15;				
				x = strlen(straddress)-1;
			}
		}
	error=SUCCESS;
	return error;
}

/***********************************************************************/
word bytes2word(byte lowb, byte highb)
{
	word w;
	char c[2];//={lowb,highb};
	c[0]=lowb;
	c[1]=highb;
	memcpy(&w,c,sizeof(w));
	return w;
}	

/***********************************************************************/
int add_word2buffer(TBuffer * buffer, word value) /* return new buffer size */
{
	memcpy(buffer->data+buffer->size, &value,sizeof(value));
	buffer->size += sizeof(value);
	return buffer->size;
}	


/***********************************************************************/
int add_byte2buffer(TBuffer * buffer, byte value) /* return new buffer size */
{
	memcpy(buffer->data+buffer->size, &value,sizeof(value));
	buffer->size += sizeof(value);
	return buffer->size;
}	

/***********************************************************************/
int add_data2buffer(TBuffer * buffer, void * data, byte size) /* return new buffer size */
{
	memcpy(buffer->data+buffer->size, data,size);
	buffer->size += size;
	return buffer->size;
}	

/***********************************************************************/
/* Add DLE if DLE exist in buffer **************************************/
int add_data2bufferWithDLE(TBuffer * buffer, TMsg msg) /* return new buffer size */
{
	byte  i;
	byte databyte[262];
	memcpy(&databyte, &msg,sizeof(msg));
	for (i=0;i<msg.size+6;i++)
	 {
	  if (databyte[i]==DLE)
	   add_byte2buffer(buffer,DLE);
 	  add_byte2buffer(buffer,databyte[i]);
	 }	
	return buffer->size;
}

/***********************************************************************/
int is_timeout(int start_time)
{
	if ((time ((time_t *) 0)-start_time)>TIME_OUT)
		return ERROR_TIMEOUT;
	else 
		return SUCCESS;
}

/***********************************************************************/
void print_symbol(byte c)
{
	switch (c)
	{
		case STX :
			puts("STX\n");
			break;
		case ETX :
			puts("ETX\n");
			break;	
		case ENQ :
			puts("ENQ\n");
			break;
		case ACK :
			puts("ACK\n");
			break;		
		case NAK :
			puts("NAK\n");
			break;		
		case DLE :
			puts("DLE\n");
			break;		
		default : printf("??:%02X\n",c);
	}
}

/***********************************************************************/
/* Cmd:0F Fnc:A2 > Read 3 address fields in SLC500                     */
/* Read Protected Typed Logical                                        */
/***********************************************************************/
int read_A2(TThree_Address_Fields address, void *value, byte size) 
{
	TCmd cmd;
	TMsg sendMsg,rcvMsg;

	int error=ERROR_READ_A2;
	
	bzero(value,size);
	bzero(&sendMsg,sizeof(sendMsg));
	bzero(&rcvMsg,sizeof(rcvMsg));
	bzero(&cmd,sizeof(cmd));
	sendMsg.dst = DEST;
	sendMsg.src = SOURCE;
	sendMsg.cmd = 0x0F;
	sendMsg.sts = 0x00;
	sendMsg.tns = ++tns;
	cmd.fnc = 0xA2;
	cmd.size = address.size;
	cmd.fileNumber = address.fileNumber;
	cmd.fileType = address.fileType;
	cmd.eleNumber = address.eleNumber;
	cmd.s_eleNumber = address.s_eleNumber;
	memcpy(&sendMsg.data,&cmd,sizeof(cmd));
	sendMsg.size = sizeof(cmd);
	if ((error=send_DF1(sendMsg))!=0)
		return error;
	if ((error=rcv_DF1(&rcvMsg))!=0)
		return error;
	if (rcvMsg.tns!=sendMsg.tns)
		return ERROR_TNS_MISMATCH;
	if (rcvMsg.sts!=0)
		return rcvMsg.sts;
	memcpy(value,rcvMsg.data,address.size);
	return SUCCESS;
}

//*********************************************************
int read_boolean(int plctype, char *straddress, int *value)
{
	int error=ERROR_READ_INTEGER;
	TThree_Address_Fields address;
	byte posit;
	int temp=1;
	int tempvalue;
	
	tempvalue=0;//init to 0
	if ((error = calc_address(straddress,&address))!=SUCCESS)
		return error;
	posit=address.s_eleNumber;
	address.s_eleNumber=0;
	if (plctype==SLC)
		error=read_A2(address,&tempvalue,sizeof(tempvalue));
	*value=(temp&(tempvalue>>posit));
	return error;
}

//*********************************************************
int read_float(int plctype, char *straddress, float *value)
{
	int error=ERROR_READ_INTEGER;
	TThree_Address_Fields address;
	
	*value=0;//init to 0
	if ((error = calc_address(straddress,&address))!=SUCCESS)
		return error;
	if (plctype==SLC)
		error=read_A2(address,value,sizeof(*value));
	return error;
}

//*********************************************************
int read_integer(int plctype, char *straddress, word *value)
{
	int error=ERROR_READ_INTEGER;
	TThree_Address_Fields address;
	
	*value=0;//init to 0
	if ((error = calc_address(straddress,&address))!=SUCCESS)
		return error;
	if (plctype==SLC)
		error=read_A2(address,value,sizeof(*value));
	return error;
}

//********************************************************
int read_Tag(int varfile, char *rcvmsg, double *response)
{
	int error=ERROR;
	int fnct;
	int plctype=SLC;
	word value;
        short varshort;
	int varbool;
	float varfloat;
	char Address[20];
	char writevalue[20];
	
	file = varfile;
	fnct=select_fnct(rcvmsg,Address,writevalue);
	switch (fnct)
	{
		case 1 :
			if ((error=read_integer(plctype,Address,&value))==SUCCESS) {
                                varshort = value;
				*response = (double) varshort;
				//sprintf(response,"%d",value);
				//strcat(response,"\n");
			}
			break;
		case 2 :
			if ((error=read_boolean(plctype,Address,&varbool))==SUCCESS) { 
				*response = (double) varbool;
				//sprintf(response,"%i",varbool);
				//strcat(response,"\n");
			}
			break;	
		case 3 :
			if ((error=read_float(plctype,Address,&varfloat))==SUCCESS) { 
				*response = (double) varfloat;
				//sprintf(response,"%f",varfloat);
				//strcat(response,"\n");
			}
			break;			
		default:
			error=fnct;
			break;
	}
	return error;
}

//********************************************************
int write_Tag(int varfile, char *rcvmsg, double *response)
{
	int error=ERROR;
	int fnct;
	int plctype=SLC;
	word value;
	int varbool;
	float varfloat;
	char Address[20];
	char writevalue[20];
	
	file = varfile;
        printf("ici %f\n",*response);
	fnct=select_fnct(rcvmsg,Address,writevalue);
        printf("ici %d:%f\n",fnct,*response);
	switch (fnct)
	{
		case 1 :
			value = (int) *response;
			if ((error=write_integer(plctype,Address,&value))==SUCCESS) { 
				*response = (double) value;
				//printf("ici %d",value);
				//strcat(response,"\n");
			}
			break;
		case 2 :
			varbool = (int) *response;
			if ((error=write_boolean(plctype,Address,&varbool))==SUCCESS) { 
				*response = (double) varbool;
				//sprintf(response,"%d",varbool);
				//strcat(response,"\n");
			}
			break;			
		case 3 :
			varfloat = *response;
			if ((error=write_float(plctype,Address,&varfloat))==SUCCESS) { 
				*response = (double) varfloat;
				//sprintf(response,"%f",varfloat);
				//strcat(response,"\n");
			}
			break;			
		default:
			error=fnct;
			break;
	}
	return error;
}

//return 1:read_integer 2:read_boolean 3:read_float
//return 11:write_integer 12:write_boolean 13:write_float
int select_fnct(char *strquery, char *address, char *value)
{
	int result=ERROR;
	int x;
	char filetype[]= "OISBTCRNF";
	int writequery=FALSE;
	char *varaddress;
	char *varvalue;
	const char delimiters[] = "=\n\r";
	
	//begin by alphaFile ?
	if (strchr(filetype,toupper(strquery[0]))==NULL) {
		return ERROR_BAD_QUERY;
	}
	//find ':' in query ?
	if (strchr(strquery,58)==NULL) {
		return ERROR_BAD_QUERY;
	}	

	//find '=' in query ?	if yes > write value in SLC
	if (strchr(strquery,61)!=NULL) {
		writequery=TRUE;
	}	
	
	varaddress = strsep (&strquery, delimiters);
	//upper ALL
	for (x=0;x<strlen(varaddress);x++) {
		if (isalpha(varaddress[x]))
			varaddress[x]=toupper(varaddress[x]);
	}
	//printf("substring=%i=%s\n",strlen(varaddress),varaddress);
	strcpy(address,varaddress);
	
	if (writequery) {
		varvalue = strsep (&strquery, delimiters);
		//printf("substring=%i=%s\n",strlen(varvalue),varvalue);
		strcpy(value,varvalue);
	}

	switch (toupper(varaddress[0]))
		{
		case 'O':
		case 'I':
		case 'S':
		case 'B':
		case 'T':
		case 'C':
		case 'R':
		case 'N':
			result=1;			
			break;
		case 'F':
			result=3;
			break;
	}
		
	//find '.' in query ?
	if (strchr(varaddress,'.')!=NULL) {
		result=2;
	}
	//find '/' in query ?
	if (strchr(varaddress,'/')!=NULL) {
		result=2;
	}
	//find ACC in query ?
	if (strstr(varaddress,"ACC")!=NULL) {
		result=1;
	}
	//find PRE in query ?
	if (strstr(varaddress,"PRE")!=NULL) {
		result=1;
	}	
	//find LEN in query ?
	if (strstr(varaddress,"LEN")!=NULL) {
		result=1;
	}
	//find POS in query ?
	if (strstr(varaddress,"POS")!=NULL) {
		result=1;
	}	
	//find EN in query ?
	if (strstr(varaddress,"EN")!=NULL) {
		result=2;
	}
	//find TT in query ?
	if (strstr(varaddress,"TT")!=NULL) {
		result=2;
	}
	//find DN in query ?
	if (strstr(varaddress,"DN")!=NULL) {
		result=2;
	}
	if (writequery)
		result +=10;
	return result;
}

/***********************************************************************/
/* Cmd:0F Fnc:AA > write 3 address fields in SLC500                    */
/* Write Protected Typed Logical                                       */
/***********************************************************************/
int write_AA(TThree_Address_Fields address, void *value, byte size) 
{
	TCmd cmd;
	TMsg sendMsg,rcvMsg;
	int error=ERROR_WRITE_AA;
	
	bzero(&sendMsg,sizeof(sendMsg));
	bzero(&rcvMsg,sizeof(rcvMsg));
	bzero(&cmd,sizeof(cmd));
	sendMsg.dst = DEST;
	sendMsg.src = SOURCE;
	sendMsg.cmd = 0x0F;
	sendMsg.sts = 0x00;
	sendMsg.tns = ++tns;
	cmd.fnc = 0xAA;
	cmd.size = address.size;
	cmd.fileNumber = address.fileNumber;
	cmd.fileType = address.fileType;
	cmd.eleNumber = address.eleNumber;
	cmd.s_eleNumber = address.s_eleNumber;
	memcpy(&sendMsg.data,&cmd,sizeof(cmd));
	sendMsg.size = sizeof(cmd);
	memcpy(&sendMsg.data[sendMsg.size],value,cmd.size);
	sendMsg.size += cmd.size; 
	if ((error=send_DF1(sendMsg))!=0)
		return error;
	if ((error=rcv_DF1(&rcvMsg))!=0)
		return error;
	if (rcvMsg.tns!=sendMsg.tns)
		return ERROR_TNS_MISMATCH;
	if (rcvMsg.sts!=0)
		return rcvMsg.sts;
	return SUCCESS;
}

/***********************************************************************/
/* Cmd:0F Fnc:AB > write W/4 fields & mask in SLC500                   */
/* Write 1 bit in SLC                                                  */
/***********************************************************************/
int write_AB(TThree_Address_Fields address, word value, word mask) 
{
	TCmd4 cmd;
	TMsg sendMsg,rcvMsg;
	int error=ERROR_WRITE_AB;
	
	bzero(&sendMsg,sizeof(sendMsg));
	bzero(&rcvMsg,sizeof(rcvMsg));
	bzero(&cmd,sizeof(cmd));
	sendMsg.dst = DEST;
	sendMsg.src = SOURCE;
	sendMsg.cmd = 0x0F;
	sendMsg.sts = 0x00;
	sendMsg.tns = ++tns;
	cmd.fnc = 0xAB;
	cmd.size = address.size;
	cmd.fileNumber = address.fileNumber;
	cmd.fileType = address.fileType;
	cmd.eleNumber = address.eleNumber;
	cmd.s_eleNumber = 0x00;
	cmd.maskbyte = mask;
	cmd.value = value;
	memcpy(&sendMsg.data,&cmd,sizeof(cmd));
	sendMsg.size = sizeof(cmd);
	if ((error=send_DF1(sendMsg))!=0)
		return error;
	if ((error=rcv_DF1(&rcvMsg))!=0)
		return error;
	if (rcvMsg.tns!=sendMsg.tns)
		return ERROR_TNS_MISMATCH;
	if (rcvMsg.sts!=0)
		return rcvMsg.sts;
	return SUCCESS;
}

//*********************************************************
int write_boolean(int plctype, char *straddress, int *value)
{
	int error=ERROR_READ_INTEGER;
	TThree_Address_Fields address;
	byte posit;
	word valuebool;
	word mask;
	
	if ((error = calc_address(straddress,&address))!=SUCCESS)
		return error;
	posit=address.s_eleNumber;
	if (plctype==SLC){
		mask = 0x0001<<posit;
		if (*value)
			valuebool = 0x0001<<posit;
		else
			valuebool = 0x0000;
		error=write_AB(address,valuebool,mask);
	}
	return error;
}

//*********************************************************
int write_float(int plctype, char *straddress, float *value)
{
	int error=ERROR_WRITE;
	TThree_Address_Fields address;
	
	if ((error = calc_address(straddress,&address))!=SUCCESS)
		return error;
	if (plctype==SLC)
		error=write_AA(address,value,sizeof(*value));
	return error;
}

//*********************************************************
int write_integer(int plctype, char *straddress, word *value)
{
	int error=ERROR_WRITE;
	TThree_Address_Fields address;
	
	if ((error = calc_address(straddress,&address))!=SUCCESS)
		return error;
	if (plctype==SLC)
		error=write_AA(address,value,sizeof(*value));
	return error;
}

