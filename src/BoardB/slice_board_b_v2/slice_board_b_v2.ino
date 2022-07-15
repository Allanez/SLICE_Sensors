//Libraries for GPS
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
static const int RXPin = 8, TXPin = 9;
static const uint32_t GPSBaud = 9600;
// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

// SD Card MODULE
#include <SPI.h>
#include <SD.h>

// I2C Display
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x20,16,2);  // set the LCD address to 0x20 for a 16 chars and 2 line display

float lati = 0, lngi = 0; 

char fname[12]="data.csv";
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  ss.begin(GPSBaud);
//  gps.begin(9600);
  lcd.init();
  lcd.backlight();
//  lcd.print("Initializing... ");
  
  
  while(!SD.begin(10)){
//    lcd.print("SD Init Failed!");
//    while(1);
  }


  
//  if(!SD.exists(fname)){
//      File myFile = SD.open(fname, FILE_WRITE);
//      
//      if(myFile){
////        char header[69] = "UNIX,DHT_HUM(%),DHT_TEMP(C),pH,TDS(ppm),TDS_RAW(ppm),TEMP(C),LAT,LNG";
////        lcd.print("File created!   ");
////        myFile.println("UNIX,DHT_HUM(%),DHT_TEMP(C),pH,TDS(ppm),TDS_RAW(ppm),TEMP(C),LAT,LNG");
////        myFile.println(header);
//      } else {
////        lcd.println("Err file create!");
//      }
//      myFile.close();
////      delay(2000);
//    }
  delay(1000);
  
}

void loop() {
//  lcd.clear();

  lcd.setCursor(0,0);
    while(ss.available() > 0) {
      gps.encode(ss.read());
      if (gps.location.isUpdated()){
        lati = gps.location.lat();
        lngi = gps.location.lng();
        Serial.print(gps.location.lat(), 6);
//      Serial.print(" Longitude= "); 
        Serial.println(gps.location.lng(), 6);
      }
    }
//      Serial.println(serial_data);
//
  lcd.print(lati);
  lcd.setCursor(8, 0);
  lcd.print(lngi);

  char waiting[17] =  "    WAITING     ";
  lcd.setCursor(0,1);
  lcd.print(waiting);
  
  // put your main code here, to run repeatedly:
  char received[17] = "    RECEIVED    ";
  lcd.setCursor(0,1);
  if(Serial.available()){
     String serial_data = Serial.readString();
      lcd.setCursor(0,1);
      lcd.print(received);
//      char 
      
      

      { // write received data to file
      File myFile = SD.open(fname, FILE_WRITE);
      if(myFile){
//        char sdwrite[17] = "WRITING";  
//        lcd.print("Writing data... ");
        myFile.print(serial_data);
        myFile.print(',');
        myFile.print(lati, 6);
        myFile.print(',');
        myFile.println(lngi, 6);
      }
      myFile.close();
      }
  }
} 
//  else {
//
//  }
//  else {
////      char wait = 
////      lcd.print("  Waiting for   ");
////      lcd.setCursor(0,1);
////      lcd.print(" serial data... ");
//  //    delay(5000);
//  }
//}

//String getSub(String s){
//  String ret_s = "";
//  int index = s.indexOf('\n');
//  ret_s = s.substring(index + 1, s.length() );
//  return ret_s;
