// Libraries for DHT
#include "dht.h"

//Libraries for RTC DS1302
//#include <ThreeWire.h>  
//#include <RtcDS1302.h>
#include <Time.h>

// DHT11
#define dht_apin A3
dht DHT;

// Temp DS18B20
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 6
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// RTC DS1302v2
#include <RTClib.h>
//   DS1302 rtc(ce_pin, sck_pin, io_pin);
DS1302 rtc(3, 5, 4);

// TDS SEN 0224
#define TdsSensorPin A1
#define VREF 5.0                  // analog reference voltage(Volt) of the ADC
#define SCOUNT  20                // sum of sample point
int analogBuffer[SCOUNT];         // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0,copyIndex = 0;
float averageVoltage = 0,tdsValue = 0,temperature = 25, rawtds = 0;

// I2C Display
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x20,16,2);  // set the LCD address to 0x20 for a 16 chars and 2 line display

// PH SEN0169
#define SensorPin A2                //pH meter Analog output to Arduino Analog Input 0
#define Offset 0.00                 //deviation compensate
#define LED 13
#define ArrayLenth  20              //times of collection
int pHArray[ArrayLenth];            //Store the average value of the sensor feedback
int pHArrayIndex=0;

// SD Card MODULE
#include <SPI.h>
#include <SD.h>

//GLOBAL TIMER VARIABLES
unsigned long timerOne = 0;
unsigned long timerTwo = 0;
unsigned long reportingTimer = 0;
unsigned long writingTimer = 0;
String targetFileName = "";

void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.print("SLICE v3.0 Init...");

  while(!Serial){
    
  }
  pinMode(TdsSensorPin,INPUT);
  pinMode(LED, OUTPUT);
  lcd.setCursor(0,0);
  lcd.print("SD check");
  lcd.setCursor(0,1);
  if(!SD.begin(10)){
    lcd.print("Failed!");
//    while(1);
  }else{
      lcd.print("Success!");
  }
  
  { //move this subrouting for opening the file as necessary
  }
//  delay(1000);
//  Serial.print("Compiled: ");
//  Serial.print(__DATE__);
//  Serial.println(__TIME__);
  delay(1000);

  rtc.begin();
  if(!rtc.isrunning()){
//    Serial.println("RTC is NOT running!");
    rtc.adjust(DateTime(__DATE__, __TIME__));
  }

    DateTime now = rtc.now();
  {     //routine to create new chuvaness
        targetFileName = String(now.month())+"-"+String(now.day())+".txt";
//        Serial.print("Checking if file ");
//        Serial.print(targetFileName);
//        Serial.print(" exists-");
               
        if(!SD.exists(targetFileName)){
//          Serial.print("negative-");
//          Serial.print("creating.");
  
          File myFile = SD.open(targetFileName, FILE_WRITE);
          if(myFile){
            myFile.println("UNIX, DHT_HUM(%), DHT_TEMP(C), pH, TDS(ppm), TDS_RAW(ppm), TEMP(C)");
            myFile.close();
//            Serial.print("created.");
          }else{
//            Serial.print("failed.");
          }
        }else{
//          Serial.print("positive.");    
        }
//        Serial.println();
  }
    delay(1000);
}

void loop () 
{
    unsigned long intervalOne = 20U;
    unsigned long intervalTwo = 1000U;
    unsigned long reportingInterval = intervalTwo;
    unsigned long writingInterval  = reportingInterval * 5; 
    unsigned int currentTime = millis();

    static float pHValue, pHVoltage;

    if(currentTime - timerOne > 20U){
      timerOne = currentTime;

      { // TDS Sampling
        analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
        analogBufferIndex++;
        if(analogBufferIndex == SCOUNT){
          analogBufferIndex = 0;
        }
      }

      { //pH Sampling
        pHArray[pHArrayIndex++] = analogRead(SensorPin);
        if(pHArrayIndex==ArrayLenth){
          pHArrayIndex=0;
        }
      }
    }
    
    if(currentTime - timerTwo >= 1000U){
      timerTwo = currentTime;
      // Temperature
      sensors.requestTemperatures(); 
      temperature=sensors.getTempCByIndex(0);
      { // TDS Central  Calculation
        tdsValue = calculateTDS(temperature);
        rawtds = calculateTDS(25);
      }

      { // pH Central Calculation 
        pHVoltage = avergearray(pHArray, ArrayLenth)*5.0/1024;
        pHValue = 3.5*pHVoltage+Offset;
      }
    }
    
    if(currentTime - reportingTimer >= reportingInterval){
      reportingTimer = currentTime;
        char LCDString1[16] = {0};
        String LCDString = "";

      DHT.read11(dht_apin);
      lcd.clear();
//      lcd.setCursor(0,0);
      LCDString = LCDString + DHT.temperature + ":"+ DHT.humidity +":"+pHValue;
      LCDString.toCharArray(LCDString1, 16);
      lcd.print(LCDString1);

      LCDString = "";
      LCDString = LCDString + tdsValue +":"+pHVoltage+":"+temperature;
      LCDString.toCharArray(LCDString1, 16);
      lcd.setCursor(0,1);
      lcd.print(LCDString1);      
  }

  if(currentTime - writingTimer >= writingInterval){
    writingTimer = currentTime;
    DateTime now = rtc.now();


    String SDString = "";
    SDString = SDString + now.unixtime() + ',' + DHT.humidity + ',' + DHT.temperature + ',' +pHValue + ',' + tdsValue + ',' + rawtds + ',' + temperature;

    Serial.print(SDString);
    {
      File myFile = SD.open(targetFileName, FILE_WRITE);
      if(myFile){
        myFile.println(SDString);
        myFile.close();
      }else{
//        Serial.println("failed to write.");
      }
    }
  }
}

float calculateTDS(float temp){
  float tds = 0;
  for(copyIndex=0;copyIndex<SCOUNT;copyIndex++){
    analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
  }
  averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0;  // read the analog value more stable by the median filtering algorithm, and convert to voltage value
  float compensationCoefficient=1.0+0.02*(temp-25.0);                      //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
  float compensationVoltage=averageVoltage/compensationCoefficient;               //temperature compensation
  tds=(133.42*compensationVoltage*compensationVoltage*compensationVoltage - 255.86*compensationVoltage*compensationVoltage + 857.39*compensationVoltage)*0.5; //convert voltage value to tds value
  return tds;
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
