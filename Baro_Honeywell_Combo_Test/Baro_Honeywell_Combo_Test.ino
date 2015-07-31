#include <Wire.h>
#include <SSC.h>
#include <Servo.h> 

SSC ssc(0x28, 8);

Servo myservo;  // create servo object to control a servo 
                // a maximum of eight servo objects can be created 
 
int pos = 20;    // variable to store the servo position 

const byte CoeffNum = 6;
unsigned long coefficients_[6];    // use long rather than int for calculations
byte i;

byte cmd;
const byte i2cAddr_    = 0x76;       // 7 bit address 0111 0110 since CSB held high
const byte cmdReset_   = 0x1E;
const byte cmdAdcRead_ = 0x00;
const byte cmdAdcConv_ = 0x40;
const byte cmdPromRd_  = 0xA0;

const unsigned int nSamples = 1;

// Define variables for temperature
unsigned long RawTemp;
// Define variables for pressure
unsigned long RawPress;


void setup() 
{
  
  Serial.begin(115200);
  Wire.begin();

  //  set min / max reading and pressure, see datasheet for the values for your sensor
  ssc.setMinRaw(0);
  ssc.setMaxRaw(16383);
  ssc.setMinPressure(0.0);
  ssc.setMaxPressure(1.6);

  //  start the sensor
  Serial.print("start()\t\t");
  Serial.println(ssc.start());
  
  
  myservo.attach(7);  // attaches the servo on pin 9 to the servo object 
  
  Serial.begin(115200);
 // Serial.println(Version);
  Serial.println();
  
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
        Serial.print("Coefficent ");
        Serial.print(i+1);
        Serial.print(" = ");
        Serial.println(coefficients_[i]); 
    }
  Serial.println();
  Serial.print("No of samples = ");
  Serial.println(nSamples);
  Serial.println();
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
    //delay(10);
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

void loop() 
{
    ssc.update(); 
    Serial.println(ReadADC(0x11));   // typical value 8077636 
    Serial.println(ReadADC(0x01));  // typical value 646544
}
