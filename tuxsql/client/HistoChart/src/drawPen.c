/***************************************************************************
 *  Copyright (C) 2006 http://www.foxinfo.fr                               *
 *  Author : Stephane LEICHT    stephane.leicht@foxinfo.fr                 *
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

#include "HistoChart.h"

int drawPen(int index) 
{
	int res=0;
	int i = 0;
	int x0 = -1;
	int y0 = -1;
	int x, y, digitalValue ;
	double voutput;
	char vmsg[30];
	char strQuery[255];
	MYSQL_ROW row;
	sprintf(strQuery,"Select UNIX_TIMESTAMP(TIMEVALUE),DATAVALUE from HISTO where ID=%i and TIMEVALUE BETWEEN FROM_UNIXTIME(%i)  and FROM_UNIXTIME(%i) ORDER BY TIMEVALUE",pen[index].ID,timeMin,timeMax);
	//sprintf(strQuery,"Select UNIX_TIMESTAMP(TIMEVALUE),DATAVALUE from HISTO where TAGNAME='%s' and TIMEVALUE>FROM_UNIXTIME(%i)  and TIMEVALUE<FROM_UNIXTIME(%i) ",pen[index].TagName,timeMin,timeMax);
	res=mysql_real_query(&Default_Db,strQuery,strlen(strQuery));
	if (res) 
	{
		_GetErrorCode(&Default_Db,MysqlErrorMsg);
		return ERROR;
	}
	SqlResult=mysql_use_result(&Default_Db);
	if (SqlResult==NULL)
	{
		_GetErrorCode(&Default_Db,MysqlErrorMsg);
		return ERROR;
	}
	//sprintf(vmsg,"Nombre : %i=%i",timeMin,timeMax);
	//MyLog(vmsg);	
	while ((row = mysql_fetch_row(SqlResult)))
	{
		unsigned long *lengths;
		lengths = mysql_fetch_lengths(SqlResult);
		if (pen[index].isDigital != 1) {
			voutput=calcScale(atof(row[1]),pen[index].I_MIN,pen[index].I_MAX, 0, hChart-2);
			y = hChart - voutput + YOFFSET -1 ;
			voutput=calcScale(atof(row[0]),timeMin,timeMax, 0, wChart-2);
			x = XOFFSET + abs(voutput) + 1;
			if (x0!=-1)   {
				if ((y0>YOFFSET) && (y>YOFFSET) && (y0<(YOFFSET+hChart)) && (y<(YOFFSET+hChart))) {
					gdImageLine(im,x0,y0,x,y,pen[index].penColor);
				} 
				if ((y0>YOFFSET) && (y0<(YOFFSET+hChart)) && (y<=YOFFSET)) {
					gdImageLine(im,x0,y0,x,YOFFSET+1,pen[index].penColor);
				} 
				if ((y0<=YOFFSET) && (y>YOFFSET) && (y<(YOFFSET+hChart)) ) {
					gdImageLine(im,x0,YOFFSET+1,x,y,pen[index].penColor);
				} 	
				if ((y0>YOFFSET) && (y>=(YOFFSET+hChart)) && (y0<(YOFFSET+hChart)) ) {
					gdImageLine(im,x0,y0,x,YOFFSET+hChart,pen[index].penColor);
				} 	
				if ((y0>=(YOFFSET+hChart)) && (y>YOFFSET) && (y<(YOFFSET+hChart)) ) {
					gdImageLine(im,x0,YOFFSET+hChart,x,y,pen[index].penColor);
				} 
					/*if ((y0>YOFFSET) && (y>YOFFSET) && (y0<(YOFFSET+hChart)) && (y<(YOFFSET+hChart))) {
						gdImageLine(im,x0,y0,x,y0,pen[index].penColor);
						gdImageLine(im,x,y0,x,y,pen[index].penColor);
					} 
	*/
				}				
			x0=x;
			y0=y;
		} else {
			if (atof(row[1]) != 0) {
				voutput=calcScale(pen[index].O_MAX,0,100, 0, hChart-2);
			}	else {
				voutput=calcScale(pen[index].O_MIN,0,100, 0, hChart-2);
			}				
			y = hChart - voutput + YOFFSET -1 ;
			voutput=calcScale(atof(row[0]),timeMin,timeMax, 0, wChart-2);
			x = XOFFSET + abs(voutput) + 1;
			if (x0!=-1)   {
				if ((y0>YOFFSET) && (y>YOFFSET) && (y0<(YOFFSET+hChart)) && (y<(YOFFSET+hChart))) {
						gdImageLine(im,x0,y0,x,y0,pen[index].penColor);
						gdImageLine(im,x,y0,x,y,pen[index].penColor);
				} 
			}				
			x0=x;
			y0=y;			
		}
	}
	mysql_free_result(SqlResult);
	res=_GetErrorCode(&Default_Db,MysqlErrorMsg);	
	return res;
}
