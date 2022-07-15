#include <Time.h>
// Libraries for DHT
#include "dht.h"
//Libraries for GPS
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
// Temp DS18B20
#include <OneWire.h>
#include <DallasTemperature.h>
// RTC DS1302v2
#include <RTClib.h>
// SD Card MODULE
#include <SPI.h>
#include <SD.h>

// DHT11
//#define dht_apin A3
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
#include "U8glib.h"
U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NONE);

// PH SEN0169
#define SensorPin A2                //pH meter Analog output to Arduino Analog Input 0
#define Offset 0.00                 //deviation compensate
#define LED 13
#define ArrayLenth  20              //times of collection
int pHArrayIndex=0;

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  //initialize OLED
  u8g.firstPage();
  do {
    draw("Initializing...", "", "");
  } while (u8g.nextPage() );
  
  pinMode(A1 ,INPUT); //TDS_SENSOR_PIN = A1
  pinMode(LED, OUTPUT);
  delay(3000);

  rtc.begin();
  if(!rtc.isrunning()){
    rtc.adjust(DateTime(__DATE__, __TIME__));
  }
  delay(1000);
}

void draw(String s1, String s2, String s3){
  u8g.setFont(u8g_font_profont12);
  u8g.setPrintPos(0, 10);
  u8g.print(s1);
  u8g.setPrintPos(0, 25);
  u8g.print(s2);
  u8g.setPrintPos(0, 40);
  u8g.print(s3);
}

void loop () 
{
    String targetFileName = "";
    String SDString = "";
    
    temp_class();
    delay(1000);
    tds_class();
    delay(1000);
//    turb_class();
//    delay(1000);
    ph_class();
    delay(1000);
    
    DateTime now = rtc.now();
//    targetFileName = String(now.month())+"-"+String(now.day())+".txt";
    targetFileName = "test.txt";  
    if(!SD.exists(targetFileName)){
      Serial.println("Creating file...");
      File myFile = SD.open(targetFileName, FILE_WRITE);
      if (myFile){
        Serial.println("File created!");
        myFile.println("UNIX, LAT, LNG, DHT_HUM(%), DHT_TEMP(C), pH, TDS(ppm), TDS_RAW(ppm), TEMP(C)");
        myFile.close();
      } else {
        Serial.println("File not created!");
      }
//      while(!myFile){}
      
    } else {
      SDString = SDString + now.unixtime()+ lati + ',' + lngi + ',' + DHT.humidity + ',' + DHT.temperature + ',' +pHValue + ',' + tdsValue + ',' + rawtds + ',' + temperature;
      File myFile = SD.open(targetFileName, FILE_WRITE);
      if(myFile){
        Serial.println("Opening file...");
//        while(!myFile){}
        Serial.println("Writing to file...");
        myFile.print(now.unixtime() + ',');
        myFile.print(lati + ',');
        myFile.print(lngi + ',');
        myFile.print(DHT.humidity + ',');
        myFile.print(DHT.temperature + ',');
        myFile.print(pHValue + ',');
        myFile.print(tdsValue + ',');
        myFile.print(rawtds + ',');
        myFile.println(temperature);
        myFile.close();
      }
    }
    print_f(lati, lngi);
}

void print_f(float lati, float lngi){
  DateTime now = rtc.now();
//  String SDString = SDString + now.unixtime()+ lati + ',' + lngi + ',' + DHT.humidity + ',' + DHT.temperature + ',' +pHValue + ',' + tdsValue + ',' + rawtds + ',' + temperature;
  Serial.println(now.unixtime());
  Serial.println(lati);
  Serial.println(lngi);
//  do {
//    draw("Yawa", "", "");
//  } while (u8g.nextPage() );
}
void temp_class(){
  sensors.requestTemperatures(); 
  temperature=sensors.getTempCByIndex(0);
}

void tds_class(){
  int analogBuffer[ArrayLenth]; 
  for(int i = 0; i < 20; i++){
    analogBuffer[i] = analogRead(A1);
    delay(200);
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
  int pHArray[ArrayLenth];
  for(int i = 0; i < 20; i++){
    pHArray[pHArrayIndex++] = analogRead(SensorPin);
    delay(200);
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
    Serial.println("Error number for the array to avraging!/n");
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
