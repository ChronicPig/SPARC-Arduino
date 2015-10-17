void serialPrint(int x)
{
  if(x == 1)
   {
    Serial.print("Lat: ");
    Serial.println(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
    Serial.print("Lon: ");
    Serial.println(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);    
   }
    Serial.println(sensorData);
}

void microPrint(int x)
{
  if(x == 1)
  {
    microSerial.print("Lat: ");
    microSerial.println(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
    microSerial.print("Lon: ");
    microSerial.println(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6); 
  }
    microSerial.println(sensorData);
}

void spiPrint()
{
  SpiSerial.print("<gps>");
  SpiSerial.print("Rocket");
  SpiSerial.print(", Lat: ");
  SpiSerial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
  SpiSerial.print(" Lon: ");
  SpiSerial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
  SpiSerial.print("</gps>");
  SpiSerial.println(sensorData);
}


