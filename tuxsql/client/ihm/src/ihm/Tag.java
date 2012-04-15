/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package ihm;

import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

/**
 *
 * @author leicht
 */
public class Tag {
    private ConnecteurSql connecteur;
    private String tagName;
    private Double valueDouble;
    private int valueInt;
    private boolean valueBool;
    private String valueString;

    public Tag(ConnecteurSql connecteur, String tagName) {
        this.connecteur = connecteur;
        this.tagName = tagName;
    }
    /**
     * @return the connecteur
     */
    public ConnecteurSql getConnecteur() {
        return connecteur;
    }

    /**
     * @param connecteur the connecteur to set
     */
    public void setConnecteur(ConnecteurSql connecteur) {
        this.connecteur = connecteur;
    }

    public void read() {
     try {
        //requete sur OBJET
        Statement myQuery = this.connecteur.getConnexion().createStatement();
        ResultSet resultat = myQuery.executeQuery("select * from DEFINITION where TAGNAME='"+getTagName()+"'");
        while (resultat.next())
        {
            this.valueDouble=resultat.getDouble("SNAPSHOT_VALUE");
        }
        resultat.close();
        myQuery.close();
        //this.updateFamille(idObjet);
        //notifyObserver(idObjet);
     } catch (SQLException ex) {
        ex.printStackTrace();
     }
    }

     public void write(double valueD) {
     try {
        //ResultSet resultat = myQuery.executeQuery("select * from DEFINITION where TAGNAME='"+getTagName()+"'");
        String strQuery="update DEFINITION set WRITE_VALUE=? where TAGNAME=?";
        PreparedStatement myQuery = this.connecteur.getConnexion().prepareStatement(strQuery);
        myQuery.setDouble(1, valueD);
        myQuery.setString(2, this.getTagName());
        int resultat = myQuery.executeUpdate();
        myQuery.close();
     } catch (SQLException ex) {
        ex.printStackTrace();
     }
    }
    /**
     * @return the tagName
     */
    public String getTagName() {
        return tagName;
    }

    /**
     * @param tagName the tagName to set
     */
    public void setTagName(String tagName) {
        this.tagName = tagName;
    }

    /**
     * @return the value
     */
    public Double getValueDouble() {
        read();
        return valueDouble;
    }

    /**
     * @param value the value to set
     */
    public void setValueDouble(Double value) {
        this.valueDouble = value;
    }

    /**
     * @return the valueString
     */
    public String getValueString() {
        read();
        valueString = valueDouble.toString();
        return valueString;
    }

    /**
     * @return the valueInt
     */
    public int getValueInt() {
        read();
        this.valueInt=valueDouble.intValue();
        return valueInt;
    }

    /**
     * @param valueInt the valueInt to set
     */
    public void setValueInt(int valueInt) {
        write(valueInt);
        //this.valueInt = valueInt;
    }

    /**
     * @return the valueBool
     */
    public boolean isValueBool() {
        read();
        if (valueDouble>0) {
            valueBool=true;
        } else {
            valueBool=false;
        }
        return valueBool;
    }

    /**
     * @param valueBool the valueBool to set
     */
    public void setValueBool(boolean valueBool) {
        if (valueBool) {
          write(1);
        } else {
            write(0);
        }
        this.valueBool = valueBool;
    }
}
