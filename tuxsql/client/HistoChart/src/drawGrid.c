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

int drawGrid() 
{
	int i, j, x, nTick, nMiniTick, index , vLegend, ypos;
	int maxChar;
	time_t vtime;
	struct tm *tTime;
	double voutput;
	double spanMin=0, interval=0 ,vValue, y;
	int TicksNumber;
	char sdate[30];
	char yLabel[20];
	//char vmsg[30];
	switch (timerange) {
		case 24 :
			nTick = 6;
			nMiniTick = 10;
			break;
		case 12 :
			nTick = 4;
			nMiniTick = 10;
			break;
		case 8 :
			nTick = 2;
			nMiniTick = 10;
			break;
		case 4 :
			nTick = 2;
			nMiniTick = 10;
			break;
		case 1 :
			nTick = 6;
			nMiniTick = 10;
			break;
		default : nTick = 6;
	}
	//set Y min and Y max value when multi pen
	if (TagNumber>1) { 	
		onePen.O_MIN = 0.0;
		onePen.O_MAX = 100.0;		
	}
	//calc initial size chart zone for pen
	if (TagNumber==1) { 
		sprintf(yLabel,"%g",onePen.O_MAX);
		Legend_Right = 26 + strlen(yLabel) *gdFontSmall->w;
	} else {
		maxChar = 0;
		for (index = 0 ; index < PENNUMBER; index++) {
			if (strlen(pen[index].TagName)>maxChar)
				maxChar = strlen(pen[index].TagName);
		}		
		Legend_Right = 40 + maxChar *gdFontSmall->w;		
	}		
	hChart = height - YOFFSET - TITLE_BOTTOM;
	wChart = width - XOFFSET - Legend_Right;
	// calc spanMin & interval & TicksNumber
	TicksNumber = GetSpan(onePen.O_MIN, onePen.O_MAX, &interval, &spanMin, 10, 15);	
	//draw tag unit
	if (TagNumber==1) { 
		gdImageStringUp(im, gdFontSmall,	width - 2 - gdFontSmall->h,	height / 2 - (strlen(onePen.UNIT) *gdFontSmall->w / 2),	onePen.UNIT, black);
	}
	// draw legend for multi pen
	vLegend = 0;
	if (TagNumber>1) { 	
		for (index = 0 ; index < PENNUMBER; index++) {
			if (pen[index].TagName[0] != 0) {
				ypos = (vLegend * (1.5* gdFontSmall->h) ) +YOFFSET + 20 ;
				gdImageLine(im,width - Legend_Right + 20 ,ypos,width - Legend_Right+30,ypos, pen[index].penColor); 
				gdImageString(im, gdFontSmall,	width - Legend_Right+35 ,	ypos - gdFontSmall->h / 2,	pen[index].TagName, black);	
				vLegend++;
			}
		}		
	}
	//X Label and tick for begin chart	
	tTime = localtime(&timeMin);
	strftime(sdate, sizeof(sdate), "%H:%M", tTime);
	x = XOFFSET;
	gdImageLine(im,x,height - TITLE_BOTTOM,x,height - TITLE_BOTTOM+5, darkgray); //for label X
	gdImageString(im, gdFontSmall,	x - (strlen(sdate) * gdFontSmall->w / 2),	height - TITLE_BOTTOM+6,	sdate, black);	
	strftime(sdate, sizeof(sdate), "%d/%m", tTime);		
	gdImageString(im, gdFontSmall,	x - (strlen(sdate) * gdFontSmall->w / 2),	height - TITLE_BOTTOM+8+gdFontSmall->h,	sdate, black);
	//X Label and tick for end chart	
	tTime = localtime(&timeMax);
	strftime(sdate, sizeof(sdate), "%H:%M", tTime);
	x = XOFFSET+wChart;
	gdImageLine(im,x,height - TITLE_BOTTOM,x,height - TITLE_BOTTOM+5, darkgray); //for label X
	gdImageString(im, gdFontSmall,	x - (strlen(sdate) * gdFontSmall->w / 2),	height - TITLE_BOTTOM+6,	sdate, black);	
	strftime(sdate, sizeof(sdate), "%d/%m", tTime);		
	gdImageString(im, gdFontSmall,	x - (strlen(sdate) * gdFontSmall->w / 2),	height - TITLE_BOTTOM+8+gdFontSmall->h,	sdate, black);
	// X label between	begin and end 	
	for (i=0; i < timerange; i=i+nTick) {
			if (i>0) {
			vtime = timeMin+(i*3600);
			tTime = localtime(&vtime);
			strftime(sdate, sizeof(sdate), "%H:%M", tTime);
			voutput=calcScale(vtime,timeMin,timeMax, 0, wChart-2);
			x = XOFFSET + abs(voutput) + 1;
			gdImageLine(im,x,height - TITLE_BOTTOM,x,height - TITLE_BOTTOM+5, darkgray); //for label X
			gdImageString(im, gdFontSmall,	x - (strlen(sdate) * gdFontSmall->w / 2),	height - TITLE_BOTTOM+6,	sdate, black);	 			
		}
	}	 
	//vertical line
	for (i=0; i < timerange; i++) {
		vtime = timeMin+(i*3600);
		voutput=calcScale(vtime,timeMin,timeMax, 0, wChart-2);
		x = XOFFSET + abs(voutput) + 1;
		if (i>0) { 
			gdImageSetStyle(im, styleDashed, 6);
			gdImageLine(im,x,YOFFSET,x,height - TITLE_BOTTOM, gdStyled);
		}
		if (timerange<=8) {
			for (j=1; j < 6; j++) {
				vtime = timeMin+(i*3600)+(j*600);
				voutput=calcScale(vtime,timeMin,timeMax, 0, wChart-2);
				x = XOFFSET + abs(voutput) + 1;	
				gdImageSetStyle(im, styleDotted, 4);			
				gdImageLine(im,x,YOFFSET,x,height - TITLE_BOTTOM, gdStyled);			
			}
		}
	}
	//Y Label and tick for begin chart	
	sprintf(yLabel,"%g",onePen.O_MIN);
	//left Y label
	gdImageLine(im,XOFFSET-4,height - TITLE_BOTTOM,XOFFSET,height - TITLE_BOTTOM, black); 
	gdImageString(im, gdFontSmall,	XOFFSET-6 - (strlen(yLabel) * gdFontSmall->w),	height - TITLE_BOTTOM - gdFontSmall->h / 2,	yLabel, black);	
	//right Y label
	gdImageLine(im,width - Legend_Right,height - TITLE_BOTTOM,width - Legend_Right+4,height - TITLE_BOTTOM, black); 
	gdImageString(im, gdFontSmall,	width - Legend_Right+8 ,	height - TITLE_BOTTOM - gdFontSmall->h / 2,	yLabel, black);			
	//Y Label and tick for end chart	
	sprintf(yLabel,"%g",onePen.O_MAX);
	//left Y label
	gdImageLine(im,XOFFSET-4,YOFFSET,XOFFSET,YOFFSET, black); 
	gdImageString(im, gdFontSmall,	XOFFSET-6 - (strlen(yLabel) * gdFontSmall->w),	YOFFSET- gdFontSmall->h / 2,	yLabel, black);	
	//right Y label
	gdImageLine(im,width - Legend_Right,YOFFSET,width - Legend_Right+4,YOFFSET, black); 
	gdImageString(im, gdFontSmall,	width - Legend_Right+8 ,	YOFFSET - gdFontSmall->h / 2,	yLabel, black);			
	// Y label between	begin and end 	and Horizontal line
	for (i=0; i < TicksNumber ; i++) {
		vValue = spanMin+(i*interval);
		if ((vValue>onePen.O_MIN) && (vValue<onePen.O_MAX)) {
			voutput=calcScale(vValue, onePen.O_MIN, onePen.O_MAX, 0, hChart-2);
			y = hChart - abs(voutput) + YOFFSET -1;
			sprintf(yLabel,"%g",vValue);
			gdImageLine(im,XOFFSET-4,y,XOFFSET,y, darkgray); //left
			gdImageString(im, gdFontSmall,	XOFFSET-6 - (strlen(yLabel) * gdFontSmall->w),	y- gdFontSmall->h / 2,	yLabel, black);	
			gdImageLine(im,width - Legend_Right,y,width - Legend_Right+4,y, darkgray); //right
			gdImageString(im, gdFontSmall,	width - Legend_Right+8 ,	y - gdFontSmall->h / 2,	yLabel, black);	
			gdImageSetStyle(im, styleDashed, 6);
			gdImageLine(im,XOFFSET,y,width - Legend_Right,y, gdStyled);
		}
	}
	
	return 0;	
}
