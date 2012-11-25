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

import java.net.*;
import java.io.*;

public class JLinxtag {
  Socket sk;
  public DataInputStream fromServer;
  public PrintStream toServer;
  String serverHost;
  int serverPort=17560;
  String tag;
  //String serveur = "RSLinx JLinx Server";
  //String groupe = "G2000";
  //String rate="2000";
  String mode;
  boolean readMode=true;
  boolean writeMode;
  String valueStr;
  String valeur;

  String requete;
  int valueInt;
  double VarFloat;
  //Constructeur
  public JLinxtag() {
    valueStr="";
    valeur="";
  }

  public JLinxtag(String sh) {
   serverHost=sh;
   valueStr="";
   valeur="";
  }

  // Propriete tag
  public void setTag(String tg) {
   tag=tg;
  }
  public String getTag() {
   return tag;
  }
  // Propriete Read
  public void setRead(boolean R) {
   readMode=R;
  }
  public boolean isRead() {
   return readMode;
  }
  // Propriete Write
  public void setWrite(boolean R) {
   writeMode=R;
  }
  public boolean isWrite() {
   return writeMode;
  }

	// Fin des proprietes

	public String execQuery() throws Exception {
		try {
			sk = new Socket(serverHost,serverPort);
			fromServer = new DataInputStream(sk.getInputStream());
			toServer = new PrintStream(new DataOutputStream(sk.getOutputStream()));
			valueStr = "Erreur : execQuery1";
			mode="";
			if (readMode) { mode="R"; }
			if (writeMode) {mode="W"; }
			if (writeMode & readMode) {mode="RW"; }
			//requete="repere="+tag+"&serveur="+serveur+"&groupe="+groupe+"&mode="+mode+"&rate="+rate+"&value="+valueStr+"!"; //fin=!
			requete="repere="+tag+"!"; //fin=!
			valueStr = "Erreur : execQuery1 sur "+requete;	
			if (tag!=null) {
				toServer.print(requete);
				valueStr=fromServer.readLine();
				if (valueStr.startsWith(tag,0)) {
					valeur=valueStr.substring(valueStr.lastIndexOf("=")+1);
				} else {
					throw new JLinxException(tag+">>"+valueStr);
				}
			} else {
				throw new JLinxException("Query incomplete");
			}
		}
		finally
		{
			sk.close();
			return valeur;
		}
	}

  public int getValueInt() throws Exception {
   valueInt=-1;
   try {
    //VarFloat= (Double.valueOf(this.execQuery())).doubleValue();
    VarFloat = (Double.valueOf(this.execQuery())).doubleValue();
    valueInt = (int) VarFloat;
    //valueInt = 1;
   } catch  (NumberFormatException e) {
      //valueInt=-1;
      throw new JLinxException("Conversion Double vers int "+e.getMessage());
     }
   return valueInt;
  }

  public String getValueStr() throws Exception {
   return valueStr;
  }
  

}
