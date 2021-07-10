#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>        // Include the Wi-Fi library
#include <ESP8266HTTPClient.h>


#define MSGLENGTH 256
#define FILENAMELENGTH 12
#define LED D0  

#define LTE 5
#define LORA 6
#define SIGFOX 3

const char* ssid     = "";      
const char* password = "";
const char* url = "http://europe-west1-precise-ego-316916.cloudfunctions.net/insertData"; //if not working change to http://


char loraFile[] = "/lora.csv";
char sigfoxFile[] = "/sigfox.csv";
char ltemFile[] = "/ltem.csv";
char nbiotFile[] = "/nbiot.csv";

char loraInit[] = "lora, lat, lon, margin, gwcnt, rssi, snr";
char sigfoxInit[] = "sigfox, lat, lon, sessionId, seqN";
char ltemInit[] = "ltem, lat, lon, rsrq, rsrp, sigPower";
char nbiotInit[] = "nbiot, lat, lon, rsrq, rsrp, sigPower";

//char curFile[FILENAMELENGTH] = {};
File myFile;
File backupFile;

char message[MSGLENGTH] = {};
char tmp;
int itr = 0;
bool readingData = 0;
char tech[FILENAMELENGTH] = {};
char techFile[FILENAMELENGTH] = {};
int state = 0;

SoftwareSerial serial2(4, 5); //RX = D2, TX = D1


void setup() {
  pinMode(LED, OUTPUT); 
  digitalWrite(LED, HIGH);
  serial2.begin(57600);
  SD.begin(15);
  Serial.begin(9600);
  
  Serial.println("connecting");
  int conn = connectWifi();  

  if (conn == 1){
    Serial.println("Connected");
    //Serial.println("creating HTTP connection");
    //beginHttp();
    readFileData(loraFile, LORA);
    readFileData(ltemFile, LTE);
    readFileData(sigfoxFile, SIGFOX);
    readFileData(nbiotFile, LTE);
  }

}

void determineTech(){
  int j = 0;
  while(message[j] != ',' && j < 100){
    tech[j] = message[j];
    j++;
  }
  if(strcmp(tech, "lora") == 0) {
    memcpy(techFile, loraFile, FILENAMELENGTH);
  } else if(strcmp(tech, "sigfox") == 0){
    memcpy(techFile, sigfoxFile, FILENAMELENGTH);
  } else if(strcmp(tech, "lte-m") == 0){
    memcpy(techFile, ltemFile, FILENAMELENGTH);
  } else if(strcmp(tech, "nb-iot") == 0){
    memcpy(techFile, nbiotFile, FILENAMELENGTH);
  }
  memset(tech, '\0', FILENAMELENGTH);
  
} 

void loop() {

  if(serial2.available()){
      tmp = serial2.read();   // read one byte from serial buffer and save to data_rcvd
      if(tmp == '<')
      {
        readingData = 1;
      }
      else if (tmp == '>')
      {
        if(itr < 3) { //if no data in message
          memset(message, '\0', sizeof message);
          itr = 0;
          readingData = 0;
        } 
        else {
          determineTech();
          myFile = SD.open(techFile, FILE_WRITE);
          
          if(myFile){
            digitalWrite(LED, (state) ? HIGH : LOW);
            state = !state;
            myFile.println(message);
            myFile.close();
          }
          
          memset(techFile, '\0', FILENAMELENGTH);
          memset(message, '\0', sizeof message);
          itr = 0;
          readingData = 0;
          delay(50);
        } 
      } 
      else if (readingData == 1)
      {
        message[itr] = tmp;
        itr++;
      }
    }

    
}

void readFileData(char * fileName, int lpwanTech){
  Serial.println("HEJ");
  myFile = SD.open(fileName,FILE_READ);
  char tmpChar = 'Q';
  String isDataSent;
  int i = 0;
  int lineCount = 0;
  char lineOfData[100];
  char dataToPOST[1024] = {'\0'};
  
  if(myFile == 0){
    Serial.println("GG");
    return;
  }

  while(myFile.available()){
    while(tmpChar != '\n' && i < 100 && myFile.available()){
      tmpChar = myFile.read();
      lineOfData[i] = tmpChar;
      i++;
    }
    
    if( verifyMeasurement(lineOfData, strlen(lineOfData),lpwanTech) == 1){
      strcat(dataToPOST,lineOfData);
    }

    memset(lineOfData,'\0',100);
    i = 0;
    tmpChar = '\0';
    //Serial.println(lineCount);
    if (lineCount == 20){
      Serial.println("DATA:");
      Serial.println(dataToPOST);
      isDataSent = beginHttp(dataToPOST);
      memset(dataToPOST,'\0',1024);
      lineCount = 0;
    }    
    lineCount++;
  }
  myFile.close();
  Serial.println("REMAINING DATA:");
  Serial.println(dataToPOST);
  
  isDataSent = beginHttp(dataToPOST);
       
  memset(dataToPOST,'\0',1024);
  
  
}



int verifyMeasurement(char * msg, int msgLength, int lpwanTech){
  int commaCount = 0;
  if(msgLength >= 45){
    return 0;  
  }
  for (int i = 0; i < msgLength; i++){
    if(msg[i] == ','){
      commaCount++;
      if(i < msgLength && (msg[i+1] == ',' || msg[i+1] == '\0')){
        return 0;
      }
    }
  }
  if (commaCount == lpwanTech){
    return 1;
  }
  return 0;
}



int connectWifi(){
  WiFi.begin(ssid, password);
  int i = 0;
  int result;
  while (WiFi.status() != WL_CONNECTED && i < 25) { 
    delay(1000);
    Serial.print('.');  
    i++;
  }

  if(WiFi.status() == WL_CONNECTED){
    Serial.println("connected!");
    result = 1;
  } else {
    result = 0;
  }
  return result;
}

String beginHttp(char * msg){
  HTTPClient http;
  
  http.begin(url);
  http.addHeader("Content-Type", "text/plain"); 
  
  int httpCode = http.POST(msg); 
  String payload = http.getString();  
  
  Serial.print("status code: ");
  Serial.println(httpCode);
  Serial.println(payload);
  
  http.end();
  return payload;
}
