#include <DHT.h>
#include <SD.h>
#include <SPI.h>

//DHT setup
#define DHTPIN 2
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);
void setup () {
  Serial.begin(9600);
  sensorSetup();
}

//Calls all begin functions for sensors
void sensorSetup () {
  dht.begin();
}

void loop () {
  //Write to SD Card
  delay(1);
  //Values to write
  Serial.println((float)currentTemp());
}

//Sensor functions
//DHT
float currentTemp () {
 float f = dht.readTemperature(true);
 return f; 
}

