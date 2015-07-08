/*
  SD card datalogger
 
 This example shows how to log data from three analog sensors to an SD card using the SD library.
 	
 The circuit:
 * analog sensors on analog ins 0, 1, and 2
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 10
 
 adapted from SD_Datalogger by Tom Igoe
 This example code is in the public domain.
 	 
*/
#include <SPI.h>       //with mega must define SPI pins
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
#include <Wire.h>;
#include <SSC.h>;
#define DEBUG

//Altimeter doesn't use libraries, all setup is right here////////////////////////////////////////////////////////////////////////////////////////
const byte CoeffNum = 6;
unsigned long coefficients_[6];    // use long rather than int for calculations
byte i;

byte cmd;
const byte i2cAddr_    = 0x76;       // 7 bit address 0111 0110 since CSB held high
const byte cmdReset_   = 0x1E;
const byte cmdAdcRead_ = 0x00;
const byte cmdAdcConv_ = 0x40;
// const byte cmdAdcD1_   = 0x00;
// const byte cmdAdcD2_   = 0x10;
// const byte cmdAdc256_  = 0x00;
// const byte cmdAdc512_  = 0x02;
// const byte cmdAdc1024_ = 0x04;
// const byte cmdAdc2048_ = 0x06;
// const byte cmdAdc4096_ = 0x08;
const byte cmdPromRd_  = 0xA0;

const unsigned int nSamples = 2;

// Define variables for temperature
unsigned long RawTemp;
long DeltaTemp;
float CalTemp;
float TempAccum;
float Temp;

// Define variables for pressure
unsigned long RawPress;
float P1;
float P2;
float P3;
float P4;
float PressAccum;
float Press;
const float sea_press = 1013.25;
float alt;
const byte MeterPin = 6;                       // analog meter attached
float PanelMeter;
//END OF ALTIMETER SETUP///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


SSC ssc(0x28);
RTC_DS1307 rtc;

// NOTE:  You have two options in this sketch
// IF interpreted is true the time and analog inputs are interpreted in seconds and volts, respectively instead of just number
// interval allows you to control the number of milliseconds between each time the A/Ds are read and written and sent out the serial port
#define interpreted true          // Set true to printout human readable time and voltages, set false for data only
#define interval 40             // can run with DEBUG still at 25Hz, 25Hz is recommended max speed, else SD card cannot keep up
//#define DEBUG                 //without it only get bare bones

// On the Sparkfun shield CS is pin 8.  Note that even if it's not used as the CS pin, this pin must be left as an output
// or the SD library functions will not work.

const int chipSelect = 10;            // Chip select is tied to pin 8 on the SparkFun SD Card Shield
int error = 0;                       // if error = 0 there is no error, error = 1 card not present, if error = 2 error opening file
long lastTime = 0;                   // we are looking for the moment the time rolls over from max to zero - this is our last time reading
long thisTime = 0;                   // and this is the current time reading

void setup()
{
  Serial.begin(115200);                         // Open serial communications at 9600 BAUD
  Wire.begin();
  rtc.begin();
  
  ssc.setMinRaw(0);
  ssc.setMaxRaw(16383);
  ssc.setMinPressure(-5);
  ssc.setMaxPressure(5);

  ssc.start();
//FOR ALTIMETER///////////////////////////////////////////////////////////////////////////////////////////////////////  
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
    delay(10);
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
//ALTIMETER STUFF END///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  String dataString = "";                          // create a string variable to form the data to log
#ifdef DEBUG
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  Serial.print("Initializing SD card...");    // Tell the world what we are doing
#endif

  pinMode(chipSelect, OUTPUT);                // we need the default chil select pin set to output even if not used
  
#ifdef DEBUG
  if (!SD.begin(10, 11, 12, 13)) {                      // check to see if the card is present and can be initiatlized
    Serial.println("Card failed, or not present");
    error = 1;                                      // we have a card error so set error = 1 (failed or not present)
    return(error);                                         // we have a problem - no need to do anything else
  }
  Serial.println("card initialized.");              // Otherwise, tell the world we were successful
#endif

  //SD.begin(10, 11, 12, 13);

  File dataFile = SD.open("datalog.txt", FILE_WRITE);  // open the file for writing
   if (dataFile)                                        // make certain we have a file to write to
  {  
    DateTime now = rtc.now();
    dataString += '\r';                                    // we are done building our string - add CR and LF to end the line
    dataString += '\n';
    dataString += ("Begin Data Capture: ");
    dataString += String(now.year(), DEC);
    dataString += ('/');
    dataString += String(now.month(), DEC);
    dataString += ('/');
    dataString += String(now.day(), DEC);
    dataString += '\r';                                    // we are done building our string - add CR and LF to end the line
    dataString += '\n';
    dataFile.print(dataString);                            // write to the SD Card
    Serial.print(dataString);                              // write to the serial port
    dataFile.close();                                      // close the file so the write is permanent
  }
}

