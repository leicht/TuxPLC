/***************************************************************************
 *  Copyright (C) 2006                                                     *
 *  Author : Stephane JEANNE    stephane.jeanne@gmail.com                  *
 *           Stephane LEICHT    stephane.leicht@gmail.com                  *
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

#include "QPlcTag.h"
#include <iostream>
#include <stdlib.h>

QPlcTag::QPlcTag()
{

}

void QPlcTag::init(QPlc &vPlc, QString Tagname)
{
    m_Plc = &vPlc;
    m_TagName = Tagname;
	m_Duration = 500;
}

bool QPlcTag::valueBool()
{
	try
	{
		ReadTag();
	}
	catch (PlcException err)
	{
		err.displayMsg();
	}
	return m_valueBool;
}

int QPlcTag::valueInt()
{
	try
	{
		ReadTag();
	}
	catch (PlcException err)
	{
		err.displayMsg();
	}
	return m_valueInt;
}

double QPlcTag::valueDouble()
{
	try
	{
		ReadTag();
	}
	catch (PlcException err)
	{
		err.displayMsg();
	}
	return m_valueDouble;
}

void QPlcTag::setValue(bool value)
{
	try
	{
		writeValue(value);
		m_valueBool = value;
		emit valueChanged(m_valueBool);	
	}
	catch(PlcException err)
	{
		err.displayMsg();
	}
}

void QPlcTag::setValue(int value)
{
	try
	{
		writeValue(value);
		m_valueInt = value;
		emit valueChanged(m_valueInt);	
	}
	catch(PlcException err)
	{
		err.displayMsg();
	}
}

void QPlcTag::setValue(double value)
{
	try
	{
		writeValue(value);
		m_valueDouble = value;
		emit valueChanged(m_valueDouble);
	}
	catch(PlcException err)
	{
		err.displayMsg();
	}
}

void QPlcTag::setOn(void)
{
	try
	{
		writeValue(true);
		m_valueBool = true;
		emit valueChanged(m_valueBool);	
	}
	catch(PlcException err)
	{
		err.displayMsg();
	}
}
//pulse
void QPlcTag::setOnOff(void)
{
	try
	{
		writeValue(true);
		m_valueBool = true;
		emit valueChanged(m_valueBool);
		writeValue(false);
		m_valueBool = false;
		emit valueChanged(m_valueBool);	
	}
	catch(PlcException err)
	{
		err.displayMsg();
	}
}

void QPlcTag::setOff(void)
{
	try
	{
		writeValue(false);
		m_valueBool = false;
		emit valueChanged(m_valueBool);	
	}
	catch(PlcException err)
	{
		err.displayMsg();
	}
}

void QPlcTag::updateValue(void)
{
	try
	{
		ReadTag();
	}
	catch (PlcException err)
	{
		err.displayMsg();
	}
}

QString QPlcTag::tagname(void)
{
	return m_TagName;
}

void QPlcTag::setTagname(QString Tagname)
{
	m_TagName = Tagname;
}

void QPlcTag::setDataType(int vDataType)
{
	m_DataType=vDataType;
}

int QPlcTag::DataType(void)
{
	return m_DataType;
}

void QPlcTag::ReadTag()
{
	DHP_Header dhp={0,0,0,0};
	int tns=getpid();
	double FValue;
	int IValue;
	bool BValue;
	switch (m_Plc->plctype())
	{
		case PLC5:
		case SLC500:
		{ 
			PLC_Read *data;
			dhp.Dest_adress=m_Plc->node();
			if (m_Plc->network()) // DHP
				data=ReadPLCData(m_Plc->Session(),m_Plc->Connection(),&dhp,NULL,0,(Plc_Type) m_Plc->plctype(),tns++,m_TagName.toAscii().data(),1);
			else
				data=ReadPLCData(m_Plc->Session(),m_Plc->Connection(),NULL,NULL,0,(Plc_Type) m_Plc->plctype(),tns++,m_TagName.toAscii().data(),1);
			if (data==NULL) throw PlcException("data vide",cip_errno);
			m_DataType = data->type;
			switch (data->type)
			{
				case PLC_BIT:
				case PLC_INTEGER:
				// TODO case PLC_TIMER:
				{
					IValue=PCCC_GetValueAsBoolean(data,0);
					if (cip_errno) throw PlcException("PCCC_GetValueAsBoolean",cip_errno);
					m_valueInt=IValue;
					emit valueChanged(m_valueInt);
				}break;
				case PLC_FLOATING:
				{
					FValue=PCCC_GetValueAsFloat(data,0);
					if (cip_errno) throw PlcException("PCCC_GetValueAsFloat",cip_errno);
					m_valueDouble=FValue;
					emit valueChanged(m_valueDouble);
				}break;
				default:{
					throw PlcException("Data type unknow");
				}
				break;
			}
			free(data);
		}; break;
		case LGX:
		{
			LGX_Read *data=ReadLgxData(m_Plc->Session(),m_Plc->Connection(),m_TagName.toAscii().data(),1);
			if (data==NULL) throw PlcException("data vide", cip_errno);
			m_DataType = data->type;
			switch(data->type)
			{
				case LGX_BOOL:
					{
						IValue=GetLGXValueAsInteger(data,0);
						if (cip_errno) throw PlcException("GetLGXValueAsInteger",cip_errno);
							if (IValue!=0) BValue=true; else BValue=false;
							m_valueBool=BValue;
							emit valueChanged(m_valueBool);
					}break;
				case LGX_BITARRAY:
					{
						//
					}break;
				case LGX_SINT:
				case LGX_INT:
				case LGX_DINT:
					{
						IValue=GetLGXValueAsInteger(data,0);
						if (cip_errno) throw PlcException("GetLGXValueAsInteger",cip_errno);
						m_valueInt=IValue;
						emit valueChanged(m_valueInt);
					}break;
				case LGX_REAL:
					{
						FValue=GetLGXValueAsFloat(data,0);
						if (cip_errno) throw PlcException("GetLGXValueAsFloat",cip_errno);
						m_valueDouble=FValue;
						emit valueChanged(m_valueDouble);
					}break;
				default:{
					throw PlcException("Data type unknow");
				}break;
			}
			free(data);
		}; break;
		default:{
			throw PlcException("PLC type unknow");
		}
		break;
	}
}

void QPlcTag::ReadDataType(void)
{
	DHP_Header dhp={0,0,0,0};
	int tns=getpid();
	
	switch (m_Plc->plctype())
	{
		case PLC5:
		case SLC500:
		{ 
			PLC_Read *data;
			dhp.Dest_adress=m_Plc->node();
			if (m_Plc->network()) // DHP
				data=ReadPLCData(m_Plc->Session(),m_Plc->Connection(),&dhp,NULL,0,(Plc_Type) m_Plc->plctype(),tns++,m_TagName.toAscii().data(),1);
			else data=ReadPLCData(m_Plc->Session(),m_Plc->Connection(),NULL,NULL,0,(Plc_Type) m_Plc->plctype(),tns++,m_TagName.toAscii().data(),1);
			if (data==NULL) throw PlcException("data vide"+cip_errno);
			m_DataType = data->type;
			free(data);
		}; break;
		case LGX:
		{
			LGX_Read *data=ReadLgxData(m_Plc->Session(),m_Plc->Connection(),m_TagName.toAscii().data(),1);
			if (data==NULL) throw PlcException("data vide"+cip_errno);
			m_DataType = data->type;
			free(data);
		}; break;
		default:{
				throw PlcException("Plc unknow");
		}
		break;
	}
}

void QPlcTag::writeValue(bool value)
{
	DHP_Header dhp={0,0,0,0};
	int tns=getpid();
	//lecture du DataType dans le PLC
	ReadDataType();
	switch (m_Plc->plctype())
	{
		case PLC5:
		case SLC500:
		{ 
			dhp.Dest_adress=m_Plc->node();
            if (m_Plc->network()) { // DHP
				if (WritePLCData(m_Plc->Session(),m_Plc->Connection(),&dhp,NULL,0,(Plc_Type) m_Plc->plctype(),tns++,m_TagName.toAscii().data(),(PLC_Data_Type)m_DataType,&value,1)<0) throw PlcException("writePlcData error",cip_errno);
            } else {
                if (WritePLCData(m_Plc->Session(),m_Plc->Connection(),NULL,NULL,0,(Plc_Type) m_Plc->plctype(),tns++,m_TagName.toAscii().data(),(PLC_Data_Type)m_DataType,&value,1)<0) throw PlcException("writPlcData error",cip_errno);
            }
		}; break;
		case LGX:
		{
			if (WriteLgxData(m_Plc->Session(),m_Plc->Connection(),m_TagName.toAscii().data(),(LGX_Data_Type)m_DataType,&value,1)<0) throw PlcException("writeLgxData error",cip_errno);
		}; break;
		default:
		{
			throw PlcException("Plc unknow",cip_errno);
		}
	}	
}
void QPlcTag::writeValue(int value)
{
	DHP_Header dhp={0,0,0,0};
	int tns=getpid();
	//lecture du DataType dans le PLC
	ReadDataType();
	switch (m_Plc->plctype())
	{
		case PLC5:
		case SLC500:
		{ 
			dhp.Dest_adress=m_Plc->node();
            if (m_Plc->network()) { // DHP
				if (WritePLCData(m_Plc->Session(),m_Plc->Connection(),&dhp,NULL,0,(Plc_Type) m_Plc->plctype(),tns++,m_TagName.toAscii().data(),(PLC_Data_Type)m_DataType,&value,1)<0) throw PlcException("writePlcData error",cip_errno);
            } else {
                if (WritePLCData(m_Plc->Session(),m_Plc->Connection(),NULL,NULL,0,(Plc_Type) m_Plc->plctype(),tns++,m_TagName.toAscii().data(),(PLC_Data_Type)m_DataType,&value,1)<0) throw PlcException("writPlcData error",cip_errno);
            }
		}; break;
		case LGX:
		{
			if (WriteLgxData(m_Plc->Session(),m_Plc->Connection(),m_TagName.toAscii().data(),(LGX_Data_Type)m_DataType,&value,1)<0) throw PlcException("writeLgxData error",cip_errno);
		}; break;
		default:
		{
			throw PlcException("Plc unknow",cip_errno);
		}
	}	
}

void QPlcTag::writeValue(double value)
{
	float fValue=value;
	DHP_Header dhp={0,0,0,0};
	int tns=getpid();
	//lecture du DataType dans le PLC
	ReadDataType();
	switch (m_Plc->plctype())
	{
		case PLC5:
		case SLC500:
		{ 
			dhp.Dest_adress=m_Plc->node();
            if (m_Plc->network()) {// DHP
				if (WritePLCData(m_Plc->Session(),m_Plc->Connection(),&dhp,NULL,0,(Plc_Type) m_Plc->plctype(),tns++,m_TagName.toAscii().data(),(PLC_Data_Type)m_DataType,&fValue,1)<0) throw PlcException("writePlcData error",cip_errno);
            } else {
                if (WritePLCData(m_Plc->Session(),m_Plc->Connection(),NULL,NULL,0,(Plc_Type) m_Plc->plctype(),tns++,m_TagName.toAscii().data(),(PLC_Data_Type)m_DataType,&fValue,1)<0) throw PlcException("writPlcData error",cip_errno);
            }
		}; break;
		case LGX:
		{
			if (WriteLgxData(m_Plc->Session(),m_Plc->Connection(),m_TagName.toAscii().data(),(LGX_Data_Type)m_DataType,&fValue,1)<0) throw PlcException("writeLgxData error",cip_errno);
		}; break;
		default:
		{
			throw PlcException("Plc unknow",cip_errno);
		}
	}	
}

int QPlcTag::duration(void)
{
	return m_Duration;
}

void QPlcTag::setDuration(int Duration)
{
	m_Duration = Duration ;
	//reste à gérer qd timer tourne déjà
}

void QPlcTag::setScan(bool value)
{
	if (value) 
	{
        m_TimerID = this->startTimer(m_Duration);
        qDebug("m_Timer %i:%i",m_TimerID,m_Duration);
	} else {
		killTimer(m_TimerID);
	}
}

void QPlcTag::timerEvent(QTimerEvent *event)
{
	if (event->timerId()==m_TimerID) 
	{
        updateValue();
	}
}

