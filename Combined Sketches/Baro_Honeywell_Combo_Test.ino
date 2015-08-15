#include <Wire.h>
#include <SSC.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#define CTRL_REG1 0x20
#define CTRL_REG2 0x21
#define CTRL_REG3 0x22
#define CTRL_REG4 0x23

int Addr = 105;                 // I2C address of gyro
int x, y, z;

SSC ssc(0x28, 8);
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

const byte CoeffNum = 6;
unsigned long coefficients_[6];    // use long rather than int for calculations
byte i;

byte cmd;
const byte i2cAddr_    = 0x76;       // 7 bit address 0111 0110 since CSB held high
const byte cmdReset_   = 0x1E;
const byte cmdAdcRead_ = 0x00;
const byte cmdAdcConv_ = 0x40;
const byte cmdPromRd_  = 0xA0;

// Define variables for temperature
unsigned long RawTemp;
// Define variables for pressure
unsigned long RawPress;


void setup() 
{
  
  Serial.begin(115200);

  //  set min / max reading and pressure, see datasheet for the values for your sensor
  ssc.setMinRaw(0);
  ssc.setMaxRaw(16383);
  ssc.setMinPressure(0.0);
  ssc.setMaxPressure(1.6);
  accel.begin();
  accel.setRange(ADXL345_RANGE_16_G);
  sensor_t sensor;
  accel.getSensor(&sensor);
  writeI2C(CTRL_REG1, 0x1F);    // Turn on all axes, disable power down
  writeI2C(CTRL_REG3, 0x08);    // Enable control ready signal
  writeI2C(CTRL_REG4, 0x80);    // Set scale (500 deg/sec)
  delay(100);                   // Wait to synchronize 
  

  //  start the sensor
  Serial.print("start()\t\t");
  Serial.println(ssc.start());
  
  // Reset MS5607 Sensor
  Wire.begin();
  Wire.beginTransmission(i2cAddr_);
  Wire.write(cmdReset_);   
  Wire.endTransmission(); 
  delay(3);

  // Read MS5607 Coefficients
  for(i=0; i<6; ++i)
    {
        coefficients_[i] = ReadCoefficient(i + 1); 
        Serial.println(coefficients_[i]); 
    }
}

// Read Coefficients subroutine
    unsigned int ReadCoefficient(const uint8_t coefNum)
    {
        unsigned int rC=0;
        Wire.beginTransmission(i2cAddr_);
        Wire.write(cmdPromRd_ + coefNum * 2); // send PROM READ command
        Wire.endTransmission(); 
        Wire.requestFrom(i2cAddr_, static_cast<uint8_t>(2));
        if(Wire.available() >= 2)
        {
            unsigned int ret = Wire.read();   // read MSB and acknowledge
            unsigned int rC  = 256 * ret;
            ret = Wire.read();        // read LSB and not acknowledge
            rC  = rC + ret;
            return rC;
        } 
        return 0;
    }

// ReadADC subroutine
long ReadADC(byte cmd)
  {
    Wire.beginTransmission(i2cAddr_); 
    Wire.write(cmdAdcConv_ | cmd);   // send convert command
    Wire.endTransmission();
    // wait for conversion
    DoInDelay();
   
    Wire.beginTransmission(i2cAddr_);
    Wire.write(cmdAdcRead_);
    Wire.endTransmission();
    const byte quantity = 3;
    Wire.requestFrom(i2cAddr_, quantity);
    if (Wire.available() >= 3)
      {
        unsigned int ret = Wire.read();  //read MSB
        unsigned long temp = 65536 * ret;
        ret = Wire.read();               // read byte
        temp = temp + 256 * ret;
        ret = Wire.read();               // read LSB
        temp = temp + ret;
        return temp;
      }
      return 0;
  }

void DoInDelay() {
  for(int i = 0; i < 2; i++) {
    Serial.print("pressure: ");
    Serial.println(ssc.pressure());
  }
}

void getGyroValues () {
  byte MSB, LSB;

  MSB = readI2C(0x29);
  LSB = readI2C(0x28);
  x = ((MSB << 8) | LSB);

  MSB = readI2C(0x2B);
  LSB = readI2C(0x2A);
  y = ((MSB << 8) | LSB);

  MSB = readI2C(0x2D);
  LSB = readI2C(0x2C);
  z = ((MSB << 8) | LSB);
}

int readI2C (byte regAddr) {
    Wire.beginTransmission(Addr);
    Wire.write(regAddr);                // Register address to read
    Wire.endTransmission();             // Terminate request
    Wire.requestFrom(Addr, 1);          // Read a byte
    while(!Wire.available()) { };       // Wait for receipt
    return(Wire.read());                // Get result
}

void writeI2C (byte regAddr, byte val) {
    Wire.beginTransmission(Addr);
    Wire.write(regAddr);
    Wire.write(val);
    Wire.endTransmission();
}

void loop() 
{
    sensors_event_t event; 
    accel.getEvent(&event);
    Serial.print("X: "); Serial.print(event.acceleration.x); Serial.print("   ");
    Serial.print("Y: "); Serial.print(event.acceleration.y); Serial.print("   ");
    Serial.print("Z: "); Serial.print(event.acceleration.z); Serial.print("   ");
    Serial.println();
    
    getGyroValues();
    Serial.print("Raw X:");  Serial.print(x/114);  
    Serial.print(" Raw Y:"); Serial.print(y/114);
    Serial.print(" Raw Z:"); Serial.println(z/114);
    
    ssc.update(); 
    Serial.println(ReadADC(0x11));   // typical value 8077636 
    Serial.println(ReadADC(0x01));  // typical value 646544
    Serial.println();
}
