/***************************************************************************
 *  Copyright (C) 2006                                                     *
 *  Author : Stephane LEICHT    stephane.leicht@gmail.com                  *
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

gdImagePtr im;
int hChart, wChart;
time_t timeMin, timeMax;
int TagNumber;
int Legend_Right; //Pixels Number for right legend

int drawChart()
{
	int res;
	int index;
	im = gdImageCreate(width, height);
	// Calc time chart
	timeMin = curtime-((timerange * (lag+1))*3600);
	timeMax = curtime-((timerange * lag)*3600);	
	if (color()) {
		MyLog("Draw color chart failed\n");
		return (-1);
	}
	//draw Background
	gdImageFill(im, 32, 32, BackgroundColor);	
	//Open DB
	res = OpenDb("localhost","histosql","histosql","histosql");
	if (res) {
		print_error(MysqlErrorMsg);
		MyLog(MysqlErrorMsg);
		return ERROR;
	}
	//get tag info
	TagNumber = 0;
	for (index = 0 ; index < PENNUMBER; index++) {
		if (GetTag(index)==0)
			TagNumber++;
	}
	if (TagNumber==1) {
		for (index = 0 ; index < PENNUMBER; index++) {
			if (pen[index].TagName[0] != 0)
				onePen = pen[index];
		}			
	}
	//draw grid
	drawGrid();
	//draw chart zone
	gdImageRectangle(im, XOFFSET, YOFFSET, width - Legend_Right, height - TITLE_BOTTOM, black);
	//draw pen
	for (index = 0 ; index < PENNUMBER; index++) {
		if (pen[index].TagName[0] != 0)
			drawPen(index);
	}	
	
	//Close DB
	_CloseDb(&Default_Db);
	/* Now output the image. Note the content type! */
	cgiHeaderContentType("image/png");
	/* Send the image to cgiOut */
	gdImagePng(im, cgiOut);
	/* Free the gd image */
    gdImageDestroy(im);	
	return 0;
}
