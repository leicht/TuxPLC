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

#include "QPlc.h"

QPlc::QPlc()
{
    //
}

void QPlc::init(QString ip,QString strPath, Plc_Type PlcType, int Network, int Node)
{
	ParsePath(strPath);
    m_ip = ip;
	m_PlcType = PlcType;
	m_Network=Network;
	m_Node=Node;
	m_time_out=10000;
	OpenConnection();
}

QString QPlc::ip(void)
{
	return m_ip;
}

void QPlc::setIp(QString vip)
{
	m_ip=vip;
	qDebug("ip:%s",m_ip.toStdString().c_str());
}

QString QPlc::path(void)
{
	return m_path;
}

void QPlc::setPath(QString vpath)
{
	m_path=vpath;
	qDebug("path:%s",m_path.toStdString().c_str());
	ParsePath(m_path);
}

void QPlc::setNetwork(int vnetwork)
{
	m_Network=vnetwork;
}

int QPlc::node(void)
{
	return m_Node;
}

void QPlc::setNode(int vnode)
{
	m_Node=vnode;
}

void QPlc::setPlcType(Plc_Type vPlcType)
{
	m_PlcType=vPlcType;
}

Eip_Session * QPlc::Session(void)
{
	return m_Session;
}

Eip_Connection * QPlc::Connection(void)
{
	return m_Connection;
}

int QPlc::GetSerial(void)
{
	return(getpid());
}

bool QPlc::OpenConnection(void)
{
	bool result=true;
	char ip[16];
	Plc_Type vPlcType;
	vPlcType=(Plc_Type) m_PlcType;
	strcpy(ip,m_ip.toStdString().c_str());
	m_Session=OpenSession(ip);
	if (m_Session!=NULL)
	{
		if (RegisterSession(m_Session)<0)
		{
			CloseSession(m_Session);
			return false;
		}
	} else
	{
		return false;
	}
	m_Connection=ConnectPLCOverCNET(m_Session,vPlcType,(long)m_Session,GetSerial(),m_time_out,m_PlcPath,m_path_size);//(int)m_Session
	if (m_Connection==NULL)
	{
		return false;
	}
    qDebug("Connection OK");
	return result;
}

// write m_PlcPath and m_path_size
bool QPlc::ParsePath(QString strPath)
{
	if (strPath.size()==0) return false;
	QStringList listPath = strPath.split(",");
	bool ok;
	for (int index = 0 ; index < listPath.size() ; index++)
	{
		m_PlcPath[index]=listPath.value(index).toInt(&ok);
		qDebug("PlcPath:%i",listPath.value(index).toInt(&ok));
	}
	m_path_size=listPath.size();
	qDebug("Taille PlcPath:%i",listPath.size());
	return true;
}

