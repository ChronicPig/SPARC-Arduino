#include <SPI.h>
#include <SD.h>

const int chipSelect = 10;

void setup()
{
  Serial.begin(9600);
  pinMode(chipSelect, OUTPUT);

  SD.begin(chipSelect);

  File test = SD.open("test.csv", FILE_WRITE);
  
  test.print(1.6, 3);
  test.print(",");
  test.print(2.20, 3);
  test.println();

  test.print(2.264556, 3);
  test.print(",");
  test.print(4.57272277, 3);
  test.println();

  test.print(3.16268352, 3);
  test.print(",");
  test.print(6.8161618, 3);
  
  test.close();
}

void loop() {
  // put your main code here, to run repeatedly:

}
