#include <Adafruit_BNO08x.h>
#include "Mux_Control.h"

#define BNO08X_CS 10
#define BNO08X_INT 9
#define BNO08X_RESET -1

Adafruit_BNO08x  bno08x[2]{(BNO08X_RESET), (BNO08X_RESET)};
sh2_SensorValue_t sensorValue;

struct SSensorData
{
  float real;
  float i;
  float j;
  float k;

  float x;
  float y;
  float z;
};

#define NUMBER_OF_SENSORS 2
SSensorData sensorData[NUMBER_OF_SENSORS];

void setup(void) {
  pinMode(10, OUTPUT); 
  digitalWrite(10, LOW);
  delay(100);
  digitalWrite(10, HIGH);
  
  Serial.begin(115200);

  while(Serial.available() == 0);

  Serial.println("Startup");
  
  for (byte i = 0 ; i < NUMBER_OF_SENSORS ; i++)
  {
    initBNO(i);
    delay(100);
  }
  
  Serial.println("Reading events");
  delay(100);
}

void initBNO(byte port)
{
  Serial.print("Init port");
  Serial.println(port);
  enableMuxPort(port);
  
  if (!bno08x[port].begin_I2C()) 
  {
    Serial.println("Failed to find BNO08x chip");
  }
  else
  {
    Serial.println("BNO08x Found!");
  
    for (int n = 0; n < bno08x[port].prodIds.numEntries; n++)
    {
      Serial.print("Part ");
      Serial.print(bno08x[port].prodIds.entry[n].swPartNumber);
      Serial.print(": Version :");
      Serial.print(bno08x[port].prodIds.entry[n].swVersionMajor);
      Serial.print(".");
      Serial.print(bno08x[port].prodIds.entry[n].swVersionMinor);
      Serial.print(".");
      Serial.print(bno08x[port].prodIds.entry[n].swVersionPatch);
      Serial.print(" Build ");
      Serial.println(bno08x[port].prodIds.entry[n].swBuildNumber);
    }
  
    setReports(port);
  }
  disableMuxPort(port);
}

void setReports(byte port) {
  Serial.println("Setting desired reports");
  
  if (! bno08x[port].enableReport(SH2_GAME_ROTATION_VECTOR)) 
  {
    Serial.println("Could not enable game vector");
  }
}

void loop() 
{
  for (byte i = 0 ; i < NUMBER_OF_SENSORS ; i++)
  {
    delay(5);
    enableMuxPort(i);

    if (bno08x[i].wasReset())
    {
      Serial.print("sensor was reset ");
      setReports(i);
    }
 
    if (bno08x[i].getSensorEvent(&sensorValue))
    {
      switch (sensorValue.sensorId)
      {
        case SH2_GAME_ROTATION_VECTOR:
          sensorData[i].real = sensorValue.un.gameRotationVector.real;
          sensorData[i].i = sensorValue.un.gameRotationVector.j;        
          sensorData[i].j = sensorValue.un.gameRotationVector.k;
          sensorData[i].k = sensorValue.un.gameRotationVector.i;
          break;
      }
    }
    disableMuxPort(i);

    Serial.print("Sensor");
    Serial.print(i);
    Serial.print("\t");

    Serial.print(sensorData[i].real);
    Serial.print("\t");
    Serial.print(sensorData[i].i);
    Serial.print("\t");
    Serial.print(sensorData[i].j);
    Serial.print("\t");
    Serial.println(sensorData[i].k);
  }
}
