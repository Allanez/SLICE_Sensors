#include <Time.h>
// Libraries for DHT
#include "dht.h"
//Libraries for GPS
//#include <TinyGPS++.h>
//#include <SoftwareSerial.h>
// Temp DS18B20
#include <OneWire.h>
#include <DallasTemperature.h>
// RTC DS1302v2
#include <RTClib.h>
// SD Card MODULE
#include <SPI.h>
#include <SD.h>

// DHT11
#define dht_apin A3
dht DHT;

//#define ONE_WIRE_BUS 6
OneWire oneWire(6); //ONE_WIRE_BUS
DallasTemperature sensors(&oneWire);

DS1302 rtc(3, 5, 4);
// TDS SEN 0224
#define TdsSensorPin A1
//#define VREF 5.0                  // analog reference voltage(Volt) of the ADC
#define SCOUNT  20                // sum of sample point
float averageVoltage = 0,tdsValue = 0,temperature = 25, rawtds = 0;
float pHValue, pHVoltage;
float lati = 0, lngi = 0;

// I2C Display
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x20,16,2);  // set the LCD address to 0x20 for a 16 chars and 2 line display

// PH SEN0169
#define SensorPin A2                //pH meter Analog output to Arduino Analog Input 0
#define Offset 0.00                 //deviation compensate
#define LED 13
#define ArrayLenth  20              //times of collection
int pHArrayIndex=0;

String targetFileName = "";
unsigned long previous = millis();
unsigned long reporting = 0;
unsigned long printing = 0;

int index = 0;

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  //initialize OLED
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print(" Initializing...");
  pinMode(A1 ,INPUT); //TDS_SENSOR_PIN = A1
  pinMode(LED, OUTPUT);
//  delay(3000);

  rtc.begin();
  if(!rtc.isrunning()){
    rtc.adjust(DateTime(__DATE__, __TIME__));
  }

  lcd.setCursor(0,0);
  if(!SD.begin(10)){
    lcd.print("SD Init Failed!");
    while(1);
  }else{
      lcd.print("SD Init Success!");
      sd_init();
  }
//  delay(3000);
}

void sd_init(){
  DateTime now = rtc.now();
  targetFileName = String(now.month())+"-"+String(now.day())+".txt";
  if(!SD.exists(targetFileName)){
    lcd.setCursor(0,0);
    lcd.print("Creating file...");
      
    File myFile = SD.open(targetFileName, FILE_WRITE);
    if (myFile){
      lcd.setCursor(0,0);
      lcd.print("File created!   ");
        
      myFile.println("UNIX,DHT_HUM(%),DHT_TEMP(C),pH,TDS(ppm),TDS_RAW(ppm),TEMP(C)");
      myFile.close();
    } else {
      lcd.setCursor(0,0);
      lcd.print("Err file create! ");
    }
  }
}

void loop () 
{
  unsigned long current = millis();
  unsigned long reading_interval = 1000;
  unsigned long print_interval = 5000;
  
//  if(current-previous >= 1000){
//    previous  = current;
//    char pota[5] = "pota";
//    lcd.clear();
//    lcd.print(pota);
//    Serial.println(pota);
//  }

  if(current - reporting >= reading_interval){
    reporting = current;
//    lcd.clear();
    switch(index){
      case 0: 
        temp_class();
        break;
      case 1:
        tds_class();
        break;
      case 2:
        ph_class();
        break;
      default:
        break; 
    }
    index = ((index+1)%3);
  }

  if(current - printing >= print_interval){
    printing = current;
    print_f();
//    lcd.setCursor(0,0);
//    lcd.print("print na this   ");
  }
}


//void writeReadingsToFile(){
//    String SDString = "";
//    DHT.read11(dht_apin);
//
//    DateTime now = rtc.now();
//    SDString = SDString + now.unixtime()+ ',' + DHT.humidity + ',' + DHT.temperature + ',' +pHValue + ',' + tdsValue + ',' + rawtds + ',' + temperature;
////    Serial.println(SDString);
//    
//    File myFile = SD.open(targetFileName, FILE_WRITE);
//    if(myFile){
//      lcd.setCursor(0,0);
//      lcd.print("Writing data... ");
//      myFile.println(SDString);
//    } else {
//      lcd.setCursor(0,0);
//      lcd.print("Err data write! ");
//    }
//    myFile.close();
//    delay(1000);
//}

void send_serial(String SDString){
//  Serial.println(SDString);
  Serial.print(SDString);
}

