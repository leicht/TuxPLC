/***************************************************************************
 *  Copyright (C) 2006 http://www.foxinfo.fr                               *
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

#ifndef _LIST_C
#define _LIST_C

#include <stdio.h>
#include "List.h"

/******************** LISTE Functions ***************/
int AddListe(LISTE *liste,void *data)
{
	//Log(LOG_DEBUG,"AddListe %p count : %d\n",data,liste->Count+1);
	if (liste->Count>=MAX_LISTE_SIZE) return(-1);
	liste->Data[liste->Count]=data;
	return(liste->Count++);
}
int RemoveListe(LISTE *liste,void *data)
{ int i;
	//Log(LOG_DEBUG,"RemoveListe %p count : %d\n",data,liste->Count);
	for (i=0;i<liste->Count;i++)
		if (liste->Data[i]==data) liste->Data[i]=NULL;
	return(CompactListe(liste));
}
int CompactListe(LISTE *liste)
{ int i,j;
	for (i=0;i<MAX_LISTE_SIZE;i++)
	{
		if (liste->Data[i]==NULL)
			for (j=i+1;j<MAX_LISTE_SIZE;j++)
				liste->Data[j-1]=liste->Data[j];
	}
	j=0;
	for (i=0;i<MAX_LISTE_SIZE;i++)
		if (liste->Data[i]!=NULL) j++;
	liste->Count=j;
	//Log(LOG_DEBUG,"CompactListe %d\n",liste->Count);
	return(j);
}
#endif /*_LIST_C*/
