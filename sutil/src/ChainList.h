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

#ifndef _CHAINLIST_H
#define _CHAINLIST_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _ELEMENT ELEMENT;
struct _ELEMENT	
	{
		ELEMENT *Prev;
		ELEMENT *Next;
		void *Data;
	};
	
typedef struct _LISTE{
								int Count;
								ELEMENT *Data;
								} LISTE;	

LISTE *CreateChListe(void);
void InitChListe(LISTE *liste);
int AddChListe(LISTE *liste,void *data);
int RemoveChListe(LISTE *liste,void *data);
int RemoveChListe_Ex(LISTE *liste,ELEMENT *elt);
void TrieChListe(LISTE *liste,int (*compare)(const void *Data1,const void *Data2));

ELEMENT *GetFirst(LISTE *liste);
ELEMENT *GetLast(LISTE *liste);
ELEMENT *GetNext(LISTE *liste,ELEMENT *Current);

//int CompactListe(LISTE *liste);
#ifdef INCLUDE_SRC
	#include "ChainList.c"
#endif

#ifdef __cplusplus
}
#endif

#endif /* _CHAINLIST_H */
