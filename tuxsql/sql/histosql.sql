USE mysql;


create database histosql;

GRANT ALL PRIVILEGES ON *.* TO 'histosql'@'localhost' IDENTIFIED BY 'histosql';

use histosql;


CREATE TABLE `DEFINITION` (
  `ID` int(11) NOT NULL auto_increment,
  `TAGNAME` varchar(30) NOT NULL default '',
  `TAG_DEFINITION` varchar(100) default NULL,
  `ADDRESS` varchar(40) default NULL,
  `DATA_TYPE` varchar(10) default NULL,
  `PLCNAME` varchar(30) default NULL,
  `TIME_SAMPLE` smallint(6) default NULL,
  `TIME_REFRESH` int(6) default NULL,
  `TIME_CLEANING` smallint(6) default '3',
  `HYSTERESIS` smallint(6) default NULL,
  `I_MIN` double default NULL,
  `I_MAX` double default NULL,
  `O_MIN` double default NULL,
  `O_MAX` double default NULL,
  `TAG_UNIT` varchar(20) default NULL,
  `RECORDING` tinyint(4) default '0',
  `READING` tinyint(4) default '0',
  `SNAPSHOT_VALUE` double default NULL,
  `SNAPSHOT_TIME` datetime default NULL,
  `WRITE_VALUE` double default NULL,
  `DIGITAL` tinyint(1) default '0',
  `TAG_SYSTEM` char(3) default NULL,
  PRIMARY KEY  (`ID`),
  UNIQUE KEY `second` (`TAGNAME`)
) TYPE=MyISAM;

--
-- Dumping data for table `DEFINITION`
--

INSERT INTO `DEFINITION` VALUES (1,'MyTag1','This is a real','Real1','0','Lgx1',60,300,3,1,0,100,0,100,'%',1,1,29.408043,'2006-12-27 10:15:46',0,'AB');
INSERT INTO `DEFINITION` VALUES (2,'MyTag2','This is a second real','Real2','0','Lgx1',60,300,3,1,0,100,0,100,'%',1,1,28.163555,'2006-12-27 10:15:48',0,'AB');
INSERT INTO `DEFINITION` VALUES (3,'MyTag3','This is an integer','int1','0','Lgx1',60,300,3,1,0,100,0,100,'%',1,1,12,'2006-12-27 10:15:48',0,'AB');
INSERT INTO `DEFINITION` VALUES (4,'MyTag4','This is a boolean','bool1','0','Lgx1',60,300,3,1,0,100,0,100,'TOR',1,1,12,'2006-12-27 10:15:48',1,'AB');

INSERT INTO `DEFINITION` VALUES (5,'MyTag5','This is a real','Real1','0','Micro2',60,300,3,1,0,100,0,100,'%',1,1,29.408043,'2006-12-27 10:15:46',0,'AB');
INSERT INTO `DEFINITION` VALUES (6,'MyTag6','This is a second real','Real2','0','Micro2',60,300,3,1,0,100,0,100,'%',1,1,28.163555,'2006-12-27 10:15:48',0,'AB');

INSERT INTO `DEFINITION` VALUES (7,'MyTag7','This is a real','f8:0','0','PLC6_0',60,300,3,1,0,100,0,100,'%',1,1,29.408043,'2006-12-27 10:15:46',0,'AB');
INSERT INTO `DEFINITION` VALUES (8,'MyTag8','This is a second real','f8:1','0','PLC6_0',60,300,3,1,0,100,0,100,'%',1,1,28.163555,'2006-12-27 10:15:48',0,'AB');
INSERT INTO `DEFINITION` VALUES (9,'MyTag9','This is an integer','n7:0','0','PLC6_0',60,300,3,1,0,100,0,100,'%',1,1,12,'2006-12-27 10:15:48',0,'AB');

INSERT INTO `DEFINITION` VALUES (10,'MyTag10','This is a real','f8:0','0','PLC6_1',60,300,3,1,0,100,0,100,'%',1,1,29.408043,'2006-12-27 10:15:46',0,'AB');
INSERT INTO `DEFINITION` VALUES (11,'MyTag11','This is a second real','f8:1','0','PLC6_1',60,300,3,1,0,100,0,100,'%',1,1,28.163555,'2006-12-27 10:15:48',0,'AB');
INSERT INTO `DEFINITION` VALUES (12,'MyTag12','This is an integer','n7:0','0','PLC6_1',60,300,3,1,0,100,0,100,'%',1,1,12,'2006-12-27 10:15:48',0,'AB');

INSERT INTO `DEFINITION` VALUES (13,'MyTag13','This is a real','Real1','0','Flex7',60,300,3,1,0,100,0,100,'%',1,1,29.408043,'2006-12-27 10:15:46',0,'AB');
INSERT INTO `DEFINITION` VALUES (14,'MyTag14','This is a second real','Real2','0','Flex7',60,300,3,1,0,100,0,100,'%',1,1,28.163555,'2006-12-27 10:15:48',0,'AB');
INSERT INTO `DEFINITION` VALUES (15,'MyTag15','This is an integer','int1','0','Flex7',60,300,3,1,0,100,0,100,'%',1,1,12,'2006-12-27 10:15:48',0,'AB');
INSERT INTO `DEFINITION` VALUES (16,'MyTag16','This is a boolean','bool1','0','Flex7',60,300,3,1,0,100,0,100,'TOR',1,1,12,'2006-12-27 10:15:48',1,'AB');

