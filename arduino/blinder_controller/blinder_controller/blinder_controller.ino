#include <EEPROM.h>
#include <IRremote.h>
#include "nRF24L01.h"
#include "RF24.h" 
#include "EEPROMAnything.h"
#include "C:\Users\viktor\Documents\Projects\SmartHome\arduino\library\ahomePackage.h"

// Define pin connections & motor's steps per revolution
#define DIRPIN 2
#define STEPPIN 3
#define UPBUTTONPIN 5
#define DOWNBUTTONPIN 6
#define UPLEDPIN 7
#define DOWNLEDPIN 8
#define IRPIN 4

IRrecv irrecv(IRPIN);
decode_results results;
RF24 myRadio (9, 10);
byte addresses[][6] = {"0"}; 
String myId = "j6g";

const int stepsPerRevolution = 200;
bool directionUp = true;
int motorSpeed = 1000;
int actualSteps = 0;
bool irUp = false;
int irCounter = 0;

struct config_t
{
    int motorSpeed;
    int actualSteps;
    int minSteps;
    int maxSteps;
    double irup;
    double irdown;
    double ircont;
} configuration;

typedef struct package Package;
Package data;
Package outgoingData;

int stepsAddress = 0;

void setup()
{
  Serial.begin(9600);
  // Declare pins as Outputs
  pinMode(STEPPIN, OUTPUT);
  pinMode(DIRPIN, OUTPUT);
  pinMode(UPLEDPIN, OUTPUT);
  pinMode(DOWNLEDPIN, OUTPUT);

  pinMode(UPBUTTONPIN, INPUT);
  pinMode(DOWNBUTTONPIN, INPUT);

  irrecv.enableIRIn();
  
  EEPROM_readAnything(0, configuration);
  Serial.print("Speed: ");
  Serial.println(configuration.motorSpeed);
  if (configuration.motorSpeed <= 0)
  {
    configuration.motorSpeed = 1000;
  }

  if (configuration.maxSteps <= 0)
  {
    configuration.maxSteps = 20;
  }
  if (configuration.minSteps != 0)
  {
    configuration.minSteps = 0;
  }
  if (configuration.actualSteps < configuration.minSteps)
  {
    configuration.actualSteps = configuration.minSteps;
  }

  if (configuration.actualSteps > configuration.maxSteps)
  {
    configuration.actualSteps = configuration.maxSteps;
  }

//  configuration.irup = 2747854299;
//  configuration.irdown = 4034314555;
//  configuration.ircont = 4294967295;
  Serial.print("ActualSteps: ");
  Serial.println(configuration.actualSteps);

  Serial.print("MaxSteps: ");
  Serial.println(configuration.maxSteps);
  Serial.print("MinSteps: ");
  Serial.println(configuration.minSteps);

  myRadio.begin(); 
  myRadio.setChannel(115); 
  myRadio.setPALevel(RF24_PA_MAX);
  myRadio.setDataRate( RF24_250KBPS ) ; 
  myRadio.openReadingPipe(1, addresses[0]);
  myRadio.openWritingPipe( addresses[0]);
  myRadio.startListening();

  myId.toCharArray(outgoingData.sender, 4);
  outgoingData.cmd = STATE;
}

