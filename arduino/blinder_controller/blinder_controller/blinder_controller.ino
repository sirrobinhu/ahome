#include <EEPROM.h>
#include <IRremote.h>
#include "nRF24L01.h"
#include "RF24.h" 
#include "EEPROMAnything.h"

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

const int stepsPerRevolution = 200;
bool directionUp = true;
int motorSpeed = 1000;
int actualSteps = 0;

struct config_t
{
    int motorSpeed;
    int actualSteps;
    int minSteps;
    int maxSteps;
} configuration;

enum tool {
  MASTER = 0,
  BALCONY = 1,
};

struct package
{
  tool id;
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

  myRadio.begin(); 
  myRadio.setChannel(115); 
  myRadio.setPALevel(RF24_PA_MAX);
  myRadio.setDataRate( RF24_250KBPS ) ; 
  myRadio.openReadingPipe(1, addresses[0]);
  myRadio.startListening();
}

void loop()
{   

  if ( myRadio.available()) 
  {    
    while (myRadio.available())
    {      
      myRadio.read( &data, sizeof(data) );
      delay(20);            
//      Serial.println(data.packageNum);
//      Serial.println(data.light);
      Serial.print("Size: ");
      Serial.println(sizeof(data));
    }
  }
  if (irrecv.decode(&results)) {
    Serial.println(results.value);
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
  if(configuration.actualSteps >= configuration.minSteps || forced) {
    configuration.actualSteps--;
    digitalWrite(UPLEDPIN, HIGH);
    digitalWrite(DIRPIN, HIGH);
    go();
  }
}

void goDown(bool forced) {
  if(configuration.actualSteps <= configuration.maxSteps || forced) {
    configuration.actualSteps++;
    digitalWrite(DOWNLEDPIN, HIGH);
    digitalWrite(DIRPIN, LOW);
    go();
  }  }

void go() {
  for(int x = 0; x < stepsPerRevolution; x++)
  {
    digitalWrite(STEPPIN, directionUp);
    delayMicroseconds(motorSpeed);
    digitalWrite(STEPPIN, LOW);
    delayMicroseconds(motorSpeed);
  }
  Serial.print("Steps: ");
  Serial.println(configuration.actualSteps);
  EEPROM_writeAnything(0, configuration);
}
