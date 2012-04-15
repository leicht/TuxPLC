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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <time.h> 
#include <ctype.h>
#include <syslog.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct termios saved_tty_parameters;			/* old serial port setting (restored on close) */
struct termios Df1_tio;							/* new serial port setting */

#define DLE 0x10
#define STX 0x02
#define ETX 0X03
#define ENQ 0x05
#define ACK 0x06
#define NAK 0x15

#define DEST 0x01
#define SOURCE 0x00

// plctype
#define SLC 1
#define PLC5 2

#define FALSE 0
#define TRUE 1

#define TIME_OUT 1 //in seconds

//Error
#define SUCCESS 0
#define ERROR -1
#define ERROR_NAK -2
#define ERROR_SENDDF1 -3
#define ERROR_RCVDF1 -4
#define ERROR_TIMEOUT -5
#define ERROR_BAD_QUERY -6
#define ERROR_TNS_MISMATCH -9
#define ERROR_READ_A2 -10
#define ERROR_READ_INTEGER -20
#define ERROR_CALC_ADDRESS -30
#define ERROR_WRITE_AA -40
#define ERROR_WRITE -41
#define ERROR_WRITE_AB -42
#define ERROR_COM1 -51
#define ERROR_COM2 -52
#define ERROR_COM3 -53

//Flag
#define DATA_FLAG 1
#define CONTROL_FLAG 2

//Server 
#define BUFSIZE		20
#define PORT		17460
#define MAXCONN		5
#define MAX_CLIENTS	10

/* Define Debug for a console appli, else Daemon*/
#define DEBUG
#ifdef DEBUG
	#define MyLog printf
#else
	#define MyLog(m) syslog(LOG_NOTICE,m) 
#endif
//**********************************************************************

typedef unsigned char byte;	/* create byte type */
typedef unsigned short word;	/* create word type */

/* TMsg structure */
typedef struct {
	byte dst;
	byte src;
	byte cmd;
	byte sts;
	word tns;
	byte data[255];
	byte size;
} TMsg;

/* TBuffer structure */
typedef struct {
	byte data[512];
	byte size;
} TBuffer;

/* TThree_Address_Fields structure*/
typedef struct {
	byte size;
	byte fileNumber;
	byte fileType;
	byte eleNumber;
	byte s_eleNumber;
} TThree_Address_Fields;

/* TCmd */
typedef struct {
	byte fnc;
	byte size;
	byte fileNumber;
	byte fileType;
	byte eleNumber;
	byte s_eleNumber;
} TCmd;

/* TCmd4 */
typedef struct {
	byte fnc;
	byte size;
	byte fileNumber;
	byte fileType;
	byte eleNumber;
	byte s_eleNumber;
	word maskbyte;
	word value;
} TCmd4;

/*************/
/* functions */
/*************/
int Df1_open_device(char [], int , int , int ,int );		/* open device and configure it */
void Df1_close_device(int);									/* close device and restore old parmeters */
char *Df1_GetDf1ErrMsg(int ErrorCode);
word bytes2word(byte lowb, byte highb);
int add_word2buffer(TBuffer * buffer, word value);
int add_byte2buffer(TBuffer *buffer, byte value); 
int add_data2buffer(TBuffer * buffer, void * data, byte size);
int add_data2bufferWithDLE(TBuffer * buffer, TMsg msg);
void print_symbol(byte c);
int is_timeout(int start_time);

word calc_crc (word crc, word buffer) ;
word compute_crc (TBuffer * buffer);
int send_DF1(TMsg Df1Data);  
int rcv_DF1(TMsg * Df1Data); 
int get_symbol(byte * b);
void sendResponse(byte response);
int read_byte(byte * b);


//read
int read_A2(TThree_Address_Fields address, void *value, byte size);
int read_integer(int plctype, char *straddress,word *value);
int read_float(int plctype, char *straddress, float *value);
int read_boolean(int plctype, char *straddress, int *value);
//write
int write_AA(TThree_Address_Fields address, void *value, byte size);
int write_integer(int plctype, char *straddress, word *value);
int write_float(int plctype, char *straddress, float *value);
int write_AB(TThree_Address_Fields address, word value, word mask);
int write_boolean(int plctype, char *straddress, int *value);

int calc_address(char *straddress,TThree_Address_Fields *address);
int select_fnct(char *strquery, char *address, char *value);
int read_Tag(int varfile, char *rcvmsg, double *response);
int write_Tag(int varfile, char *rcvmsg, double *response);
//int server(void);




