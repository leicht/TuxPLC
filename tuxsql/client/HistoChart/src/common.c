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

double calcScale(double inputValue, double I_MIN, double I_MAX, double O_MIN, double O_MAX) {
	double rangeInput = I_MAX-I_MIN;
	double rangeOutput = O_MAX-O_MIN;
	return inputValue*(rangeOutput/rangeInput)+O_MAX-(I_MAX*(rangeOutput/rangeInput));
}

int GetSpan(double Vmin,double Vmax,double *Interval,double *Origine,int MinGrad,int MaxGrad)
{
	double Tab[]={1,2,4,5,10};
	int TabSize = sizeof(Tab)/sizeof(double);
	double span,result=0,acc1=0,acc2=0;
	int facteur,i,j;
	*Interval=0; 
  
  if ((Vmax-Vmin)<=0) //return(0);
	{ 
		double Vmin_temp=Vmin;
		Vmin=Vmax;
		Vmax=Vmin_temp;
	}	
  span=Vmax-Vmin;
	*Origine=Vmin;
  
  if (span<MinGrad) facteur=-ceil(log10(MinGrad/span));
		else facteur=ceil(log10(span/MinGrad))-1;
  for(i=MinGrad;i<=MaxGrad;i++)
   for(j=0;j<TabSize;j++)
    {
      acc1=(i*Tab[j])/(span*pow(10,-facteur));
      if ((acc1>=1)&&((acc1<acc2)||(acc2==0)))
       {
         acc2=acc1;
         result=i;
         *Interval=Tab[j]*pow(10,facteur);
       }
    }
 *Origine=floor(Vmin/(*Interval))*(*Interval);
 if ((*Origine+(*Interval)*result)<Vmax) result+=1;
 return(result);
}

void getHour() {
	//char vmsg[60];
	int diff;
	time(&curtime);
	diff = curtime % 3600;
	//sprintf(vmsg,"diff : %i *** cur : %i *** %s",diff,curtime,ctime(&curtime));
	//MyLog(vmsg);
	curtime = curtime - diff + 3600;
}

void print_error(char *errormsg) 
{
	gdImageString(im, gdFontSmall,	im->sx / 2 - (strlen(errormsg) * gdFontSmall->w / 2),	im->sy / 2 - gdFontSmall->h / 2,	errormsg, black);
}

int GetTag(int index)
{	
	int res;
	char strQuery[255];
	MYSQL_ROW row;
	sprintf(strQuery,"select TAGNAME,I_MIN,I_MAX,O_MIN,O_MAX,TAG_UNIT,ID,DIGITAL from DEFINITION where TAGNAME='%s'",pen[index].TagName);	
	res=mysql_real_query(&Default_Db,strQuery,strlen(strQuery));
	if (res) 
	{
		_GetErrorCode(&Default_Db,MysqlErrorMsg);
		return ERROR;
	}
	SqlResult=mysql_store_result(&Default_Db);
	if (SqlResult==NULL)
	{
		_GetErrorCode(&Default_Db,MysqlErrorMsg);
		return ERROR;
	}
	// Empty result
	if (res=mysql_num_rows(SqlResult)==0) {
		bzero(&pen[index],sizeof(TPen));
		sprintf(MysqlErrorMsg,"Empty Result");
		return	EMPTYRESULT;
	}
	while ((row = mysql_fetch_row(SqlResult)))
	{
		unsigned long *lengths;
		lengths = mysql_fetch_lengths(SqlResult);
		if (row[1]!=NULL)	pen[index].I_MIN=atof(row[1]);	
		if (row[2]!=NULL)	pen[index].I_MAX=atof(row[2]);	
		if (row[3]!=NULL)	pen[index].O_MIN=atof(row[3]);	
		if (row[4]!=NULL)	pen[index].O_MAX=atof(row[4]);
		snprintf(pen[index].UNIT,lengths[5]+1,"%s",row[5]);		
		if (row[6]!=NULL)	pen[index].ID=atoi(row[6]);
		if (row[7]!=NULL)	pen[index].isDigital=atoi(row[7]);			
	}
	mysql_free_result(SqlResult);
	res=_GetErrorCode(&Default_Db,MysqlErrorMsg);
	return res;
}
