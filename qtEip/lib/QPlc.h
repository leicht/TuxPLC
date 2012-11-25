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

#ifndef QPLC_H
#define QPLC_H
#include <QObject>
#include <tuxeip/TuxEip.h>
#include <string>
#include <QStringList>

class QPlc : public QObject
{
	Q_OBJECT
public:
    QPlc();
    void init(QString ip,QString strPath, Plc_Type PlcType, int Network, int Node);
    //enum NetworkTypes {CNET,DHP_A,DHP_B};
    //enum PlcTypes {PLC5=1,SLC500,LGX};
	QString ip(void);
	void setIp(QString vip);
	QString path(void);
	void setPath(QString vpath);
    int network(void) { return m_Network; };
    void setNetwork(int vnetwork);
	int node(void);
	void setNode(int vnode);
    Plc_Type plctype(void) { return m_PlcType; };
    void setPlcType(Plc_Type plctype);
	Eip_Session * Session(void);
	Eip_Connection * Connection(void);
	bool OpenConnection(void);

signals:

public slots:

protected:
	//

private:
	QString m_ip;
	QString m_path;
	BYTE m_PlcPath[100];
	unsigned char m_path_size;
    Plc_Type m_PlcType;
    int m_Network;
	int m_Node;
	Eip_Session *m_Session;
	Eip_Connection *m_Connection;
	int m_time_out;
	int GetSerial(void);
	bool ParsePath(QString strPath);
};

#endif // QPLC_H
