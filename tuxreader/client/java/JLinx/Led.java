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
import java.awt.image.*;

public class Led extends Component implements Runnable
{
	static final int[] codes = {
		0xff000000,		// 0 Noir    ff=opaque 000000=RGB
		0xffffffff,		// 1 Blanc
		0x00ffffff,		// 2 Transparent
		0xff800000,		// 3 Rouge fonc�
		0xff008000,		// 4 Vert fonc�
		0xff00ff00,		// 5 Vert clair
		0xffc0c0c0,		// 6 Gris clair
		0xff404040,		// 7 Gris fonc�
		0xff999999,		// 8 Gris moyen
		0xffff0000		// 9 Rouge clair
	};

  static String onString =  "2222222000000222222222222001111110022222222001100000088002222201100555555008802222010555555555508022201055511555555508022010551155555555080201055115555555555060010551555555555550600105555555555555506001051155555555555060010511555555555550600105555555555555506020805555555555550602208055555555555506022208055555555550602222088005555550066022222008800000066002222222200666666002222222222220000002222222";
  static String onBlinkString =  "2222222000000222222222222001111110022222222001100000088002222201100444444008802222010444444444408022201044411444444408022010441144444444080201044114444444444060010441444444444440600104444444444444406001041144444444444060010411444444444440600104444444444444406020804444444444440602208044444444444406022208044444444440602222088004444440066022222008800000066002222222200666666002222222222220000002222222";
  static String offString = "2222222000000222222222222001111110022222222001100000088002222201100999999008802222010999999999908022201099911999999908022010991199999999080201099119999999999060010991999999999990600109999999999999906001091199999999999060010911999999999990600109999999999999906020809999999999990602208099999999999906022208099999999990602222088009999990066022222008800000066002222222200666666002222222222220000002222222";
  static String offBlinkString = "2222222000000222222222222001111110022222222001100000088002222201100333333008802222010333333333308022201033311333333308022010331133333333080201033113333333333060010331333333333330600103333333333333306001031133333333333060010311333333333330600103333333333333306020803333333333330602208033333333333306022208033333333330602222088003333330066022222008800000066002222222200666666002222222222220000002222222";
  static String disableString = "2222222000000222222222222001111110022222222001100000088002222201100777777008802222010777777777708022201077711777777708022010771177777777080201077117777777777060010771777777777770600107777777777777706001071177777777777060010711777777777770600107777777777777706020807777777777770602208077777777777706022208077777777770602222088007777770066022222008800000066002222222200666666002222222222220000002222222";

	static final int[] onPix = new int[400];
	static final int[] onBlinkPix = new int[400];
	static final int[] offPix = new int[400];
	static final int[] offBlinkPix = new int[400];
  static final int[] disablePix = new int[400];

	static
	{
		for (int i = 0; i < 400; i++)
		{
			onPix[i] = codes[(onString.charAt(i)) - 48];
			onBlinkPix[i] = codes[(onBlinkString.charAt(i)) - 48];
			offPix[i] = codes[(offString.charAt(i)) - 48];
			offBlinkPix[i] = codes[(offBlinkString.charAt(i)) - 48];
      disablePix[i] = codes[(disableString.charAt(i)) - 48];
		}
	}

	final Image ledOnImage;
	final Image ledOffImage;
	final Image ledOnBlinkImage;
	final Image ledOffBlinkImage;
  final Image ledDisableImage;

  Thread runner;
  boolean blink;
  boolean blinkLed;
  int blinkTime=500;

	boolean enabled = false;
  boolean ledOn = false;

	public Led()
	{
   ledOnImage = createImage(new MemoryImageSource(20, 20, onPix, 0, 20));
   ledOffImage = createImage(new MemoryImageSource(20, 20, offPix, 0, 20));
   ledOnBlinkImage = createImage(new MemoryImageSource(20, 20, onBlinkPix, 0, 20));
   ledOffBlinkImage = createImage(new MemoryImageSource(20, 20, offBlinkPix, 0, 20));
   ledDisableImage = createImage(new MemoryImageSource(20, 20, disablePix, 0, 20));
   setSize(20,20);
	}

	public void setEnabled(boolean OnOff)
  {
   if (OnOff!=enabled) {
    enabled = OnOff;
    repaint();
   }
  }

	public void setLedOn()
  {
   if (!ledOn) {
    ledOn = true;
    enabled = true;
    repaint();
   }
  }

	public void setLedOff()
  {
   if (ledOn) {
    ledOn = false;
    enabled = true;
    repaint();
   }
  }

	public void setBlinkTime(int time)
  {
    blinkTime = time;
  }

  public int getBlinkTime() {
   return blinkTime;
  }

	public void setBlink(boolean onOff)
  {
   if (onOff) {
    blink = true;
    if (runner == null) {
     runner = new Thread(this);
     runner.start();
    }
   } else {
      blinkLed = false;
      blink = false;
      if (runner != null) {
       runner = null;
       repaint();
      }
     }
  }

  public boolean getBlink() {
   return blink;
  }

	public void paint(Graphics g)
	{
   if (enabled){
		if (ledOn){
      if (blinkLed) {
			 g.drawImage(ledOnBlinkImage, 0, 0, this);
      } else {
	 g.drawImage(ledOnImage, 0, 0, this);
	}
    } else
       if (blinkLed) {
			  g.drawImage(ledOffBlinkImage, 0, 0, this);
       } else {
	  g.drawImage(ledOffImage, 0, 0, this);
	 }
   } else
			g.drawImage(ledDisableImage, 0, 0, this);
	}

	public void run()
	{
		try
		{
			while (true)
			{
				Thread.sleep(blinkTime);
				if (!blink)	break;
				blinkLed = !blinkLed;
	repaint();
			}
		}
		catch(InterruptedException ie)
		{
		}
	}
}


