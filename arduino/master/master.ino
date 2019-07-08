#include "RF24.h" 
#include <ArduinoJson.h>
#include <TimeLib.h>
#include "C:\Users\viktor\Documents\Projects\SmartHome\arduino\library\ahomePackage.h"

#define RESET_PIN 3
#define LED 2

RF24 myRadio (7, 8); // in Mega can use> (48, 49); 
byte addresses[][6] = {"0"}; 
bool sTime = false;

typedef struct package Package;
Package outgoingData;
Package incomingData;
bool ledPin = true;

void setup() {
  Serial.begin(9600);
  
  digitalWrite(RESET_PIN, HIGH);
  pinMode(RESET_PIN, OUTPUT);
  pinMode(LED, OUTPUT);
  delay(100);
  
  myRadio.begin();
  myRadio.setPayloadSize(128);
  myRadio.setChannel(115); 
  myRadio.setPALevel(RF24_PA_MAX);
  myRadio.setDataRate( RF24_250KBPS ) ; 
  myRadio.openReadingPipe(1, addresses[0]);
  myRadio.openWritingPipe(addresses[0]);
  myRadio.startListening();
}

void loop() {
    while (Serial.available() > 0) { // if any data available
      String incomingString = Serial.readString();    
      Serial.println(incomingString);
      const char *buf = incomingString.c_str();
      char *command = strtok((char*)buf, ":");    
      char *value= strtok(NULL, "");

      char *receiver = strtok((char*)value, ",");
      char *ovalue= strtok(NULL, "");
      myRadio.stopListening();
      
      if(String(command)== "TEMP") {  
        outgoingData.cmd = TEMP;        
      }  
  
      if(String(command) == "DT") {
        outgoingData.cmd = DT;
      }
      
      if(String(command) == "TM") {
        outgoingData.cmd = TM;
      }

      if(String(command) == "PULL") {
        outgoingData.cmd = PULL;
      }

      if(String(command) == "SPEED") {
        outgoingData.cmd = SPEED;
      }

      strcpy(outgoingData.value, ovalue);             
      strcpy(outgoingData.receiver, receiver);
      myRadio.write(&outgoingData, sizeof(outgoingData));      
      myRadio.startListening();
  }

    DynamicJsonDocument myStruct(200);
    char t[10]="test";
    if ( myRadio.available()) 
    {    
      while (myRadio.available())
      {
        digitalWrite(LED, HIGH);
        myRadio.read( &incomingData, sizeof(incomingData) );
        delay(20);      
        digitalWrite(LED, LOW);      
      }
      myRadio.stopListening();
  
      myStruct["cmd"] = String(incomingData.cmd);
      myStruct["sender"] = incomingData.sender;
      myStruct["value"] = incomingData.value;
         
      serializeJson(myStruct, Serial);
      Serial.println();
      myRadio.startListening();
    }
}
