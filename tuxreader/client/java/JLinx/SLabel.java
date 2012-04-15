/***************************************************************************
 *  Copyright (C) 2006 http://www.foxinfo.fr                               *
 *  Author : Stephane LEICHT stephane.leicht@foxinfo.fr                    *
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

public class SLabel extends Panel
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
  Color
    bColor,
    fColor = new Color(0,0,0);

	public SLabel()
	{
		setText("");
	}

	public SLabel(String s)
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
         setBackground(bColor);
	 repaint();
	}


	public void paint(Graphics g)
	{
	g.setFont(labelFont);
	g.setColor(fColor);
	g.drawString(caption, xt, yt);
	}
}
