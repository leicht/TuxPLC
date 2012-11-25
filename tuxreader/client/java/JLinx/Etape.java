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

public class Etape extends Panel
{
	int
		largeur = 21,		// Largeur du composant
		hauteur = 21,		// Hauteur du composant
		xt = 1, 	// Coordonn� horizontale du texte
		yt = 10,	// Coordonn� verticale du texte
		lt = 0; 	// Longueur du texte
	Font labelFont = new Font("Dialog", Font.BOLD, 12);
	FontMetrics fontMetrics = getFontMetrics(labelFont);
	String caption;
  Color
    bColor= new Color(0XFF,0XFF, 0XA5),
    fColor ;

	public Etape()
	{
		setText("");
	}

	public Etape(String s)
	{
		setText(s);
	}

	public void setText(String s)
	{
	 caption = s;
	 lt = fontMetrics.stringWidth(caption);
         xt = ((largeur-lt)/2);
	 yt = ((hauteur - fontMetrics.getHeight())/2)-2+fontMetrics.getHeight();
	 setSize(largeur, hauteur);
	 setBackground(bColor);
	 repaint();
	}


	public void paint(Graphics g)
	{
	g.setFont(labelFont);
	g.setColor(fColor);
        g.drawRect(0,0,largeur-1,hauteur-1);
	g.drawString(caption, xt, yt);
	}
}
