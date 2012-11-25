/***************************************************************************
 *  Copyright (C) 2006                                                     *
 *  Author : Stephane LEICHT stephane.leicht@gmail.com                     *
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
package JLinx;

import java.awt.*;

public class Level extends Component
{
 int
  lBarre = 0,		// Longueur de la barre de progression
  hBarre = 0,		// Hauteur de la barre de progression
  hBarreMax = 0,	// Longueur maximale de la barre de progression
  currentValue=0,	// Valeur courante de la donn� �repr�enter
  maxValue = 100;	// Valeur maximale de la donn� �repr�enter
  Color progressColor = Color.yellow;
  Color	lineColor = Color.black;

  public Level ()
	{
	 	this.setSize(100, 100);
		this.setDimensions();
                this.repaint();
	}

  void setDimensions()
	{
		hBarreMax = this.getSize().height-2 ;
		lBarre = this.getSize().width-2;
		hBarre = currentValue * hBarreMax / maxValue;
		if (hBarre > hBarreMax)
			hBarre = hBarreMax;
		if (hBarre < 0)
			hBarre = 0;
	}

  public void setCurrentValue(int x)
	{
		if (currentValue!=x) {currentValue = x;
		                      repaint();
                                     }
	}

  public int getCurrentValue()
  {
   return currentValue;
  }

	public void setMaxValue(int x)
	{
		maxValue = x;
		repaint();
	}

  public int getMaxValue()
  {
   return maxValue;
  }

  public void setProgressColor(Color c)
  {
   progressColor=c;
  }

  public Color getProgressColor()
  {
   return progressColor;
  }

  public void paint(Graphics g)
  {
		setDimensions();
		g.setColor(lineColor);
                g.drawLine(0,this.getSize().height-1,this.getSize().width-1,this.getSize().height-1);
                g.drawLine(0,0,0,this.getSize().height-1);
                g.drawLine(this.getSize().width-1,this.getSize().height-1,this.getSize().width-1,0);
		g.setColor(progressColor);
		g.fillRect(1,this.getSize().height-hBarre-1, lBarre, hBarre);
  }
}
