// this method simply says the address of a registry in barometer as needed
// for some functions
void lookAtRegister(byte reg){
  Wire.beginTransmission(MS5607_Address);
  Wire.write(reg);   
  Wire.endTransmission();
}

// Read Coefficients subroutine for Baro
unsigned int ReadCoefficient(const uint8_t coefNum)
  {
    unsigned int rC=0;
    Wire.beginTransmission(MS5607_Address);
    Wire.write(0xA0 | (coefNum << 1)); // send PROM READ command
    Wire.endTransmission(); 
    Wire.requestFrom(MS5607_Address, static_cast<uint8_t>(2));
       rC = Wire.read();
       rC = (rC << 8) | Wire.read();
       return rC;
  }

// for reading baro's ADC as defined by what register was just looked at
unsigned long ReadADC(){  
    Wire.beginTransmission(MS5607_Address);
    Wire.write(0x00);
    Wire.endTransmission();
    Wire.requestFrom(MS5607_Address, (uint8_t)3);
        unsigned long temp = Wire.read();
        temp = (temp << 8) | Wire.read();
        temp = (temp << 8) | Wire.read();
        return temp;
  }

