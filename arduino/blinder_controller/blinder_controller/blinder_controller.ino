#include <EEPROM.h>
#include "EEPROMAnything.h"
// Define pin connections & motor's steps per revolution
#define DIRPIN 2
#define STEPPIN 3
#define UPBUTTONPIN 5
#define DOWNBUTTONPIN 6
#define UPLEDPIN 7
#define DOWNLEDPIN 8

const int stepsPerRevolution = 200;
bool directionUp = true;
int motorSpeed = 1000;
int minSteps = 0;
int maxSteps = 20;
int actualSteps = 0;
struct config_t
{
    int motorSpeed;
    int actualSteps;
} configuration;

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

  EEPROM_readAnything(0, configuration);
  Serial.print("Speed: ");
  Serial.println(configuration.motorSpeed);
}
void loop()
{ 
  
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
  if(configuration.actualSteps >= minSteps || forced) {
    configuration.actualSteps--;
    digitalWrite(UPLEDPIN, HIGH);
    digitalWrite(DIRPIN, HIGH);
    go();
  }
}

void goDown(bool forced) {
  if(configuration.actualSteps <= maxSteps || forced) {
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
    delayMicroseconds(motorSpeed);
    digitalWrite(STEPPIN, LOW);
    delayMicroseconds(motorSpeed);
  }
  Serial.print("Steps: ");
  Serial.println(configuration.actualSteps);
  EEPROM_writeAnything(0, configuration);
}
