#include <GravityTDS.h>
//#include <ArduinoLowPower.h>

#include <EEPROM.h>
#include <Wire.h> 
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include "DHT.h"

#include <ThreeWire.h>  
#include <RtcDS1302.h>

#include <SPI.h>
#include <SD.h>

//****RTC****
ThreeWire myWire(4, 5, 3); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);
#define countof(a) (sizeof(a) / sizeof(a[0]))
String curr_time;
//****RTC end****

//****TDS****
#define tds A1
GravityTDS gravityTds;
float tds_temp = 25, tds_val = 0;
//****TDS end****

//****Turbidity****
int turb_pin = A0;
float volt;
float turb_val;
//****Turbudity end****

//****Temperature****
#define ONE_WIRE_BUS 6
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float temp_celc=0;
float temp_fahr=0;
//****Temperature end****

//****pH****
int ph_pin = A2;
float calibration_value = 0;
unsigned long int avgval;
int buffer_arr[20], temp;
float ph_val = 0;
//****pH end****

//****DHT****
#define DHTPIN A3// Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

float dht_hum = 0;
float dht_celc = 0;
float dht_fahr = 0;
float dht_hic = 0;
float dht_hif = 0;
//****DHT end****

//****SD Card****
File myFile;
char fname[15];
//String fname;
//****end SD Card****

void setup()
{

  while (!Serial);
  Serial.begin(9600);
  delay(100);

  SD.begin(10);
  rtc_init();

  Serial.println("==== SLICE Node 1 ====");

  gravityTds.setPin(tds);
  gravityTds.setAref(5.0);  //reference voltage on ADC, default 5.0V on Arduino UNO
  gravityTds.setAdcRange(1024);  //1024 for 10bit ADC;4096 for 12bit ADC
  gravityTds.begin();  //initialization

  dht.begin();
  Serial.println("begin test");
}

int16_t packetnum = 0;  // packet counter, we increment per transmission
 
void loop()
{
  int int_temp, int_ph;
  long int_turb, int_tds;
  
  int pos = 0;
  byte b1, b2, b3;
  byte loraMsg[10];

  int global_reading_delay = 5000;
  Serial.println("Collecting data...");

  RtcDateTime now = Rtc.GetDateTime();

    printDateTime(now);
    Serial.println();

    if (!now.IsValid())
    {
//         Common Causes:
//            1) the battery on the device is low or even missing and the power line was disconnected
        Serial.println("RTC lost confidence in the DateTime!");
    }

  
  delay(global_reading_delay);
  now = Rtc.GetDateTime();
  printDateTime(now);
  dht_class();
  Serial.println("Printing to file ");
  myFile = SD.open("insitu.txt", FILE_WRITE);
  myFile.print(curr_time);
  myFile.print(" - Humidity: ");
  myFile.print(dht_hum);
  myFile.print(", Temperature (C): ");
  myFile.print(dht_celc);
  myFile.print(", Temperature (F): ");
  myFile.print(dht_fahr);
  myFile.print(", Heat Index (C): ");
  myFile.print(dht_hic);
  myFile.print(", Heat Index (F): ");
  myFile.println(dht_hif);
  
//  myFile = SD.open(String(fname));
//  Serial.println(fname);
//  // read from the file until the line ends
//  String data = myFile.readString();
//  Serial.println(data);
//  // close the file:
//  myFile.close();

  delay(global_reading_delay);
  
  now = Rtc.GetDateTime();
  printDateTime(now);
  
  temp_class();
  myFile.println(curr_time);
  myFile.print(" - Water Temperature: ");
  myFile.println(temp_celc);
  
  Serial.print("Water Temperature: ");
  Serial.println(temp_celc);
//  
  delay(global_reading_delay);
  
  now = Rtc.GetDateTime();
  printDateTime(now);
  
  turb_class();
  myFile.print(curr_time);
  myFile.print(" - Turbidity: ");
  myFile.println(turb_val);

  Serial.print("Turbidity: ");
  Serial.println(turb_val);
  
  delay(global_reading_delay);
  
  now = Rtc.GetDateTime();
  printDateTime(now);
  
  tds_class();
  myFile.print(curr_time);
  myFile.print(" - TDS: ");
  myFile.println(tds_val);

  Serial.print("TDS: ");
  Serial.println(tds_val);
  
  delay(global_reading_delay);
   
  now = Rtc.GetDateTime();
  printDateTime(now);
  
  ph_class();
  myFile.print(curr_time);
  myFile.print(" - pH: ");
  myFile.println(ph_val);
  myFile.println();
  Serial.print("pH: ");
  Serial.println(ph_val);

  myFile.close();
  
  delay(global_reading_delay);
}

