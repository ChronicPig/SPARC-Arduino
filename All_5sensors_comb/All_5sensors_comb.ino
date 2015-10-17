/*
all sensors work, just need to make it so sensors without FiFo offload on time schedual.
rework main loop to be more time efficient and redo timers to make sure everything is sampled at 100Hz.
only thing outside of loop is the Axis updates, may be able to merge into one method for efficiency.
barely gets all measurments out in 10ms (100hz).
*/
unsigned int t_1 = 0;
unsigned int t_2 = 0;

#include <SoftwareSerial.h>
#include <Wire.h>
#include <WiFly.h>
#include <SPI.h>
#include <TinyGPS.h>

const byte ADC_ADDR = 0x48; //I2C address of 4 channel ADC from Adafruit
const byte MS5607_Address = 0x76; // I2C address of MS5607 barometer (static pressure sensor)
const byte AXL345_Address = 0x53; //I2C address of the AXL345 (accelerometer)
const byte L3G4200D_Address = 0x69; //I2C address of the L3G4200D (gyroscope)
const byte HSC_Address = 0x28; //I2C address of honeywell diff. pressure sensor (dynamic pressure)

#define CTRL_REG1 0x20
#define CTRL_REG2 0x21
#define CTRL_REG3 0x22
#define CTRL_REG4 0x23
#define CTRL_REG5 0x24

#define microRX 5
#define microTX 4
#define gpsRX 2
#define gpsTX 3

TinyGPS gps;
SoftwareSerial microSerial = SoftwareSerial(microRX, microTX);
SoftwareSerial gpsSerial = SoftwareSerial(gpsRX, gpsTX);

int x = 100;
String sensorData;

//variables used to hold tinygps data
float flat, flon;
unsigned long age;

unsigned long coefficients_[6]; // barometer coefficients for later calculations

// buffers for Acc and Gyro FIFOs to dump into
int Acc[3];

// used to hold how many samples in Acc and Gyro FIFOs
byte level_Acc;

// Define variables for static temperature and static pressure respectively (24bit)
unsigned long Static_Temp;
unsigned long Static_Press;

// Define var for diff pressure sensor
unsigned int Dyn_Press;

//Used for holding the most recent value from the acceleromteter in the vertical axis (-x)
int verticalAcceleration;

void setup() 
{ 
  microSerial.begin(115200);
  WiFly.begin();
  Wire.begin();
  SpiSerial.begin();
  gpsSerial.begin(9600);
  TWBR = 24; // for 400kHz I2C
  Serial.begin(115200);
  if(WiFly.join("Maleta"))
  {
    Serial.println("Connection Successful");
  }
  else
  {
    Serial.println("Connection Failed");
  }

  setADC(0, 0);

  setupAXL345(); // write to registries as needed. just edit registry entries
  //setupL3G4200D(); // write to registries as needed. just edit registry entries
  
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
  verticalAcceleration = Acc[1];
  delay(4);

  while( (millis() - t_1) < 3){
    //wait for Baro ADC
  }
  Static_Temp = ReadADC();  // fianlly grab temp from baro ADC (typical value 8077636)
   
  lookAtRegister(0x44);  // tell baro to start getting static pressure ADC value
  t_1 = millis();  //mark time of command

  delay(4);
  
  while( (millis() - t_1) < 3){
    //wait for Baro ADC
  }
  Static_Press = ReadADC();  // finally grab static pressure from baro ADC (typical value 6465444)

  sensorData = "";
  sensorData += "T: "; sensorData += Static_Temp; sensorData += " ";
  sensorData += "SP: "; sensorData += Static_Press; sensorData += " ";
  sensorData += "DP: "; sensorData += Dyn_Press; sensorData += " ";
  sensorData += "A: "; sensorData += verticalAcceleration; sensorData += " ";
  sensorData += "ADC: "; sensorData += Static_Press;
  
  if(x == 100)
  {
    x = 0;
    while(gpsSerial.available())
    {
      char c = gpsSerial.read();
      if(gps.encode(c))
      {
        gps.f_get_position(&flat, &flon, &age);
      }
    }
    serialPrint(1);
    microPrint(1);
    spiPrint();       
  }
  else
  {
    serialPrint(0);
    microPrint(0);   
  }

  x += 1;
  delay(10); // some time to catch up
}

