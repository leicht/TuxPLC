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

#ifndef _MYSQL_C
#define _MYSQL_C

#include <mysql/mysql.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "MySQL.h"

#define PROG_GROUP "TUXHISTO"
#define MIN_REQUEST_LEN 500

MYSQL Default_Db;
unsigned int MysqlError;
char MysqlErrorMsg[ERROR_MSG_LEN];
MYSQL_RES *SqlResult=NULL;

int _GetErrorCode(MYSQL *db,char *msg)
{
	MysqlError=mysql_errno(db);
	if (MysqlError!=0)
		strncpy(MysqlErrorMsg,mysql_error(db),ERROR_MSG_LEN-1);
		else strcpy(msg,"Success");
	return(MysqlError);
}
int _OpenDb(MYSQL *db,char *host,char *user,char *password,char *dbname)
{
	mysql_init(db);
	mysql_options(db,MYSQL_READ_DEFAULT_GROUP,PROG_GROUP);
	mysql_real_connect(db,host,user,password,dbname,0,NULL,0);
	return(_GetErrorCode(db,MysqlErrorMsg));
}
void _CloseDb(MYSQL *db)
{
	mysql_close(db);
	db=NULL;
}
int _Select(MYSQL *db,char *sel_str,...)
{	va_list list;
	va_start(list,sel_str);
	char str[255];
	vsprintf(str,sel_str,list);
	mysql_real_query(db,str,strlen(str));
	va_end(list);
	int res=_GetErrorCode(db,MysqlErrorMsg);
	return(res);
}
MYSQL_RES *_Store_result(MYSQL *db)
{
	SqlResult=mysql_store_result(db);
	return(SqlResult);
}
int _Execute(MYSQL *db,char *exec_str,...)
{	va_list list;
	va_start(list,exec_str);
	char str[1000];
	vsprintf(str,exec_str,list);
	mysql_real_query(db,str,strlen(str));
	va_end(list);
	int res=_GetErrorCode(db,MysqlErrorMsg);
	if (!res) return(mysql_affected_rows(db)); else	return(res);
}
int _GetCount(MYSQL *db,char *sel_str)
{
	int res=mysql_real_query(db,sel_str,strlen(sel_str));
	if (res)
	{
		_GetErrorCode(db,MysqlErrorMsg);
		return(-1);
	}
	SqlResult=mysql_store_result(db);
	if (SqlResult==NULL)
	{
		_GetErrorCode(db,MysqlErrorMsg);
		return(-1);
	}
	res=mysql_num_rows(SqlResult);
	mysql_free_result(SqlResult);
	_GetErrorCode(db,MysqlErrorMsg);
	return(res);
}

#endif /*_MYSQL_C*/
