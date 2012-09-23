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
package JLinxWeb;

import java.awt.*;
import java.applet.*;
import java.net.*;
import JLinx.*;

 public class LedTOR extends Applet implements Runnable {
     int ValeurInt1,ValeurInt2;
     int rate=2;
     boolean bascule=false;
     String Myhost;
     Thread runner;
     JLinxtag JLinx1;
     JLinxtag JLinx2;
     Led led1 = new Led();
     String soundFile="beep.au";

     public void init() {
	Myhost = getCodeBase().getHost();
	JLinx1 = new JLinxtag(Myhost);
	JLinx2 = new JLinxtag(Myhost);
         setLayout(null);
         setBackground(Color.darkGray);
         String paramName = getParameter("Bcolor");
         if (paramName != null)
             setBackground(Color.decode(paramName));//0xe1ff00=jaune (RVB)
         if (getParameter("tag1") != null)
             JLinx1.setTag(getParameter("tag1"));
         if (getParameter("tag2") != null)
             JLinx2.setTag(getParameter("tag2"));
         if (getParameter("SoundFile") != null)
             soundFile=(getParameter("SoundFile"));
         if (getParameter("Rate") != null)
             rate=Integer.parseInt(getParameter("Rate"));
         add(led1);
     }

     public void update(Graphics screen) {
         paint(screen);
     }

     void pause(int duration) {
         try {
             Thread.sleep(duration);
         } catch (InterruptedException e) { }
     }

     public void start() {
         if (runner == null) {
             runner = new Thread(this);
             runner.start();
         }
    }

     public void stop() {
         if (runner != null) {
             runner = null;
         }
     }
    /* RUN */
     public void run() {
         Thread thisThread = Thread.currentThread();
         while (runner == thisThread) {
          try {
             ValeurInt1 = JLinx1.getValueInt();
  		ValeurInt2 = JLinx2.getValueInt();
		//showStatus("info : "+JLinx1.getValueStr()+"@"+JLinx2.getValueStr());
             if ((ValeurInt1==0) & (bascule==false)) {
              play(getCodeBase(),soundFile);
             }
             bascule=(ValeurInt1==0);

             if (ValeurInt1==0)
		{
			led1.setLedOff();
			led1.setBlink(false);
		}
	     else
		{
			led1.setLedOn();
			if (ValeurInt2>0)
			 led1.setBlink(true);
			else
 			 led1.setBlink(false);
		}
             if (ValeurInt1==-1) led1.setEnabled(false);
              else led1.setEnabled(true);
             showStatus("info : "+JLinx1.getValueStr()+"@"+JLinx2.getValueStr());
             repaint();
          }
          catch  (Exception e) {
	  	//ValeurInt=ValeurInt;
            showStatus("Erreur : "+e.getMessage()); 
	  }
          finally {
            pause(rate*1000);
          }

         }
     }


 }
