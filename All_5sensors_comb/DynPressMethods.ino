
unsigned int getDynPress(){
  Wire.requestFrom(HSC_Address, static_cast<uint8_t>(2)); //if read 3 or 4 bytes, then can get temp as well
  unsigned int a = Wire.read() << 10; //shifted 10bits then back 2 to get rid of top 2 (or 3?) bits of sensor status
  a = a >> 2;
  a = a | Wire.read(); // add the LSB
  return a; //should be 8192 with no difference in pressure
}

