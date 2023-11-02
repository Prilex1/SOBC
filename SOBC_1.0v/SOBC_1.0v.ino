#include <Arduino_MKRGPS.h> 
#include <OneWire.h> 
#include <DallasTemperature.h> 
#include <MKRWAN.h> 
#include <TimeLib.h>

// SENSOR EN BRIDA FORA DE LA CAPSULA

#define MAX_TRAMA 14 //tamaño del buffer de datos

LoRaModem modem; 

int connected;

int segundo = 0; //segundo del minuto en que queremos que emita un dato

static uint8_t  datos[MAX_TRAMA]= { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

OneWire  ds(2); 
DallasTemperature sensorDS18B20(&ds); 
int temp1; 
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
 
  Serial.println("Initialized GPS!"); 

 
} 
 
void loop() { 
 
  if (GPS.available()) {

    unsigned long epochTime = GPS.getTime();
    time_t t = epochTime;

    if (second(t)  == segundo)
      {

         sensorDS18B20.requestTemperatures(); 
         temp1 = sensorDS18B20.getTempCByIndex(0); 
         temp2 = sensorDS18B20.getTempCByIndex(1); 

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
 
 
 
    Serial.println("emitimos a TTN"); 
 
    if (connected){    
      modem.beginPacket();   
      modem.write(datos,MAX_TRAMA);
      modem.endPacket();
    } 
    delay(1200); 
    
    }
  } 
}
