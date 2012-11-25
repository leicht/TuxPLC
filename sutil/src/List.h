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
		
#ifndef _LIST_H
#define _LIST_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef MAX_LISTE_SIZE
	#define MAX_LISTE_SIZE 50
#endif
	
typedef struct _LISTE{
								int Count;
								void *Data[MAX_LISTE_SIZE];
								} LISTE;	
	
int AddListe(LISTE *liste,void *data);
int RemoveListe(LISTE *liste,void *data);
int CompactListe(LISTE *liste);

#ifdef INCLUDE_SRC
	#include "List.c"
#endif

#ifdef __cplusplus
}
#endif

#endif /* _LIST_H */
