/*
Copyright 2010 Fox informatique http://www.foxinfo.fr
Author: Stephane LEICHT stephane@leicht.fr

This file is part of Metrologix.

Metrologix is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Metrologix is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

package ihm;

/**
 *
 * @author leicht
 */
import java.sql.*;

public class ConnecteurSql {

  private String     driver          = "com.mysql.jdbc.Driver";
  private String     chaineConnexion = "jdbc:mysql://domos/histosql";
  private String     login	     = "histosql";
  private String     password	     = "histosql";
  private Connection connexion;

  public void setDriver (String driver) throws SQLException
  {
    this.driver = driver;
    fermerConnexion ();
  }
  public void setChaineConnexion (String chaineConnexion)
                                             throws SQLException
  {
    this.chaineConnexion = chaineConnexion;
    fermerConnexion ();
  }

  public void setLogin (String login) throws SQLException
  {
    this.login = login;
    fermerConnexion ();
  }

  public void setPassword (String password) throws SQLException
  {
    this.password = password;
    fermerConnexion ();
  }

  public void fermerConnexion () throws SQLException
  {
    if (this.connexion != null && !this.connexion.isClosed())
      this.connexion.close();
  }

  public Connection getConnexion() throws SQLException
  {
    try
    {
      if (this.connexion == null || this.connexion.isClosed())
      {
        Class.forName(this.driver);
        if (login != null)
          this.connexion = DriverManager.getConnection (this.chaineConnexion, this.login, this.password);
        else
          this.connexion = DriverManager.getConnection (this.chaineConnexion);
        //verifierTables (this.connexion);
      }
      return this.connexion;
    }
    catch (ClassNotFoundException ex)
    {
      throw new SQLException(
                   "Classe introuvable " + ex.getMessage ());
    }
  }

 protected void verifierTables (Connection connexion)
                                    throws SQLException
  {
    if (!verifierTable(connexion, "OBJET"))
    {
      /*Statement instruction = connexion.createStatement();
      instruction.executeUpdate("CREATE TABLE UTILISATEUR"
          + " (PSEUDONYME CHAR(30), MOTDEPASSE CHAR(30),"
          + " AUTORISATION CHAR(1))");
      instruction.executeUpdate("CREATE INDEX INDEXPSEUDO"
          + " ON UTILISATEUR (PSEUDONYME)");
      instruction.close ();*/
    }

  }

  protected boolean verifierTable (Connection connexion,
                                   String table)
                                    throws SQLException
  {
    DatabaseMetaData info = connexion.getMetaData ();
    ResultSet resultat = info.getTables (
                    connexion.getCatalog(), null, table, null);
    boolean tableExiste = resultat.next ();
    resultat.close ();
    return tableExiste;
  }
}


