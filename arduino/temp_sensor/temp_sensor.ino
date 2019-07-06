#include "RF24.h"
#include "LowPower.h"
#include "C:\Users\viktor\Documents\Projects\SmartHome\arduino\library\ahomePackage.h"

#define TEMP_PIN A0
#define PHOTOCELL_PIN A4
#define D2 2

float reading;
RF24 myRadio (7, 8); // in Mega can use> (48, 49); 
byte addresses[][6] = {"0"}; 
String myId = "c5j";

bool sendTemp = true;

typedef struct package Package;
Package data;

void setup() {
  Serial.begin(9600);
  analogReference(INTERNAL);
  delay(100);
  myRadio.begin();  
  myRadio.setChannel(115); 
  myRadio.setPALevel(RF24_PA_MAX);
  myRadio.setDataRate( RF24_250KBPS ) ; 
  myRadio.openWritingPipe( addresses[0]);
  pinMode(PHOTOCELL_PIN, INPUT);
  pinMode(D2, OUTPUT);

  itoa(master, data.receiver, 10);
  myId.toCharArray(data.sender, 4);
}

void loop() {  
  if (sendTemp)
  {
    float tempC = (analogRead(TEMP_PIN) * 0.1039);  
    itoa(round(tempC), data.value, 10);
    data.cmd = TEMP;
    Serial.print(data.cmd);
    Serial.print(": ");
    Serial.println(data.value);    
  } else {
    int light = analogRead(PHOTOCELL_PIN);    
    itoa(light, data.value, 10);
    data.cmd = LIGHT;
    Serial.print(data.cmd);
    Serial.print(": ");
    Serial.println(data.value);    
  }     
  delay(50);
  Serial.print("size: ");
  Serial.println(sizeof(data));
  delay(50);
  Serial.print("sender: ");
  Serial.println(String(data.sender));
  
  sendTemp = !sendTemp;
  myRadio.write(&data, sizeof(data));  
  digitalWrite(D2, HIGH);
  delay(10);
  digitalWrite(D2, LOW);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}
