//SD Card
#include <SPI.h>
#include <SD.h>
//Altimeter
#include <Wire.h>
#include "IntersemaBaro.h"
Intersema::BaroPressure_MS5607B baro(true);
//Gyro
#include "I2Cdev.h"
#include "MPU6050.h"
MPU6050 accelgyro;
int16_t ax, ay, az;
int16_t gx, gy, gz;
#define OUTPUT_READABLE_ACCELGYRO

const int chipSelect = 10;

void sensorBegin()
{
  SD.begin( chipSelect );
  pinMode( chipSelect, OUTPUT );
  //Altimeter
  baro.init();
  //Accel
  accelgyro.initialize();
}

int alt () {
 return baro.getHeightCentiMeters(); 
}

void getAccel () {
 accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz); 
}

void setup()
{
  Serial.begin( 9600 );
  sensorBegin();  

  File SensorData = SD.open( "saData.csv", FILE_WRITE );
  for ( int i = 1; i <= 150; i++ )
  {
    getAccel();
    SensorData.print("a/g:\t");
    SensorData.print(ax); SensorData.print("\t");
    SensorData.print(ay); SensorData.print("\t");
    SensorData.print(az); SensorData.print("\t");
    SensorData.print(gx); SensorData.print("\t");
    SensorData.print(gy); SensorData.print("\t");
    SensorData.print(gz);
    SensorData.print(alt());
    SensorData.println("");
    delay( 200 );
  }
  SensorData.close();
  Serial.println( "Done" );
}

void loop()
{
}
