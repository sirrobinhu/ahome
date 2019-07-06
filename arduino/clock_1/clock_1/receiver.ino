#include "RF24.h" 
#include <LiquidCrystal.h>
#include <math.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <dht.h>

#define BACKLIGHT_PIN 9

#define RS 12
#define RW 11
#define ENABLE 10
#define D4 5
#define D5 4
#define D6 3
#define D7 2
#define PHOTOCELL_PIN A0
#define DHT_APIN A1 // Analog Pin sensor is connected to
#define D39 39
#define D42 42

RF24 myRadio (7, 8); // in Mega can use> (48, 49); 
byte addresses[][6] = {"0"}; 
LiquidCrystal lcd(RS, RW, ENABLE, D4, D5, D6, D7);
int backLight = 250;
int lastReceivingTime = 0;
int actualTime = 0;
dht DHT;

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

float Celcius=0;
float Fahrenheit=0;

byte customChar[] = {
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B01010,
  B00000,
  B00000
};

byte startByte[] = {
  B00000,
  B00000,
  B11111,
  B11110,
  B11100,
  B11000,
  B10000,
  B00000
};

byte endByte[] = {
  B00000,
  B00000,
  B11111,
  B01111,
  B00111,
  B00011,
  B00001,
  B00000
};


void setup() 
{
  Serial.begin(9600);
  delay(100);
  myRadio.begin(); 
  myRadio.setChannel(115); 
  myRadio.setPALevel(RF24_PA_MAX);
  myRadio.setDataRate( RF24_250KBPS ) ; 
  myRadio.openReadingPipe(1, addresses[0]);
  myRadio.startListening();

  pinMode(PHOTOCELL_PIN, INPUT);
  pinMode(BACKLIGHT_PIN, OUTPUT);
  pinMode(D42, OUTPUT);
  pinMode(D39, OUTPUT);
  lcd.begin(20,4);
  lcd.clear();
  lcd.createChar(0, customChar);
  lcd.createChar(1, startByte);
  lcd.createChar(2, endByte);
  lcd.home();
  lcd.setCursor(0,1);
  for (int i = 0; i < 10; i++){
    lcd.write((byte)1);
    lcd.write((byte)2);
  }
  
  analogWrite(BACKLIGHT_PIN, backLight);
}

void loop()  
{
  int photocellReading = analogRead(PHOTOCELL_PIN);

  int bl = map(photocellReading, 0, 1000, 1, 2);
//  backLight = map(photocellReading, 0, 800, 5, 255);
//  if (backLight > 255){
//    backLight = 255;
//  }

  switch (bl) {
    case 1:
      backLight = 150;
      break;
    case 2:
      backLight = 250;
      break;
    case 3:
      backLight = 100;
      break;    
    default:
      backLight = 255;
      break;
  }

//  Serial.println(bl);
//  Serial.println(backLight);
  
  analogWrite(BACKLIGHT_PIN, backLight);
  digitalWrite(D42, HIGH);
  digitalWrite(D39, HIGH);
  tmElements_t tm;
  if (RTC.read(tm)) {
    lcd.setCursor(0,0);
    lcd.print(print2digits(tm.Hour));
    lcd.print(':');
    lcd.print(print2digits(tm.Minute));
    //lcd.print(':');
    //lcd.print(print2digits(tm.Second));
    lcd.print("     ");
    lcd.print(print2digits(tm.Day));
    lcd.print('/');
    lcd.print(print2digits(tm.Month));
    lcd.print('/');
    lcd.print(tmYearToCalendar(tm.Year));

    actualTime = String(tmYearToCalendar(tm.Year) + print2digits(tm.Month) + print2digits(tm.Day) + print2digits(tm.Hour) + print2digits(tm.Minute) + print2digits(tm.Second)).toInt();
//    Serial.println(actualTime - lastReceivingTime);
    int lTime = actualTime - lastReceivingTime;
    
  } else {
    if (RTC.chipPresent()) {
      Serial.println("The DS1307 is stopped.  Please run the SetTime");
      Serial.println("example to initialize the time and begin running.");
      Serial.println();
    } else {
      Serial.println("DS1307 read error!  Please check the circuitry.");
      Serial.println();
    }
    //delay(9000);
  }
  
  if ( myRadio.available()) 
  {
    //    Serial.println("Available");
    while (myRadio.available())
    {
      myRadio.read( &data, sizeof(data) );      
    }

    if(data.id == MASTER) {
      Serial.println (data.id);

      Serial.println (sizeof(data));
      if(data.h > 0 && data.m > 0 && data.d > 0 && data.mo > 0 && data.y > 0 && data.y < 2055)
      {
        
        setTime(data.h, data.m, 00, data.d, data.mo, data.y);
        RTC.set(now());
      }

      lastReceivingTime = String(tmYearToCalendar(tm.Year) + print2digits(tm.Month) + print2digits(tm.Day) + print2digits(tm.Hour) + print2digits(tm.Minute) + print2digits(tm.Second)).toInt();    
      lcd.setCursor(0,3);
      lcd.print("Outside temp:  ");
      //lcd.print(String(round(data.temperature + 1)));
      lcd.print(String(round(data.temperature)));
      lcd.print(" C");
      lcd.print((char)223);
      Serial.print("light: ");
      Serial.println(data.light);
      Serial.print("pnum: ");
      Serial.println(data.packageNum);
      Serial.print("hour: ");
      Serial.println(data.h);
      Serial.print("minute: ");
      Serial.println(data.m);
      Serial.print("day: ");
      Serial.println(data.d);
      Serial.print("month: ");
      Serial.println(data.mo);
      Serial.print("year: ");
      Serial.println(data.y);
      Serial.print("temperature: ");
      Serial.println(data.temperature);      
      Serial.print("text: ");
      Serial.println(data.text);      
    }
    
    DHT.read11(DHT_APIN);  
    
    lcd.setCursor(0,2);
    lcd.print("Inside temp:   ");  
    lcd.print(String(round(DHT.temperature -1)));  
    lcd.print(" C");
    lcd.print((char)223);    
    }
}

String print2digits(int number) {
  if (number >= 0 && number < 10) {
    return('0' + String(number));
  }
  return(String(number));
}
