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

public class JLinxsocket {
  Socket sk;
  public DataInputStream fromServer;
  public PrintStream toServer;
  String serverHost;
  int serverPort=17560;

  public JLinxsocket() {

  }

  public JLinxsocket(String sh) {
   serverHost=sh;
  }

  public void setServerHost(String sh) {
   serverHost=sh;
  }

  public String getServerHost() {
   return serverHost;
  }

  public void connect() throws IOException {
        sk = new Socket(serverHost,serverPort);
        fromServer = new DataInputStream(sk.getInputStream());
        toServer = new PrintStream(new DataOutputStream(sk.getOutputStream()));
  }

  public void disconnect() throws IOException {
         sk.close();
  }


}