void loop()
{
  if ( myRadio.available()) 
  {    
    while (myRadio.available())
    {      
      myRadio.read( &data, sizeof(data) );
      delay(20);
      if(String(data.receiver) == myId) {
        if(data.cmd == PULL){
          Serial.println(data.value);
          if(String(data.value) == "up") {
            goUp(false); 
          }
          if(String(data.value) == "top") {
            goTop(); 
          }
          if(String(data.value) == "down") {
            goDown(false); 
          }
          if(String(data.value) == "bottom") {
            goBottom(); 
          }
        } 
        if(data.cmd == SPEED) {
          configuration.motorSpeed = String(data.value).toInt();
          Serial.print("Value: ");
          Serial.println(data.value);
          Serial.print("Speed: ");
          Serial.println(configuration.motorSpeed);
        }
        Serial.print("Value: ");
        Serial.println(data.value);
        Serial.print("Command: ");
        Serial.println(data.cmd);
        if(data.cmd == SMAX) {
          configuration.maxSteps = String(data.value).toInt();
          Serial.print("Value: ");
          Serial.println(data.value);
        }
        if(data.cmd == SMIN) {
          configuration.minSteps = String(data.value).toInt();
          Serial.print("Value: ");
          Serial.println(data.value);
        }
        if(data.cmd == IRUP) {
          configuration.irup = String(data.value).toDouble();
          Serial.print("Set irup");
        }
        if(data.cmd == IRDWN) {
          configuration.irdown = String(data.value).toDouble();
          Serial.print("Set irdwn");
        }
        if(data.cmd == IRCNT) {
          configuration.ircont = String(data.value).toDouble();
          Serial.print("Set ircnt");
        }
      }
    }
  }
  if (irrecv.decode(&results)) {
    Serial.println(results.value);
    if(results.value == configuration.irup)
    {
      irUp = true;   
      irCounter = 0;   
    }

    if(results.value == configuration.irdown)
    {
      irUp = false;  
      irCounter = 0;    
    }

    if(results.value == configuration.ircont)
    {
      irCounter++;
      Serial.println(irCounter);
      if(irCounter == 3) {
        if(irUp == true){
          goTop();
        }else {
          goBottom();
        }
        irCounter = 0;
      } else {
        if(irUp == true){
          goUp(false);  
          digitalWrite(UPLEDPIN, HIGH);
          digitalWrite(DOWNLEDPIN, LOW);
        }else {
          goDown(false);
          digitalWrite(UPLEDPIN, LOW);
          digitalWrite(DOWNLEDPIN, HIGH);
        }
      }      
    }
    irrecv.resume();
  }
  
  int upButtonState = digitalRead(UPBUTTONPIN);  
  if(upButtonState == HIGH)
  {    
    goUp(false);    
  } else {
    digitalWrite(UPLEDPIN, LOW);
  }
  int downButtonState = digitalRead(DOWNBUTTONPIN);  
  if(downButtonState == HIGH)
  {
    goDown(false);
  } else {
    digitalWrite(DOWNLEDPIN, LOW);  
  }  
}
void goUp(bool forced){
  Serial.println("up");
  if(configuration.actualSteps >= configuration.minSteps || forced) {
    configuration.actualSteps--;
    digitalWrite(UPLEDPIN, HIGH);
    digitalWrite(DIRPIN, HIGH);
    go();
  }
}

void goTop(){  
  Serial.println("up");
  while(configuration.actualSteps >= configuration.minSteps){
    configuration.actualSteps--;
    digitalWrite(DOWNLEDPIN, HIGH);
    digitalWrite(DIRPIN, HIGH);
    go();
  }  
}

void goBottom(){
  Serial.println("down");
  while(configuration.actualSteps < configuration.maxSteps){
    configuration.actualSteps++;
    digitalWrite(UPLEDPIN, HIGH);
    digitalWrite(DIRPIN, LOW);
    go();
  }  
}

void goDown(bool forced) {
  Serial.println("down");
  if(configuration.actualSteps < configuration.maxSteps || forced) {
    configuration.actualSteps++;
    digitalWrite(DOWNLEDPIN, HIGH);
    digitalWrite(DIRPIN, LOW);
    go();
  }  
}

void go() {
  for(int x = 0; x < stepsPerRevolution; x++)
  {
    digitalWrite(STEPPIN, directionUp);
    delayMicroseconds(configuration.motorSpeed);
    digitalWrite(STEPPIN, LOW);
    delayMicroseconds(configuration.motorSpeed);
  }
  Serial.print("Steps: ");
  Serial.println(configuration.actualSteps);
  EEPROM_writeAnything(0, configuration);
  if(configuration.actualSteps == configuration.maxSteps || configuration.actualSteps == configuration.minSteps) {
    myRadio.stopListening();   
    delay(50);
    itoa(configuration.actualSteps, outgoingData.value, 10);     
//    myRadio.write(&outgoingData, sizeof(outgoingData)); 
    delay(50);
    myRadio.startListening(); 
  }  
}
