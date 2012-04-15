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

int 	black,
		red,
		green,
		blue,
		white,
		magenta,
		cyan,
		yellow,
		orange,
		lightgray,
		midgray,
		darkgray,
		midgreen,
		darkmagenta,
		darkcyan,
		lightlightgray;
int 	BackgroundColor;
int styleDotted[4], styleDashed[6];
	
int color()
{
	red = gdImageColorAllocate(im, 255, 0, 0);
	green = gdImageColorAllocate(im, 0, 255, 0);
	blue = gdImageColorAllocate(im, 0, 0, 255);
	black = gdImageColorAllocate(im, 0, 0, 0);	
	white = gdImageColorAllocate(im, 255, 255, 255);
	magenta = gdImageColorAllocate(im, 255, 0, 255);
	cyan = gdImageColorAllocate(im, 0, 255, 255);
	yellow = 	gdImageColorAllocate(im, 255, 255, 0);
	orange = gdImageColorAllocate(im, 0xFF, 0x80, 0x00);	
	lightgray = gdImageColorAllocate(im, 0xC3, 0xC3, 0xC3);
	midgray = gdImageColorAllocate(im, 0xA0, 0xA0, 0xA0);
	darkgray = gdImageColorAllocate(im, 0x80, 0x80, 0x80);
	midgreen = gdImageColorAllocate(im, 0, 0xC0, 0);
	darkmagenta = gdImageColorAllocate(im, 0x80, 0, 0x80);	
	darkcyan = gdImageColorAllocate(im, 0, 0x80, 0x80);
	lightlightgray = gdImageColorAllocate(im, 0xFA, 0xFA, 0xFA);
	
	BackgroundColor = lightlightgray;
	pen[0].penColor = red;
	pen[1].penColor = blue;
	pen[2].penColor = green;
	pen[3].penColor = magenta;
	pen[4].penColor = cyan;
	pen[5].penColor = orange;
	pen[6].penColor = darkcyan;
	pen[7].penColor = darkmagenta;

	/* Set up dotted style. Leave every other pixel alone. */
	styleDotted[0] = darkgray;
	styleDotted[1] = gdTransparent;
	styleDotted[2] = gdTransparent;
	styleDotted[3] = gdTransparent;	
	/* Set up dashed style. Three on, three off. */
	styleDashed[0] = darkgray;
	styleDashed[1] = darkgray;
	styleDashed[2] = darkgray;
	styleDashed[3] = gdTransparent;
	styleDashed[4] = gdTransparent;
	styleDashed[5] = gdTransparent;

	return SUCCES;
}
