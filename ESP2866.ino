//test code for your esp2866

#include <dht.h>

#include<stdlib.h>
#include <SoftwareSerial.h>

dht DHT1;
#define dht_pin 8
// LED
int ledPin = 13;
// LM35 analog input

int temp = 22;
// replace with your channel's thingspeak API key
String apiKey = "VN93UPWI0HDB3QYY";

// connect 10 to TX of Serial USB
// connect 11 to RX of serial USB
SoftwareSerial ser(2,3); // RX, TX

// this runs once
void setup() {                
  // initialize the digital pin as an output.
  pinMode(ledPin, OUTPUT);    

  // enable debug serial
  Serial.begin(9600);
  // enable software serial
  ser.begin(9600);

  // reset ESP8266
 // ser.println("AT+RST");
}


// the loop
void loop() {

  // blink LED on board
  digitalWrite(ledPin, HIGH);   
  delay(200);               
  digitalWrite(ledPin, LOW);

  
  DHT1.read11(dht_pin);
  temp= DHT1.temperature; 

  // convert to string
  char buf[16];
  String strTemp = dtostrf(temp, 4, 1, buf);

  Serial.println(strTemp);

  // TCP connection
  
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // api.thingspeak.com
  cmd += "\",80";
  ser.println(cmd);
  Serial.println(cmd);
  
  if(ser.find("Error")){
    Serial.println("AT+CIPSTART error");
    return;
  }

  // send data length
  
  // prepare GET string
  String getStr = "GET /update?api_key=";
  getStr += apiKey;
  getStr +="&field1=";
  getStr += String(strTemp);
  //getStr += "\r\n\r\n";

  String cmd1="AT+CIPSEND=";
  cmd1+=String(getStr.length());
  ser.println(cmd1);
  Serial.println(cmd1);

  if(ser.find(">")){
    ser.println(getStr);
    //ser.println(getStr);
    Serial.println(">"+getStr);
  }
  else{
    ser.println("AT+CIPCLOSE");
    // alert user
    Serial.println("AT+CIPCLOSE");
  }

  // thingspeak needs 15 sec delay between updates
  //ser.println("AT+RST");
  delay(30000);  
}
