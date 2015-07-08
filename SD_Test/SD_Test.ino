#include <DHT.h>
#include <SPI.h>
#include <SD.h>

#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const int chipSelect = 10;

void sensorBegin()
{
  SD.begin( chipSelect );
  pinMode( chipSelect, OUTPUT );
  dht.begin();
}

float currentTemp()
{
 float f = dht.readTemperature( true );
 return f; 
}

void setup()
{
  Serial.begin( 9600 );
  sensorBegin();  

  File temperatures = SD.open( "tempTest.csv", FILE_WRITE );
  for ( int i = 1; i <= 150; i++ )
  {
    temperatures.print( i );
    temperatures.print( ", " );
    temperatures.print( currentTemp() );
    temperatures.println();
    delay( 200 );
  }
  temperatures.close();
  Serial.println( "Done" );
}

void loop()
{
}
