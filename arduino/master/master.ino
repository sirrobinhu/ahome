#include "RF24.h" 
#include <ArduinoJson.h>
#include <TimeLib.h>

#define RESET_PIN 3
#define LED 2

RF24 myRadio (7, 8); // in Mega can use> (48, 49); 
byte addresses[][6] = {"0"}; 
bool sTime = false;

enum tool {
  MASTER = 0,
  BALCONY = 1,
};

struct package
{
  tool id = MASTER;
  unsigned int packageNum = 0;
  byte light = 0;  
  byte h = 0;
  byte m = 0;
  byte d = 0;
  byte mo = 0;
  byte y = 0;
  char temperature = 0;
  char  text[10];  
};
typedef struct package Package;
Package data;
Package incoming_data;
bool ledPin = true;

void setup() {
  Serial.begin(9600);
  
  digitalWrite(RESET_PIN, HIGH);
  pinMode(RESET_PIN, OUTPUT);
  pinMode(LED, OUTPUT);
  delay(100);
  
  myRadio.begin(); 
//  myRadio.enableDynamicPayloads();
  myRadio.setPayloadSize(128);
  myRadio.setChannel(115); 
  myRadio.setPALevel(RF24_PA_MAX);
  myRadio.setDataRate( RF24_250KBPS ) ; 
  myRadio.openReadingPipe(1, addresses[0]);
  myRadio.openWritingPipe(addresses[0]);
  myRadio.startListening();  
}

void loop() {
  //digitalWrite(LED, ledPin);
  while (Serial.available() > 0) { // if any data available
    String incomingString = Serial.readString();    
    
    const char *buf = incomingString.c_str();
    char *command = strtok((char*)buf, ":");    
    char *value= strtok(NULL, "");
    
    if(!strcmp(command, "led")) {
      if(!strncmp(value, "on", 2)) {
        digitalWrite(LED, HIGH);
      }
  
      if(!strncmp(value, "off", 3)) {
        digitalWrite(LED, LOW);
      }
    }

    
    if(!strcmp(command, "time")) {
      String val = String(value);
//      Serial.println(val);
      
      String y;
      String mo;
      String d;
      String h;
      String mi;
      y = String(value[0]) + String(value[1]) + String(value[2]) + String(value[3]);
            
      mo = val[5];
      if(String(val[4]) != "0") {
        mo = String(val[4]) + mo;
        }

      d = String(val[7]);
      if(String(val[6]) != "0") {
        d = String(val[6]) + d;
      }

      h = String(val[9]);
      if(String(val[8]) != "0") {
        h = String(val[8]) + h;
      }
      
      mi = String(val[11]);
      if(String(val[10]) != "0") {
        mi = String(val[10]) + mi;
      }
      
      
      setTime(h.toInt(), mi.toInt(), 00, d.toInt(), mo.toInt(), y.toInt());
      sTime = true;
    }    
  }

  DynamicJsonDocument myStruct(200);
  char t[10]="test";
  if ( myRadio.available()) 
  {    
    while (myRadio.available())
    {
      digitalWrite(LED, HIGH);
      myRadio.read( &data, sizeof(data) );
      delay(20);      
      digitalWrite(LED, LOW);      
    }    
    
//    if(sizeof(data.id) == 0){
//      digitalWrite(RESET_PIN, LOW);
//    }

    myRadio.stopListening();    
        
    //data.light = 44;
    //data.packageNum = 111;
    //strcpy(data.text, "Another!");       


    if(sTime) {
      data.h = hour();    
      data.m = minute();
      data.d = day();
      data.mo = month();
      data.y = year();
      sTime = false;
    }
        
    myRadio.write(&data, sizeof(data));
    setTime(0, 0, 0, 0, 0, 0);
    String deviceId(incoming_data.id);
    String test(t);
    myStruct["light"] = data.light;
    myStruct["packageNum"] = data.packageNum;
    myStruct["id"] = deviceId;
    myStruct["temperature"] = data.temperature;
       
    serializeJson(myStruct, Serial);
    Serial.println();
    myRadio.startListening();
  }
}
