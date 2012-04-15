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

#ifndef _CHAINLIST_C
#define _CHAINLIST_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ChainList.h"

/******************** Declarations privées **********/

void _SwapElt(ELEMENT *Elt1,ELEMENT *Elt2);

/******************** LISTE Functions ***************/
LISTE *CreateChListe(void)
{
	LISTE *liste=malloc(sizeof(LISTE));
	InitChListe(liste);
	return(liste);
}
void InitChListe(LISTE *liste)
{
	if (liste!=NULL) memset(liste,0,sizeof(LISTE));
}
int AddChListe(LISTE *liste,void *data)
{
	//Log(LOG_DEBUG,"AddListe %p count : %d\n",data,liste->Count+1);
	if (data!=NULL)
	{
		ELEMENT *New_elt=malloc(sizeof(ELEMENT));
		if (New_elt!=NULL)
		{
			//printf("adding data %p / elt %p / list %p\n",data,New_elt,liste);
			New_elt->Data=data;
			if (liste->Count==0)//(liste->Data==NULL) /* First Elt */
			{
				liste->Data=New_elt;
				New_elt->Next=New_elt;
				New_elt->Prev=New_elt;
				liste->Count=1;
			} else
			{
				ELEMENT *First=liste->Data;
				New_elt->Next=First;
				if (First->Prev!=First) /* 2 or more Elt */
				{
					ELEMENT *Last=(First->Prev);
					New_elt->Prev=Last;
					First->Prev=New_elt;
					Last->Next=New_elt;
				} else /* Only 1 Elt */
				{
					New_elt->Prev=First;
					First->Prev=New_elt;
					First->Next=New_elt;
				}
				liste->Count+=1;
			}
		} else return(-1);
		return(liste->Count);
	} else return(-1);
}
int RemoveChListe(LISTE *liste,void *data)
{
	ELEMENT *elt=GetFirst(liste);
	if (elt==NULL) return(-1);
	do
	{
		if (data==elt->Data)
		{
			int res=RemoveChListe_Ex(liste,elt);
			//printf("RemoveListe data %p / elt %p / list %p\n",data,elt,liste);
			free(elt);
			return(res);
		}
	} while ((elt=GetNext(liste,elt))!=NULL);
	//printf("RemoveListe %p not found in %d\n",data,liste);
	return(-1);
}
int RemoveChListe_Ex(LISTE *liste,ELEMENT *elt)
{
	//Log(LOG_DEBUG,"RemoveListe %p count : %d\n",data,liste->Count);
	if ((elt!=NULL)&&(liste->Data!=NULL))
	{
		if (liste->Data->Prev==liste->Data) // un seul element
		{
			liste->Data=NULL;
			liste->Count=0;
			return(liste->Count);
		} else
		{
			if (elt==liste->Data) liste->Data=elt->Next;
			elt->Prev->Next=elt->Next;
			elt->Next->Prev=elt->Prev;
			elt->Prev=NULL;
			elt->Next=NULL;
			liste->Count-=1;
			return(liste->Count);
		}
		return(-1);
	} else return(-1);
}
ELEMENT *GetFirst(LISTE *liste)
{
	return(liste->Data);
}
ELEMENT *GetLast(LISTE *liste)
{
	if((liste->Data!=NULL)&&(liste->Data->Prev!=NULL)) return(liste->Data->Prev);
		else return(NULL);
}
ELEMENT *GetNext(LISTE *liste,ELEMENT *Current)
{
	if((Current!=NULL)&&(liste->Data!=NULL)&&(Current->Next!=Current)&&(Current->Next!=liste->Data))
		return(Current->Next); else return(NULL);
}
void _SwapElt(ELEMENT *Elt1,ELEMENT *Elt2)
{
	void *temp=Elt2->Data;
	Elt2->Data=Elt1->Data;
	Elt1->Data=temp;
}
void TrieChListe(LISTE *liste,int (*compare)(const void *Data1,const void *Data2))
{
	ELEMENT *elt1=GetFirst(liste);
	while (elt1!=NULL)
	{
		ELEMENT *elt2=GetNext(liste,elt1);
		while (elt2!=NULL)
		{
			/*elt1 > elt2*/
			if(compare(elt1->Data,elt2->Data)>0) _SwapElt(elt1,elt2);
			elt2=GetNext(liste,elt2);
		}
		elt1=GetNext(liste,elt1);
	}
}

#endif /*_CHAINLIST_C*/