void loop()
{
  Serial.println("void loop reached") //////////////////////////////////////////////only for DEBUG purposes
  
  String dataString = "";                          // create a string variable to form the data to log
  
  
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  // this opens the file and appends to the end of file
  // if the file does not exist, this will create a new file.
  do  {
    lastTime = thisTime;                            // keep track of the time from last trip through
    thisTime = (millis() % interval);               // get a new time from this trip through
  }  while (lastTime <= thisTime);                  // watch for the moment in time that we roll over

  File dataFile = SD.open("datalog.txt", FILE_WRITE);  // open the file for writing

#ifdef DEBUG
  if (dataFile)                                        // make certain we have a file to write to
  {  
#endif
    DateTime now = rtc.now();
    dataString += String(now.hour(), DEC);
    dataString += (':');
    dataString += String(now.minute(), DEC);
    dataString += (':');
    dataString += String(now.second(), DEC);

    dataString += ",\t";                          // add the comma separator
    
    //read static pressure (avg of nSamples)
    PressAccum = 0;
    
    for(i=0; i<nSamples; i++) {
    
    // read temperature
    RawTemp = ReadADC(0x18);   // typical value 8077636 
    DeltaTemp = RawTemp - coefficients_[4] * 256;  
     
    // read pressure
    RawPress = ReadADC(0x08);  // typical value 6465444
    P1 = (float(RawPress)/1024L)*(float(coefficients_[0])/1024L);
    P2 = (float(RawPress)/16384L)*(float(coefficients_[2])/16384L)*(float(DeltaTemp)/32768L);
    P3 = (float(coefficients_[1])*4);
    P4 = (float(coefficients_[3])/1024l)*(float(DeltaTemp)/2048L);
    PressAccum += P1 + P2 - P3 - P4;
    }
    
    float staticPress = PressAccum/nSamples/100;
    //end of parallax static pressure calculation, in millibars
    
    ssc.update();
    
    float ratio = (ssc.pressure()*680.97)/staticPress;  //should be *68.97
    float Machx1000 = 1000*sqrt(5*pow((ratio + 1) , 2.0/7.0) - 5);
    
    dataString += (long)(ssc.pressure()*1000); //SHOULD BE *100
    dataString += "\t";
    dataString += (long)(ssc.temperature()*100);
    dataString += " \t";
    dataString += (long)(staticPress*1.45);
    dataString += " \t";
    dataString += (long)(Machx1000);
    dataString += "";
    
    dataString += '\r';                                    // we are done building our string - add CR and LF to end the line
    dataString += '\n';
    dataFile.print(dataString);                            // write to the SD Card
    Serial.print(dataString);                              // write to the serial port
    dataFile.close();    // close the file so the write is permanent
  #ifdef DEBUG
  }
 
  // if the file isn't open, pop up an error:
  else                                                     // so if we we did not do the above, we must have had an error
  {
    if (error == 0) {                                      // we want to do this only once - that is the case where we have not previously detected errors
      Serial.println("error opening datalog.txt");         // tell the world of our error
      error = 2;                                           // flag which error we had
    }
  }
 #endif 
}
/*as of the night of Sept. 23-   need to check math for Mach number
                             -   delete serial print of calculated Mach number
                             -   use an avg of static pressure instead of instantaneos
                             -   get mach # to something documentable in "long" data type   
*/









