use lpwan_measurements;

CREATE TABLE LTE_M (
  MeasurementId		INT 			AUTO_INCREMENT,
  Technology 		varchar(15) 	DEFAULT NULL,
  Latitude 			varchar(15) 	DEFAULT NULL,
  Longitude 		varchar(15) 	DEFAULT NULL,
  RSRQ 				int(10) 		DEFAULT NULL,
  RSRP 				int(10) 		DEFAULT NULL,
  RSSI 				int(10) 		DEFAULT NULL,
  Network_Operator 	varchar(20) 	DEFAULT NULL,
  PRIMARY KEY(MeasurementId)
);
CREATE TABLE LTE_M_Clean (
  MeasurementId		INT 			AUTO_INCREMENT,
  Technology 		varchar(15) 	DEFAULT NULL,
  Latitude 			varchar(15) 	DEFAULT NULL,
  Longitude 		varchar(15) 	DEFAULT NULL,
  RSRQ 				int(10) 		DEFAULT NULL,
  RSRP 				int(10) 		DEFAULT NULL,
  RSSI 				int(10) 		DEFAULT NULL,
  Network_Operator 	varchar(20) 	DEFAULT NULL,
  PRIMARY KEY(MeasurementId)
);


CREATE TABLE Lora (
  MeasurementId		INT 			AUTO_INCREMENT,
  Technology varchar(15) DEFAULT NULL,
  Latitude varchar(15) DEFAULT NULL,
  Longitude varchar(15) DEFAULT NULL,
  Margin int(10) DEFAULT NULL,
  NOfGateways int(10) DEFAULT NULL,
  RSSI varchar(10) DEFAULT NULL,
  SNR varchar(10) DEFAULT NULL,
  PRIMARY KEY(MeasurementId)
);
CREATE TABLE Lora_Clean (
  MeasurementId		INT 			AUTO_INCREMENT,
  Technology varchar(15) DEFAULT NULL,
  Latitude varchar(15) DEFAULT NULL,
  Longitude varchar(15) DEFAULT NULL,
  Margin int(10) DEFAULT NULL,
  NOfGateways int(10) DEFAULT NULL,
  RSSI varchar(10) DEFAULT NULL,
  SNR varchar(10) DEFAULT NULL,
  PRIMARY KEY(MeasurementId)
);

CREATE TABLE NB_IOT (
  MeasurementId		INT 			AUTO_INCREMENT,
  Technology varchar(15) DEFAULT NULL,
  Latitude varchar(15) DEFAULT NULL,
  Longitude varchar(15) DEFAULT NULL,
  RSRQ int(10) DEFAULT NULL,
  RSRP int(10) DEFAULT NULL,
  RSSI int(10) DEFAULT NULL,
  Network_Operator varchar(20) DEFAULT NULL,
  PRIMARY KEY(MeasurementId)
);
CREATE TABLE NB_IOT_Clean (
  MeasurementId		INT 			AUTO_INCREMENT,
  Technology varchar(15) DEFAULT NULL,
  Latitude varchar(15) DEFAULT NULL,
  Longitude varchar(15) DEFAULT NULL,
  RSRQ int(10) DEFAULT NULL,
  RSRP int(10) DEFAULT NULL,
  RSSI int(10) DEFAULT NULL,
  Network_Operator varchar(20) DEFAULT NULL,
  PRIMARY KEY(MeasurementId)
);


CREATE TABLE Sigfox_STM (
MeasurementId		INT 			AUTO_INCREMENT,
  Technology varchar(15) DEFAULT NULL,
  Latitude varchar(15) DEFAULT NULL,
  Longitude varchar(15) DEFAULT NULL,
  SessionID varchar(10) DEFAULT NULL,
  SequenceNumber varchar(15) DEFAULT NULL,
  
  PRIMARY KEY(MeasurementId)
);
CREATE TABLE Sigfox_STM_Clean (
MeasurementId		INT 			AUTO_INCREMENT,
  Technology varchar(15) DEFAULT NULL,
  Latitude varchar(15) DEFAULT NULL,
  Longitude varchar(15) DEFAULT NULL,
  SessionID varchar(10) DEFAULT NULL,
  SequenceNumber varchar(15) DEFAULT NULL,
  
  PRIMARY KEY(MeasurementId)
);
