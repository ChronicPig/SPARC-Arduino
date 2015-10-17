
void setADC(byte channel, byte gainLevel) {
 //channel is 2bit, gain is 3bit, OR ELSE WILL SCREW EVERYTHING UP
 byte MSB;
 MSB = 0b01000000 | (channel << 4) | (gainLevel << 1);
  Wire.beginTransmission(ADC_ADDR);
  Wire.write(0x01);
  Wire.write(MSB);
  Wire.write(0b10000011);
  Wire.endTransmission();
  
  Serial.print(MSB);
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