void temp_class(){
  Serial.println("TEMP");
//  char reading[13] = "Reading temp";
//  lcd.setCursor(0,0);
//  lcd.print(reading);
//  lcd.setCursor(0,1);
//  lcd.print("Temperature...  ");
  sensors.requestTemperatures(); 
  temperature=sensors.getTempCByIndex(0);
}

void tds_class(){
    Serial.println("TDS");

//  char reading[13] = "Reading tds ";
//  lcd.setCursor(0,0);
//  lcd.print(reading);
  int analogBuffer[ArrayLenth]; 
  for(int i = 0; i < 20; i++){
    analogBuffer[i] = analogRead(A1);
//    delay(40);
  }
  averageVoltage = getMedianNum(analogBuffer,SCOUNT) * (float)5.0 / 1024.0;  // read the analog value more stable by the median filtering algorithm, and convert to voltage value/ 5.0 = VREF
  float compensationCoefficient=1.0+0.02*(temperature-25.0);                      //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
  float compensationVoltage=averageVoltage/compensationCoefficient;               //temperature compensation
  tdsValue=(133.42*compensationVoltage*compensationVoltage*compensationVoltage - 255.86*compensationVoltage*compensationVoltage + 857.39*compensationVoltage)*0.5; //convert voltage value to tds value

  compensationCoefficient=1.0+0.02*(25.0-25.0);                      //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
  compensationVoltage=averageVoltage/compensationCoefficient;               //temperature compensation
  rawtds=(133.42*compensationVoltage*compensationVoltage*compensationVoltage - 255.86*compensationVoltage*compensationVoltage + 857.39*compensationVoltage)*0.5; //convert voltage value to tds value
}

void ph_class(){
    Serial.println("PH");

//  char reading[13] = "Reading ph  ";
//  lcd.setCursor(0,0);
//  lcd.print(reading);
  int pHArray[ArrayLenth];
  for(int i = 0; i < ArrayLenth; i++){
    pHArray[pHArrayIndex++] = analogRead(SensorPin);
//    delay(40);
  }
  pHVoltage = avergearray(pHArray, ArrayLenth)*5.0/1024;
  pHValue = 3.5*pHVoltage+Offset;
}

int getMedianNum(int bArray[], int iFilterLen) 
{
      int bTab[iFilterLen];
      for (byte i = 0; i<iFilterLen; i++)
      bTab[i] = bArray[i];
      int i, j, bTemp;
      for (j = 0; j < iFilterLen - 1; j++) 
      {
      for (i = 0; i < iFilterLen - j - 1; i++) 
          {
        if (bTab[i] > bTab[i + 1]) 
            {
        bTemp = bTab[i];
            bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
         }
      }
      }
      if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
      else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
      return bTemp;
}

double avergearray(int* arr, int number){
  int i;
  int max,min;
  double avg;
  long amount=0;
  if(number<=0){
//    Serial.println("Error number for the array to avraging!/n");
    return 0;
  }
  if(number<5){   //less than 5, calculated directly statistics
    for(i=0;i<number;i++){
      amount+=arr[i];
    }
    avg = amount/number;
    return avg;
  }else{
    if(arr[0]<arr[1]){
      min = arr[0];max=arr[1];
    }
    else{
      min=arr[1];max=arr[0];
    }
    for(i=2;i<number;i++){
      if(arr[i]<min){
        amount+=min;        //arr<min
        min=arr[i];
      }else {
        if(arr[i]>max){
          amount+=max;    //arr>max
          max=arr[i];
        }else{
          amount+=arr[i]; //min<=arr<=max
        }
      }//if
    }//for
    avg = (double)amount/(number-2);
  }//if
  return avg;
}

void print_f(){
  char LCDString1[16] = {0};
  String LCDString = "";

  DHT.read11(dht_apin);
      
  lcd.clear();
  lcd.setCursor(0,0);
  LCDString = LCDString + DHT.temperature + ":"+ DHT.humidity +":"+pHValue;
//  LCDString = LCDString.substring(0, 16);
  LCDString.toCharArray(LCDString1, 16);
  lcd.print(LCDString1);
  
  LCDString = "";
  LCDString = LCDString + tdsValue +":"+pHVoltage+":"+temperature;
//  LCDString = LCDString.substring(0, 16);
  LCDString.toCharArray(LCDString1, 16);
  lcd.setCursor(0,1);
  lcd.print(LCDString1);
}
