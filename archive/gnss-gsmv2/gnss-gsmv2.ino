#include <SoftwareSerial.h>
SoftwareSerial sim808(10,11);

char phone_no[] = "09512596007"; // replace with your phone no.
String data[5];
#define DEBUG true
String state,timegps,latitude,longitude;

void setup() {
 sim808.begin(9600);
 Serial.begin(9600);
 delay(50);

 sim808.println("AT+CSMP=17,167,0,0");  // set this parameter if empty SMS received
 delay(100);
 sim808.println("AT+CMGF=1"); 
 delay(400);

 sendData("AT+CGNSPWR=1",1000,DEBUG);
 delay(50);
 sendData("AT+CGNSSEQ=RMC",1000,DEBUG);
 delay(150);
}

void loop()
{
  if(Serial.available()){
    String data_rcvd = Serial.readString();
    sendTabData("AT+CGNSINF",1000,DEBUG);
    Serial.println("Serial data received!");
    if (state !=0) {
      
      Serial.println("State  :"+state);
      Serial.println("Time  :"+timegps);
      Serial.println("Latitude  :"+latitude);
      Serial.println("Longitude  :"+longitude);
      Serial.println("Data  :"+data_rcvd);
//  
      sim808.print("AT+CMGS=\"");
      sim808.print(phone_no);
      sim808.println("\"");
      
      delay(300);

      sim808.print("data: ");
      sim808.println(data_rcvd);
      sim808.print("http://maps.google.com/maps?q=loc:");
      sim808.print(latitude);
      sim808.print(",");
      sim808.print(longitude);

      delay(200);
      sim808.println((char)26); // End AT command with a ^Z, ASCII code 26
      delay(200);
      sim808.println();
//      delay(1000);
      delay(20000);
      sim808.flush();
      
    } else {
      Serial.println("GPS Initialising...");
    }  
  } else {
//    Serial.println("No serial detected!");
  }
}

void sendTabData(String command , const int timeout , boolean debug){
  sim808.flush();
  sim808.println(command);
  Serial.println("--------Command  : " + command);
  long int time = millis();
  int i = 0;

  while((time+timeout) > millis()){
    while(sim808.available()){
      char c = sim808.read();
      if (c != ',') {
         data[i] +=c;
         delay(100);
      } else {
        i++;  
      }
      if (i == 5) {
        delay(100);
        goto exitL;
      }
    }
  }exitL:
  if (debug) {
    state = data[1];
    timegps = data[2];
    latitude = data[3];
    longitude =data[4];  
  }
  Serial.println("---DATA LOG---");
  for (int i = 0; i < 5; i++){
    Serial.println(data[i]);
  }
  Serial.println("---END DATA LOG---");
}

String sendData (String command , const int timeout ,boolean debug){
  String response = "";
  sim808.flush();
  sim808.println(command);
  Serial.println("--------Command send  : " + command);
  long int time = millis();
  int i = 0;

  while ( (time+timeout ) > millis()){
    while (sim808.available()){
      char c = sim808.read();
      response +=c;
    }
  }
  if (debug) {
//     Serial.print(response);
     }
     return response;
}
