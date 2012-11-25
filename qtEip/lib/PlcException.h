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

#ifndef PLCEXCEPTION_H
#define PLCEXCEPTION_H
#include <string>
#include <iostream>

//
class PlcException  
{
public:
	PlcException(std::string message);
	PlcException(std::string message, int errcode);
	std::string msg(void);
	void displayMsg(void);
private:
	std::string m_msg;
	int m_errcode;
};
#endif
