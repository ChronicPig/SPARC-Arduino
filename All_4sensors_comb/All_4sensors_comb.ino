/*
all sensors work, just need to make it so sensors without FiFo offload on time schedual.
rework main loop to be more time efficient and redo timers to make sure everything is sampled at 100Hz.
only thing outside of loop is the Axis updates, may be able to merge into one method for efficiency.
barely gets all measurments out in 10ms (100hz).
*/
unsigned int t_1 = 0;
unsigned int t_2 = 0;

#include <Wire.h>

const byte MS5607_Address = 0x76; // I2C address of MS5607 barometer (static pressure sensor)
const byte AXL345_Address = 0x53; //I2C address of the AXL345 (accelerometer)
const byte L3G4200D_Address = 0x69; //I2C address of the L3G4200D (gyroscope)
byte HSC_Address = 0x28; //I2C address of honeywell diff. pressure sensor (dynamic pressure)

#define CTRL_REG1 0x20
#define CTRL_REG2 0x21
#define CTRL_REG3 0x22
#define CTRL_REG4 0x23
#define CTRL_REG5 0x24

unsigned long coefficients_[6]; // barometer coefficients for later calculations

// buffers for Acc and Gyro FIFOs to dump into
int Acc[3][32];
int Gyro[3][32];

// used to hold how many samples in Acc and Gyro FIFOs
byte level_Acc;
byte level_Gyro;

// Define variables for static temperature and static pressure respectively (24bit)
unsigned long Static_Temp;
unsigned long Static_Press;

// Define var for diff pressure sensor
unsigned int Dyn_Press;

void setup() 
{ 
  Wire.begin();
  TWBR = 24; // for 400kHz I2C
  Serial.begin(115200);

  setupAXL345(); // write to registries as needed. just edit registry entries
  setupL3G4200D(); // write to registries as needed. just edit registry entries
  
  // Reset barometer to make sure its "booted up" correctly
  lookAtRegister(0x1E); 

  // Read MS5607_Address Coefficients
  for(int i=0; i<6; ++i)
    {
        coefficients_[i] = ReadCoefficient(i + 1); 
        Serial.print("Coefficent ");
        Serial.print(i+1);
        Serial.print(" = ");
        Serial.println(coefficients_[i]); 
    }
}

void loop() 
{
  t_2 = millis();
  
  lookAtRegister(0x54); // tell baro to get temperature value at OSR 1024
  t_1 = millis(); //mark time of command
  
  Dyn_Press = getDynPress(); // get dynamic pressure in mean time

    updateAccArrays(); //and update/print Acc data
    // print all new Acc data
    Serial.println("Acc");
    for(int k = 0; k < 3; k++){
      Serial.print("Axis ");
      Serial.print(k);
      Serial.print(" :: ");
        for( int m = 0; m < level_Acc; m++){
         Serial.print(Acc[k][m]);
         Serial.print(", ");
        }
      Serial.println();
    }
  
  while( (millis() - t_1) < 3){
    //wait for Baro ADC
  }
  Static_Temp = ReadADC();  // fianlly grab temp from baro ADC (typical value 8077636)
   
  lookAtRegister(0x44);  // tell baro to start getting static pressure ADC value
  t_1 = millis();  //mark time of command
  
    updateGyroArrays();  // in meantime update/print all new Gyro data
    // print all new Gyro data
    Serial.println("Gyro");
    for(int k = 0; k < 3; k++){
      Serial.print("Axis ");
      Serial.print(k);
      Serial.print(" :: ");
        for( int m = 0; m < level_Gyro; m++){
         Serial.print(Gyro[k][m]);
         Serial.print(", ");
        }
      Serial.println();
    }

  while( (millis() - t_1) < 3){
    //wait for Baro ADC
  }
  Static_Press = ReadADC();  // finally grab static pressure from baro ADC (typical value 6465444)    

  Serial.print(Static_Temp);
  Serial.print(" : ");
  Serial.println(Static_Press);
  Serial.println(Dyn_Press);
  Serial.println(millis() - t_2);

  Serial.println("***wait***");

  delay(10); // some time to catch up
}

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

void setupL3G4200D(){  
  // 100Hz sample rate, higher bandwidth, enable x, y, z and turn off power down:
  writeRegister(L3G4200D_Address, CTRL_REG1, 0b00011111);
  // no filters, don't know how to use them CTRL_REG2:
  writeRegister(L3G4200D_Address, CTRL_REG2, 0b00000000);
  // no interrupts, set as a push/pull device:
  writeRegister(L3G4200D_Address, CTRL_REG3, 0b00001000);
  // full scale output set to 2000dps, and LSB and MSB registries cont. updated:
  writeRegister(L3G4200D_Address, CTRL_REG4, 0b00110000);
  // no filters or interrupts, but FIFO is enabled
  writeRegister(L3G4200D_Address, CTRL_REG5, 0b01000000);
  // FIFO in stream mode; stack is cont. updated
  writeRegister(L3G4200D_Address, 0x2E, 0b01000000);
}

void updateAccArrays(){
  level_Acc = (readRegister_Int(AXL345_Address, 0x39)) & 0b00111111;
    for(int i = 0; i < level_Acc; i++){
      burstReadAccAxis(i);
    }  
 }  

void burstReadAccAxis(byte i) {
  
  byte LSB;
  byte MSB;

  Wire.beginTransmission(AXL345_Address);
  Wire.write(0x32); // register to read
  Wire.endTransmission();

  Wire.requestFrom(AXL345_Address, (uint8_t)6); // read all registers

    for(int m = 0; m < 3; m++){
      LSB = Wire.read();
      MSB = Wire.read();
      Acc[m][i] = ((MSB << 8) | LSB);
  }
 }

void updateGyroArrays() {
  level_Gyro = (readRegister_Int(L3G4200D_Address, 0x2F)) & 0b00011111;
   for(int i = 0; i < level_Gyro; i++){
     updateGyroAxis(i);
   }
}

void updateGyroAxis(byte i){

  byte LSB;
  byte MSB;
  
  Wire.beginTransmission(L3G4200D_Address);
  Wire.write(0xA8); // same as 0x28 for axis data, but with MSB at logic 1 to enable burst read of all axis
  Wire.endTransmission();

  Wire.requestFrom(L3G4200D_Address, (uint8_t)6); // read a byte
  
   for(int k = 0; k < 3; k++){
     LSB = Wire.read();
     MSB = Wire.read();
     Gyro[k][i] = ((MSB << 8) | LSB);
   }
 }

void writeRegister(int deviceAddress, byte address, byte val) {
    Wire.beginTransmission(deviceAddress); // start transmission to device 
    Wire.write(address);       // send register address
    Wire.write(val);         // send value to write
    Wire.endTransmission();     // end transmission
}

int readRegister_Int(int deviceAddress, byte address){

    int v;
    Wire.beginTransmission(deviceAddress);
    Wire.write(address); // register to read
    Wire.endTransmission();

    Wire.requestFrom(deviceAddress, 1); // read a byte

    while(!Wire.available()) {
        // waiting
    }

    v = Wire.read();
    return v;
}

unsigned int getDynPress(){
  Wire.requestFrom(HSC_Address, static_cast<uint8_t>(2)); //if read 3 or 4 bytes, then can get temp as well
  unsigned int a = Wire.read() << 10; //shifted 10bits then back 2 to get rid of top 2 (or 3?) bits of sensor status
  a = a >> 2;
  a = a | Wire.read(); // add the LSB
  return a; //should be 8192 with no difference in pressure
}

