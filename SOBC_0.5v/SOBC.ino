#include <MKRWAN.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <TinyGPS++.h>
#include <SPI.h>
#include <SD.h>

#define SENSOR_PIN 2
#define GPS_SERIAL Serial1
#define BAND EU868
LoRaModem modem;

// Create instances of necessary libraries
OneWire oneWire(SENSOR_PIN);
DallasTemperature sensors(&oneWire);
TinyGPSPlus gps;

// TTN credentials
const char *devAddr = "YOUR_DEVICE_ADDRESS";
const char *nwkSKey = "YOUR_NETWORK_SESSION_KEY";
const char *appSKey = "YOUR_APP_SESSION_KEY";

// SD card module
const int chipSelect = 4;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  if (!modem.begin(BAND)) {
    Serial.println("LoRa initialization failed!");
    while (1);
  }
  
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed!");
    while (1);
  }

  sensors.begin();
  GPS_SERIAL.begin(9600);
}

void loop() {
  if (GPS_SERIAL.available() > 0) {
    while (GPS_SERIAL.available() > 0) {
      gps.encode(GPS_SERIAL.read());
    }
    if (gps.location.isUpdated()) {
      float lat = gps.location.lat();
      float lon = gps.location.lng();
      float alt = gps.altitude.meters();
      
      sensors.requestTemperatures(); // Request temperature readings
      
      float temp1 = sensors.getTempCByIndex(0);
      float temp2 = sensors.getTempCByIndex(1);

      // Calculate time since started in seconds
      unsigned long currentTime = millis();
      unsigned long timeSinceStarted = currentTime / 1000;

      // Prepare the data for transmission
      String data = String(timeSinceStarted) + "," +
                    String(lat, 6) + "," +
                    String(lon, 6) + "," +
                    String(alt, 2) + "," +
                    String(temp1, 2) + "," +
                    String(temp2, 2) + "," +
                    String(temp1 - temp2, 2); // Difference between two temperature sensors

      // Send data to TTN
      modem.beginPacket();
      modem.write(data.c_str());
      modem.endPacket();

      // Save data to SD card
      File dataFile = SD.open("data.csv", FILE_WRITE);
      if (dataFile) {
        dataFile.println(data);
        dataFile.close();
      }
    }
  }
}
