#include <Wire.h>
//DONT'T USE THIS
void setup() {
  Wire.begin();
  adcBegin();
  Serial.begin(9600);
}

void loop() {
  Serial.println(adcRead());
}

#define ADC_ADDR 0x48

byte MSB = 0b01000000;
byte LSB = 0b10000011;

void adcBegin() {
  Wire.beginTransmission(ADC_ADDR);
  Wire.write(0x01);
  Wire.write(MSB);
  Wire.write(LSB);
  Wire.endTransmission();
}

int adcRead() {
  int temp; 
  Wire.beginTransmission(ADC_ADDR);
  Wire.write(0x00);
  Wire.endTransmission(); 
  Wire.requestFrom(ADC_ADDR, 2);
  temp = ((Wire.read() << 8) | Wire.read());
  return temp;
}

