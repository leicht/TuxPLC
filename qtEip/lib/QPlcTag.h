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

#ifndef QPLCTAG_H
#define QPLCTAG_H

#include <QObject>
#include <QTimerEvent>
#include <tuxeip/TuxEip.h>
#include "QPlc.h"
#include "PlcException.h"

class QPlcTag : public QObject
{
	Q_OBJECT
public:
    QPlcTag();
    void init(QPlc &vPlc, QString Tagname);
	void setTagname(QString Tagname);
	QString tagname(void);
	int DataType(void);
	void setDataType(int vDataType);
	int duration(void);
	void setDuration(int Duration);
	void setScan(bool value);

signals:
	void valueChanged(bool newValue);
	void valueChanged(int newValue);
	void valueChanged(double newValue);

public slots:
	bool valueBool(void);
	int valueInt(void);
	double valueDouble(void);
	void setValue(bool value);
	void setValue(int value);
	void setValue(double value);
	void setOn(void);
	void setOff(void);
	void setOnOff(void);
	void updateValue(void);

protected:
	void timerEvent(QTimerEvent *event);

private:
	QString m_TagName;
	bool m_valueBool;
	int m_valueInt;
	double m_valueDouble;
	int m_DataType;
	QPlc *m_Plc;
	int m_TimerID;
	int m_Duration; // time in mS for one scan
	bool m_Scan; //True = scan cycle
	void ReadTag(void);
	int WriteTag(void);
	void ReadDataType(void);
	void writeValue(bool value);
	void writeValue(int value);
	void writeValue(double value);
};

#endif // QPLCTAG_H