void rtc_init(){
    Serial.print("compiled: ");
    Serial.print(__DATE__);
    Serial.println(__TIME__);

    Rtc.Begin();

    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    printDateTime(compiled);
    Serial.println();

    if (!Rtc.IsDateTimeValid()) 
    {
        // Common Causes:
        //    1) first time you ran and the device wasn't running yet
        //    2) the battery on the device is low or even missing

        Serial.println("RTC lost confidence in the DateTime!");
        Rtc.SetDateTime(compiled);
    }

    if (Rtc.GetIsWriteProtected())
    {
        Serial.println("RTC was write protected, enabling writing now");
        Rtc.SetIsWriteProtected(false);
    }

    if (!Rtc.GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) 
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }
    else if (now > compiled) 
    {
        Serial.println("RTC is newer than compile time. (this is expected)");
    }
    else if (now == compiled) 
    {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }
}

void temp_class(){
  sensors.requestTemperatures(); 
  temp_celc=sensors.getTempCByIndex(0);
  temp_fahr=sensors.toFahrenheit(temp_celc);
}

void turb_class(){
    volt = 0;
    for(int i=0; i<800; i++)
    {
        volt += ((float)analogRead(turb_pin)/1024)*5;
    }
    volt = volt/800;
    volt = round_to_dp(volt,2);
//    Serial.print("Turbidity volt: ");
//    Serial.print(volt);
    if(volt < 2.5){
      turb_val = 3000;
    }else{
      turb_val = -1120.4*square(volt)+5742.3*volt-4352.9; 
    }
}

void tds_class(){
    gravityTds.setTemperature(tds_temp);  // set the temperature and execute temperature compensation
    gravityTds.update();  //sample and calculate
    tds_val = gravityTds.getTdsValue();  // then get the value
}

void ph_class(){
  for(int i=0; i<20; i++){
    buffer_arr[i] = analogRead(ph_pin);
    delay(20);
  }
  for(int i=0; i<19; i++){
    for(int j=i+1; j<20; j++){
      if(buffer_arr[i] > buffer_arr[j]){
        temp = buffer_arr[i];
        buffer_arr[i] = buffer_arr[j];
        buffer_arr[j] = temp;
      }
    }
  }
  avgval = 0;
  for(int i=2; i<18; i++){
    avgval += buffer_arr[i];
  }
  float volt = (float)avgval * 5.0 / 1024 / 16;
  ph_val = 3.5 * volt + calibration_value;
}

void dht_class(){
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  dht_hum = dht.readHumidity();
  // Read temperature as Celsius (the default)
  dht_celc = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  dht_fahr = dht.readTemperature(true);

  // Compute heat index in Fahrenheit (the default)
  dht_hif = dht.computeHeatIndex(dht_fahr, dht_hum);
  // Compute heat index in Celsius (isFahreheit = false)
  dht_hic = dht.computeHeatIndex(dht_celc, dht_hum, false);
}

float round_to_dp( float in_value, int decimal_place )
{
  float multiplier = powf( 10.0f, decimal_place );
  in_value = roundf( in_value * multiplier ) / multiplier;
  return in_value;
}

//void setFileName(const RtcDateTime& dt){
////    char datestring[15];
//
//    snprintf_P(fname, 
//            countof(fname),
//            PSTR("%02u-%02u-%04u.txt"),
//            dt.Month(),
//            dt.Day(),
//            dt.Year()
//            );
////    fname = datestring;
//
////    snprintf_P(fname, countof(fname), datestring);
//    Serial.println(fname);
//}

void printDateTime(const RtcDateTime& dt)
{
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
//    Serial.print(datestring);
    curr_time = datestring;
    
}
