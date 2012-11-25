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

public class Label3D extends Panel
{
	int
		largeur = 0,		// Largeur du composant
		hauteur = 10,		// Hauteur du composant
		xt = 1, 	// Coordonn� horizontale du texte
		yt = 10,	// Coordonn� verticale du texte
		lt = 0; 	// Longueur du texte
	Font labelFont = new Font("Dialog", Font.BOLD, 16);
	FontMetrics fontMetrics = getFontMetrics(labelFont);
	String caption;
	boolean enabled = true;
  Color
    bColorB = new Color(0XFF,0XFF, 0XA5),
    fColorD = new Color(0,0,0),
    fColor = new Color(92, 92, 92),
    fColorB = new Color(128, 128, 255);
    
	public Label3D()
	{
		setText("");
	}

	public Label3D(String s)
	{
		setText(s);
	}

	public void setText(String s)
	{
		caption = s;
		lt = fontMetrics.stringWidth(caption);
    hauteur = 4+fontMetrics.getAscent();
		largeur = lt + 3;
    yt = hauteur - 4;
		setSize(largeur, hauteur);
    setBackground(bColorB);
		repaint();
	}
	
	public void setEnabled(boolean b)
	{
		enabled = b;
		repaint();
	}


	public void paint(Graphics g)
	{
		g.setFont(labelFont);
    if (enabled)
		{
			g.setColor(fColorD);
			g.drawString(caption, (xt + 1), (yt + 1));
			g.setColor(fColorB);
			g.drawString(caption, xt, yt);
		}
		else
		{
			g.setColor(fColorB);
			g.drawString(caption, (xt + 1), (yt + 1));
			g.setColor(fColor);
			g.drawString(caption, xt, yt);
		}
	}
}
