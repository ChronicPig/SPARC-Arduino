
void setupAXL345(){
  // all interupts are disabled by defult
  
  // sets to 100Hz data rate
  writeRegister(AXL345_Address, 0x2C, 0b00001010);
  // sets range to +/-16g and full resolution, and right justifys data in registers
  writeRegister(AXL345_Address, 0x31, 0b00001011);
  // sets FIFO to stream mode
  writeRegister(AXL345_Address, 0x38, 0b10000000);
  // takes it out of stand by into measurment mode so it can start working.
  // cant write to registers while in measurment mode.
  writeRegister(AXL345_Address, 0x2D, 0b00001000);
}

void updateAccArrays(){
  level_Acc = (readRegister_Int(AXL345_Address, 0x39)) & 0b00111111;
    for(int i = 0; i < level_Acc; i++){
      burstReadAccAxis();
    }  
 }  

void burstReadAccAxis() {
  
  byte LSB;
  byte MSB;

  Wire.beginTransmission(AXL345_Address);
  Wire.write(0x32); // register to read
  Wire.endTransmission();

  Wire.requestFrom(AXL345_Address, (uint8_t)6); // read all registers

    for(int m = 0; m < 3; m++){
      LSB = Wire.read();
      MSB = Wire.read();
      Acc[m] = ((MSB << 8) | LSB);
  }
 }

