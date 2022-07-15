//Libraries for GPS
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
// I2C Display
#include "U8glib.h"
U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NONE);

// GPS
TinyGPSPlus gps;    // The TinyGPS++ object
SoftwareSerial ss(8, 9);    // The serial connection to the GPS device. (RX, TX)
float lati = 0, lngi = 0;

// SD Card MODULE
#include <SPI.h>
#include <SD.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  ss.begin(9600);
  //initialize OLED
  u8g.firstPage();
  do {
    draw("Initializing...", "", "", "");
  } while (u8g.nextPage() );
  delay(1000);
  u8g.firstPage();
    do {
      draw("No serial data...", "", "", "");
    } while (u8g.nextPage() );
//    delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()){
    int StringCount = 0;
    String data[10];
    String serial_data = Serial.readString();

    while (serial_data.length() > 0)
    {
      int index = serial_data.indexOf(',');
      if (index == -1) // No , found
      {
        data[StringCount++] = serial_data;
        break;
      }
      else
      {
        data[StringCount++] = serial_data.substring(0, index);
        serial_data = serial_data.substring(index+1);
      }
    }

    if(ss.available() > 0) {
      gps.encode(ss.read());
      if (gps.location.isUpdated()){
        lati = gps.location.lat();
        lngi = gps.location.lng();
      }
    }

    char LCDString1[20] = {0};
    char LCDString2[20] = {0};
    char LCDString3[20] = {0};
    char LCDString4[20] = {0};
    String LCDString = "";

    LCDString = LCDString + lati + "," + lngi;
    LCDString.toCharArray(LCDString1, 20);

    LCDString = "Hum: " + data[1] + " Temp: " + data[2];
    LCDString.toCharArray(LCDString2, 20);
    
    LCDString = "pH: " + data[3] + " TDS: " + data[4];
    LCDString.toCharArray(LCDString3, 20);
    
    LCDString = "TDS(Raw): " + data[5] + " Temp: " + data[6];
    LCDString.toCharArray(LCDString4, 20);
    u8g.firstPage();
    do {
      draw(LCDString1 , LCDString2, LCDString3, LCDString4);
    } while (u8g.nextPage() );
//    u8g.firstPage();
  } 
}

void draw(String s1, String s2, String s3, String s4){
  u8g.setFont(u8g_font_profont12);
  u8g.setPrintPos(0, 10);
  u8g.print(s1);
  u8g.setPrintPos(0, 25);
  u8g.print(s2);
  u8g.setPrintPos(0, 40);
  u8g.print(s3);
  u8g.setPrintPos(0, 55);
  u8g.print(s4);
}
