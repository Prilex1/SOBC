#include <Arduino_MKRGPS.h> 
#include <OneWire.h> 
#include <DallasTemperature.h> 
#include <MKRWAN.h> 
#include <TimeLib.h>

/*
    SOBC v1 - by Josep Curto Lapeira - Launched at the Servet IX weather balloon

    The goal with this project is to make an open-source, OnBoard Computer for the weather ballon launches.
    I'm finishing my first version and it will be working at a 100% by November. Feel free to download the 
    code and finding bugs or making it better, I'm all ears.

    IMPORTANT:

    The code required 5 libraries for it to work properly (Arduino_MKRGPS, OneWire, DallasTemperature, MKRWAN, Timelib).
    The two DS18B20 sensors at attached at the digital pin 2 and the GPS (MKRGPS) to the i2c connector on the MKR WAN 1310 board.
 */

#define MAX_TRAMA 14 //Indicates the quantity of bytes it will send

LoRaModem modem; 

int connected;

int segundo = 0; //Second of the minute it will send data. It will skip some minutes to follow EU laws.

static uint8_t  datos[MAX_TRAMA]= { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
String appEui = ""; // Put the appEui from TTN
String appKey = ""; // Put the appKey from TTN
 
OneWire  ds(2); // Pin of the temperature sensors
DallasTemperature sensorDS18B20(&ds); 
int temp1;  // Variables for temperature and GPS data
int temp2; 
float flat; 
float flon; 
float falt; 
unsigned int altitud;
 
 
void setup() { 
  Serial.begin(9600); 
 
  sensorDS18B20.begin();  
 
  if (!modem.begin(EU868)) { 
    Serial.println("Failed to start module"); 
    while (1) {} 
  }; 
   
   connected = modem.joinOTAA(appEui, appKey); 
   if (!connected) { 
    Serial.println("Something went wrong; are you indoor? Move near a window and retry"); 
    while (1) {} 
  } 

   if (connected){    
      modem.beginPacket();   
      modem.write(datos,MAX_TRAMA); 
  }
 
  if (!GPS.begin()) { 
    Serial.println("Failed to initialize GPS!"); 
    while (1); 
  } 
  
  airborne6(); // Sets the GPS mode to Airbone to avoid problems with high altitudes. If an error pops out, you will need to modify the MKRGPS library, 
               // if you don't know how to do that, contact me via Twitter (@JosepCurto3).

  Serial.println("SOBC - Started"); // If this appears, the code is working correctly
} 
 
void loop() { 
 
  if (GPS.available()) {

    unsigned long epochTime = GPS.getTime();
    time_t t = epochTime;

    if (second(t)  == segundo)
      {

         sensorDS18B20.requestTemperatures(); 
         temp1 = sensorDS18B20.getTempCByIndex(0) + 55; // Avoids problems with negative numbers, set 65 to avoid erros with extreme readings (REMBER THE NUMBER YOU PUT (IF YOU CHANGE IT))
         temp2 = sensorDS18B20.getTempCByIndex(1) + 55; 

         Serial.println(temp1);
         Serial.println(temp2);
        
         flat   = GPS.latitude(); 
         flon  = GPS.longitude(); 
         falt   = GPS.altitude(); 
 
         altitud = falt; 
         unsigned long latitud,longitud; 
         latitud = flat * 10000000; // multiply by 10 million to convert float to integer  
         datos[0] = (byte) ((latitud & 0xFF000000) >> 24 ); 
         datos[1] = (byte) ((latitud & 0x00FF0000) >> 16 );  
         datos[2] = (byte) ((latitud & 0x0000FF00) >> 8 ); 
         datos[3] = (byte) ((latitud & 0x000000FF)); 
  
         longitud = flon * 10000000; // the same as before  datos[4] = (byte) ((longitud & 0xFF000000) >> 24 ); 
         datos[5] = (byte) ((longitud & 0x00FF0000) >> 16 );  datos[6] = (byte) ((longitud & 0x0000FF00) >> 8 ); 
         datos[7] = (byte) ((longitud & 0x000000FF)); 
         datos[8] = (byte) ((altitud & 0xFF00) >> 8 );  
         datos[9] = (byte) ((altitud & 0x00FF)); 

         datos[10] = (byte) ((temp2 & 0xFF00) >> 8 );
         datos[11] = (byte) ((temp2 & 0x00FF));

         datos[12] = (byte) ((temp1 & 0xFF00) >> 8 );
         datos[13] = (byte) ((temp1 & 0x00FF));
 
 
 
    Serial.println("Sending to TTN"); 
 
    if (connected){    
      modem.beginPacket();   
      modem.write(datos,MAX_TRAMA); // Data is sended to TTN
      modem.endPacket();
    } 
    delay(1200); 
    
    }
  } 
}

void airborne6 (void){ // Sets Airbone mode.

Serial.println("Cambiando a Airborne6");

uint8_t payload[] = { // UBX-CFG-NAV5 (0x06 0x24)
 //  para payload ver UBX-13003221 - R17 section 32.10.15.1 Navigation Engine Settings

//0     1     2     3     4    5     6     7     8     9     10    11 
0xFF, 0xFF, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00,
0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

GPS.sendUbx(0x06,0x24, payload,sizeof(payload));

  Serial.println("  > done");
}
