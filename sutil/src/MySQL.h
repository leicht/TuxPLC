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

#ifndef _MYSQL_H
#define _MYSQL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <mysql/mysql.h>

#define ERROR_MSG_LEN 512
	
extern MYSQL Default_Db;
extern unsigned int MysqlError;
extern char MysqlErrorMsg[ERROR_MSG_LEN];
extern MYSQL_RES *SqlResult;

	
int _GetErrorCode(MYSQL *db,char *msg);
#define GetErrorCode _GetErrorCode(&Default_Db,MysqlErrorMsg)
	
int _OpenDb(MYSQL *db,char *host,char *user,char *password,char *dbname);
#define OpenDb(host,user,password,dbname) _OpenDb(&Default_Db,host,user,password,dbname)
	
void _CloseDb(MYSQL *db);
#define CloseDb _CloseDb(&Default_Db)

int _Select(MYSQL *db,char *sel_str,...);
#define Select(sel_str) _Select(&Default_Db,sel_str)

MYSQL_RES *_Store_result(MYSQL *db);
#define Store_result _Store_result(&Default_Db)

int _Execute(MYSQL *db,char *exec_str,...);
#define Execute() _Execute(&Default_Db)

int _GetCount(MYSQL *db,char *sel_str);
#define GetCount(sel_str) _GetCount(&Default_Db,sel_str)

#ifdef INCLUDE_SRC
	#include "MySQL.c"
#endif

#ifdef __cplusplus
}
#endif

#endif /* _MYSQL_H */
