  // ** Passthrough Defines & Includes: **
  #include <Arduino.h>
  #include <string.h>
  
  #define R4XX // Uncomment when you use the ublox R4XX module
  //#define ARDUINO_SODAQ_SFF
  
  #if defined(ARDUINO_SODAQ_SFF)
  // SODAQ SARA SFF 
  #define MODEM_STREAM Serial1
  #define powerPin SARA_ENABLE
  #define voltagePin SARA_R4XX_TOGGLE
  #endif
  
  // ** GPS Defines & Includes: **
  #define WIRE_BUS 1
  #define SERIAL_BUS 2
  
  #define GPS_INTERFACE WIRE_BUS 
  
  #include <Wire.h>
  #define GPS_STREAM Wire
  #define GPS_ADR 0x42
  
  
  #define CONSOLE_STREAM SerialUSB
  #define CONSOLE_BAUD 9600
  
  #define READ_TIMEOUT_MS 100
  
  #define BUFFER_SIZE 256
  #define GPS_COORDS_SIZE 100
  
  // ** Passthrough Fields **
  #if defined(R4XX)
  unsigned long baud = 115200;  //start at 115200 allow the USB port to change the Baudrate
  #else 
  unsigned long baud = 9600;  //start at 9600 allow the USB port to change the Baudrate
  #endif
  
  char tmp;
  char tmp2;
  uint8_t gpsCoords[GPS_COORDS_SIZE] = {0};
  uint8_t gpsCoordsClean[GPS_COORDS_SIZE] = {0};
  uint8_t testArr[] = "12.345678,91.234567,\0";
  char latLongPosition[GPS_COORDS_SIZE] = {0};
  int isGPSInitialized = 0;
  
  typedef enum Module { NA, SARA, GPS } Module;
  Module module = NA;
  
  // ** GPS Fields **
  uint8_t buffer[BUFFER_SIZE];
  
  void setup()
  {
  // ** Passthrough Setup **
  #ifdef powerPin
      // Put voltage on the nb-iot module
      pinMode(powerPin, OUTPUT);
      digitalWrite(powerPin, HIGH);
  #endif
  
  #ifdef R4XX
      // Switch module voltage
      pinMode(voltagePin, OUTPUT);
      digitalWrite(voltagePin, LOW);
  #endif
  
  #ifdef enablePin
      // Set state to active
      pinMode(enablePin, OUTPUT);
      digitalWrite(enablePin, HIGH);
  #endif // enablePin
  
    // Start communication
    MODEM_STREAM.begin(baud);
  
    Serial.begin(57600);
  
    // ** GPS SETUP **
    gpsEnable(true);
    Wire.begin();
    CONSOLE_STREAM.begin(CONSOLE_BAUD);
  }
  
  void loop() 
  {
    // ** Passthrough Loop **
    
    memset(buffer, 0, BUFFER_SIZE);
    readGPS();
    
    while (Serial.available()){
      tmp = Serial.read();
      
      if (tmp == '<'){
        tmp = Serial.read();
        module = determineModuleFromMessage(tmp);
      }
      if(module == GPS){
        memset(buffer, 0, BUFFER_SIZE);
        readGPS();
        writeGPSToSerial();
        module = NA;
        break;
      } else {
        MODEM_STREAM.write(tmp);
      }
    }
    
    while (MODEM_STREAM.available())
    {     
      Serial.write((char) MODEM_STREAM.read());
    }
    
  }
  
  Module determineModuleFromMessage(char msg){
    Module result;
    if (msg == 'S'){
      result = SARA;
    }
    else if (msg == 'G'){
      result = GPS;
    }
    else {
      result = NA;
    }
    tmp2 = Serial.read();
    return result;
  }
  
  void updateGPSCoordinates(){
    int j = 0;
    for (int i = 0; i < BUFFER_SIZE - 50; i++){
      if (buffer[i] == '$' ){
        if (buffer[i+1] == 'G' &&
            buffer[i+2] == 'N' &&
            buffer[i+3] == 'R' &&
            buffer[i+4] == 'M' &&
            buffer[i+5] == 'C'){
              memset(gpsCoords, 0, GPS_COORDS_SIZE);
              while (buffer[i] != '\n' && buffer[i] != '\0' && i < BUFFER_SIZE){
                gpsCoords[j] = buffer[i];
                i++;
                j++;
              }
              gpsCoords[j] = '\n';
              isGPSInitialized = 1;
              return;
         }
      }
    }
    cleanGPS();
  }
  
  void gpsEnable(bool state)
  {
    //Enable GPS module
    pinMode(GPS_ENABLE, OUTPUT);
    digitalWrite(GPS_ENABLE, state);
  }
  
  size_t readGPS()
  {
    return readUbloxI2cStream();
  }
  
  size_t readConsole() 
  {
    return readSerialStream((Stream*)&CONSOLE_STREAM);
  }
  
  size_t readUbloxI2cStream()
  {
    uint16_t count = 0;
    Wire.beginTransmission(GPS_ADR);
    Wire.write((uint8_t)0xFD);
    Wire.endTransmission(false);
    Wire.requestFrom(GPS_ADR, 2);
    count = (uint16_t)(Wire.read() << 8) | Wire.read();
    count = (count > BUFFER_SIZE) ? BUFFER_SIZE : count;
  
    if (count) {
      for (size_t i = 0; i < (count-1); i++) {
        Wire.requestFrom(GPS_ADR, 1, false);
        buffer[i] = Wire.read();
      }
      Wire.requestFrom(GPS_ADR, 1);
      buffer[count-1] = Wire.read();
      updateGPSCoordinates();
    }
    return count;
  }
  
  
  size_t readSerialStream(Stream* stream) 
  {
    uint32_t last = millis();
    size_t count = 0;
    while ((count < BUFFER_SIZE) && (millis() < (last + READ_TIMEOUT_MS))) {
      if (stream->available()) {
        buffer[count++] = stream->read();
        last = millis();
      }
    }
    return count;  
  }
  
  void writeGPS(size_t count)
  {
    Wire.beginTransmission(GPS_ADR);
    Wire.write(buffer, count);
    Wire.endTransmission();
  }
  
  void convertToLatLong(){
    //WE ALWAYS ASSUME N/E due to location
    char degreesN[3];
    char minutesN[9];
    char degreesE[4];
    char minutesE[9];
  
    memcpy(&degreesN[0], &gpsCoordsClean[0], 2*sizeof(char));
    memcpy(&minutesN[0], &gpsCoordsClean[2], 8*sizeof(char));
    memcpy(&degreesE[0], &gpsCoordsClean[13], 3*sizeof(char));
    memcpy(&minutesE[0], &gpsCoordsClean[16], 8*sizeof(char));
    
    double degN = atof(degreesN); //= scanf("%ld",degreesN);
    double minN = atof(minutesN);
    double degE = atof(degreesE);
    double minE = atof(minutesE);
    
    double latitude = degN + (minN/60.0 );
    double longitude = degE + (minE/60.0);
  
    String pos = String(latitude, 6) + ',' + String(longitude, 6) + ',';
    memset(latLongPosition, '\0', GPS_COORDS_SIZE);
    strcpy(latLongPosition, pos.c_str());
    CONSOLE_STREAM.println("\n--- inside latlong function ---");
    CONSOLE_STREAM.println(latLongPosition);
  
  }
  
  void printToConsole(uint8_t * arr, int arr_size){
    for(int i = 0; i <= arr_size; i++) {
      CONSOLE_STREAM.write((char) arr[i]);
    }
    CONSOLE_STREAM.write('\n');
  }
  
  
  void writeConsole(size_t count)
  {
    for (size_t i = 0; i < count; i++) {
      CONSOLE_STREAM.write(buffer[i]);
    }
  }
  
  void cleanGPS(){
    int beginParam = 3;
    int endParam = 5;
    int commaCnt = 0;
    int i = 0;
    int j = 0;
    bool startClean = false;
    
    while (gpsCoords[i] != '\0' && commaCnt <= endParam){
      if(gpsCoords[i] == ',' && commaCnt == beginParam-1){
          startClean = true;
          commaCnt++;
      } else if(gpsCoords[i] == ','){
          commaCnt++;
      }
      if(startClean){
        gpsCoordsClean[j] = gpsCoords[i+1];
        j++;
      }
      i++;
    }
    convertToLatLong();
    memset(gpsCoordsClean, '\0', GPS_COORDS_SIZE);
  }
  
  void printGPS(){
    int i = 0;
    while (i < GPS_COORDS_SIZE && latLongPosition[i] != '\n'){
      CONSOLE_STREAM.write(latLongPosition[i]);
      i++;
    }
    CONSOLE_STREAM.write('\n');
  }
  
  void writeGPSToSerial(){
    int i = 0;
    CONSOLE_STREAM.println("\n--- inside write gps to serial function ---");
    while (i < GPS_COORDS_SIZE && latLongPosition[i] != '\n' && latLongPosition[i] != '\0'){
      Serial.write(latLongPosition[i]); 
      CONSOLE_STREAM.write(latLongPosition[i]);
      i++;
    }
   
  }
  