INSERT INTO `DEFINITION` VALUES (17,'MyTag17','This is a Modbus real','F401403','','Modbus_plc',20,20,30,0,0,50,0,50,'%',1,1,21.809999,'2006-12-27 10:16:17',0,'MB');
INSERT INTO `DEFINITION` VALUES (18,'MyTag18','This is a Modbus integer','400001','','Modbus_plc',20,20,30,0,0,50,0,50,'%',1,1,13,'2006-12-27 10:16:17',0,'MB');
INSERT INTO `DEFINITION` VALUES (19,'MyTag19','This is a Modbus Double integer','D400003','','Modbus_plc',20,20,30,0,0,50,0,50,'%',1,1,10256,'2006-12-27 10:16:17',0,'MB');

INSERT INTO `DEFINITION` VALUES (20,'Tag20Df1','This is a DF1 integer','S2:42','','slc500_1',20,20,30,0,0,50,0,50,'sec',1,1,21.809999,'2006-12-27 10:16:17',0,'DF1');

-- MySQL dump 9.11
--
-- Host: localhost    Database: histosql
-- ------------------------------------------------------
-- Server version	4.0.24_Debian-10sarge1-log

--
-- Table structure for table `PLC`
--

CREATE TABLE `PLC` (
  `PLCNAME` varchar(30) NOT NULL default '',
  `PLC_PATH` varchar(50) default NULL,
  `PLC_TYPE` varchar(15) default NULL,
  `PLC_NETWORK` varchar(10) default NULL,
  `PLC_NODE` smallint(6) default NULL,
  `PLC_ENABLE` tinyint(4) default '0',
  PRIMARY KEY  (`PLCNAME`)
) TYPE=MyISAM;

--
-- Dumping data for table `PLC`
--

INSERT INTO `PLC` VALUES ('Lgx1','192.168.1.3,1,0','LGX','CNET',0,1);
INSERT INTO `PLC` VALUES ('Micro2','192.168.1.4,1,0','LGX','CNET',0,1);
INSERT INTO `PLC` VALUES ('PLC6_0','192.168.1.5,1,1,2,3','PLC','CNET',0,1);
INSERT INTO `PLC` VALUES ('PLC6_1','192.168.1.5,1,1,2,2,1,3','PLC','DHP_B',4,1);
INSERT INTO `PLC` VALUES ('Flex7','192.168.1.3,1,1,2,5,1,0','LGX','CNET',0,1);

-- MySQL dump 9.11
--
-- Host: localhost    Database: histosql
-- ------------------------------------------------------
-- Server version	4.0.24_Debian-10sarge1-log

--
-- Table structure for table `MODBUS`
--

CREATE TABLE `MODBUS` (
  `PLCNAME` varchar(30) NOT NULL default '',
  `PLC_PATH` varchar(20) NOT NULL default '',
  `PLC_TTY` varchar(20) NOT NULL default '/dev/ttyS0',
  `PLC_SPEED` int NOT NULL default '9600',
  `PLC_PARITY` varchar(5) default 'none',
  `PLC_DATA` tinyint(4) NOT NULL default '8',
  `PLC_STOP` tinyint(4) NOT NULL default '1', 
  `DEVICE_ID` smallint(6) NOT NULL default '1',
  `PLC_ENABLE` tinyint(4) NOT NULL default '0',
  PRIMARY KEY  (`PLCNAME`)
) TYPE=MyISAM;

--
-- Dumping data for table `MODBUS`
--

//INSERT INTO `MODBUS` VALUES ('PM500','192.168.1.10',0,1);
INSERT INTO `MODBUS` VALUES ('PM500','','/dev/ttyS1',19200,0,8,1,1,1);

-- MySQL dump 9.11
--
-- Host: localhost    Database: histosql
-- ------------------------------------------------------
-- Server version	4.0.24_Debian-10sarge1-log

--
-- Table structure for table `HISTO`
--

CREATE TABLE `DF1` (
  `PLCNAME` varchar(30) NOT NULL default '',
  `PLC_TTY` varchar(20) NOT NULL default '/dev/ttyS0',
  `PLC_SPEED` int NOT NULL default '9600',
  `PLC_PARITY` tinyint(4) NOT NULL default '0',
  `PLC_DATA` tinyint(4) NOT NULL default '8',
  `PLC_STOP` tinyint(4) NOT NULL default '1', 
  `PLC_ENABLE` tinyint(4) NOT NULL default '0',
  PRIMARY KEY  (`PLCNAME`)
) TYPE=MyISAM;

INSERT INTO `DF1` VALUES ('slc500_1','/dev/ttyS0',9600,0,8,1,1);
INSERT INTO `DF1` VALUES ('microlx','/dev/ttyS1',38400,0,8,1,1);

CREATE TABLE `HISTO` (
  `ID` int(11) NOT NULL default '0',
  `TIMEVALUE` datetime NOT NULL default '0000-00-00 00:00:00',
  `DATAVALUE` double default NULL,
  PRIMARY KEY  (`ID`,`TIMEVALUE`)
) TYPE=MyISAM;

