// Same than device_type in the database
enum command {
  LIGHT,
  TEMP,
  TM,
  DT,
  PULL,
  SPEED,
  STATE,
  SMAX,
  SMIN
};

char master[] =  "a1h";

struct package
{
  char sender[3];
  char receiver[3];
  byte channelId = 111;  
  byte h = 0;
  byte m = 0;
  byte d = 0;
  byte mo = 0;
  byte y = 0;
  command cmd;
  char value[12];
};
