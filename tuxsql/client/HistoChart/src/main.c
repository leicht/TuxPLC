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

TPen pen[8];
TPen onePen;
int timerange;
int lag;
int height,width;
time_t curtime;
	
int cgiMain() {
	int index;
	char vtag[5];
	cgiFormIntegerBounded("width", &width, 100, 1200, 800);
	cgiFormIntegerBounded("height", &height, 100, 900, 300);
	cgiFormInteger("range", &timerange, 24);
	cgiFormInteger("lag", &lag, 0);	
	getHour();
	// get tag0 to tag7 param
	for (index = 0 ; index < PENNUMBER; index++) {
		sprintf(vtag,"tag%i",index);
		cgiFormString(vtag, pen[index].TagName, 30);
	}
	if (drawChart() != 0)
	{
		MyLog("Draw chart failed\n");
		return (-1);
	}
	return 0;
}
