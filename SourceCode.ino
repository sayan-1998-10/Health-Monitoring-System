#include <dht.h>

#include "Wire.h" // This library allows you to communicate with I2C devices.
#include "SoftwareSerial.h"
#include <stdlib.h>
#include <Adafruit_SSD1306.h>
#define DHTPIN 8
#define OLED_Address 0x3C
Adafruit_SSD1306 oled(1);

int x=0;
int lastx=0;
int lasty=0;
int LastTime=0;
int ThisTime;
bool BPMTiming=false;
bool BeatComplete=false;
int BPM=0;
#define UpperThreshold 560
#define LowerThreshold 500


dht DHT1;


String apiKey = "VN93UPWI0HDB3QYY";    // Edit this API key according to your Account
String Host_Name = "CleverEagle";         // Edit Host_Name
String Password = "4SConsultancy";          // Edit Password

int my_temperature = 22;
// replace with your channel's thingspeak API key
SoftwareSerial ser(10, 11); // RX, TX

const int MPU_ADDR = 0x68; // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.
int16_t acc_x, acc_y, acc_z; // variables for accelerometer raw data
int16_t gyro_x, gyro_y, gyro_z; // variables for gyro raw data
int16_t temp_mpu; // variables for temperature data


#define ACCEL_COUNT 125
byte accelBuffer[ACCEL_COUNT];
int index = 2;
int i =0;
char tmp_str[7]; // temporary variable used in convert function
char* convert_int16_to_str(int16_t i) { // converts int16 to string. Moreover, resulting strings will have the same length in the debug monitor.
  sprintf(tmp_str, "%6d", i);
  return tmp_str;
}
void setup() {

 
  Serial.begin(9600);
  oled.begin(SSD1306_SWITCHCAPVCC, OLED_Address);
  oled.clearDisplay();
  oled.setTextSize(2);
  Wire.begin();
  Serial1.begin(9600);
  ser.begin(9600);
  Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  ser.println("AT+RST");               // Resetting ESP8266
  char inv ='"';
  String cmd = "AT+CWJAP";
       cmd+= "=";
       cmd+= inv;
       cmd+= Host_Name;
       cmd+= inv;
       cmd+= ",";
       cmd+= inv;
       cmd+= Password;
       cmd+= inv;
  ser.println(cmd);  
 initBuffer();
}
void loop() {
  
 
  
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
  Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
  Wire.requestFrom(MPU_ADDR, 7 * 2, true); // request a total of 7*2=14 registers

  oled_display();
  delay(2);
  esp2866();
  delay(2);
  read_sensor();

  if(index >= ACCEL_COUNT - 3) {
    // delay(1000);
    sendToRemote();
    //delay(1000);
  if(index>=122){
    initBuffer();
    }
   // Serial.println("------------- Send 20 accel data to remote");
  }


  // delay
  delay(100);
}
void read_sensor() {

  //Raw values
  // "Wire.read()<<8 | Wire.read();" means two registers are read and stored in the same variable
  acc_x  = Wire.read() << 8; //msb
  acc_x |= Wire.read(); // reading registers: 0x3B (ACCEL_XOUT_H) and 0x3C (ACCEL_XOUT_L)
  acc_y  = Wire.read() << 8 ;
  acc_y |= Wire.read(); // reading registers: 0x3D (ACCEL_YOUT_H) and 0x3E (ACCEL_YOUT_L)
  acc_z  = Wire.read() << 8 ;
  acc_z |= Wire.read(); // reading registers: 0x3F (ACCEL_ZOUT_H) and 0x40 (ACCEL_ZOUT_L)
  temp_mpu   = Wire.read() << 8 | Wire.read(); // reading registers: 0x41 (TEMP_OUT_H) and 0x42 (TEMP_OUT_L)
  /*uncomment for gyro values
    gyro_x = Wire.read() << 8 ;
    gyro_x |= Wire.read(); // reading registers: 0x43 (GYRO_XOUT_H) and 0x44 (GYRO_XOUT_L)
    gyro_y = Wire.read() << 8 ;
    gyro_y |= Wire.read(); // reading registers: 0x45 (GYRO_YOUT_H) and 0x46 (GYRO_YOUT_L)
    gyro_z = Wire.read() << 8 ;
    gyro_z |= Wire.read(); // reading registers: 0x47 (GYRO_ZOUT_H) and 0x48 (GYRO_ZOUT_L)
  */
  
  if (index < ACCEL_COUNT && index > 1) {
    int tempX = acc_x;
    int tempY = acc_y;
    int tempZ = acc_z;

    char temp = (char)(tempX >> 8);
    if (temp == 0x00)
      temp = 0x7f;
    accelBuffer[index] = temp;
    index++;
    temp = (char)(tempX);
    if (temp == 0x00)
      temp = 0x01;
    accelBuffer[index] = temp;
    index++;

    temp = (char)(tempY >> 8);
    if (temp == 0x00)
      temp = 0x7f;
    accelBuffer[index] = temp;
    index++;
    temp = (char)(tempY);
    if (temp == 0x00)
      temp = 0x01;
    accelBuffer[index] = temp;
    index++;

    temp = (char)(tempZ >> 8);
    if (temp == 0x00)
      temp = 0x7f;
    accelBuffer[index] = temp;
    index++;
    temp = (char)(tempZ);
    if (temp == 0x00)
      temp = 0x01;
    accelBuffer[index] = temp;
    index++;
}

  // print out data
  
  Serial.print("aX = ");
  Serial.print(acc_x, DEC);
  Serial.print(" | aY = ");
  Serial.print(acc_y, DEC);
  Serial.print(" | aZ = ");
  Serial.println(acc_z, DEC);
  // the following equation was taken from the documentation [MPU-6000/MPU-6050 Register Map and Description, p.30]
  //Serial.print(" | tmp = "); Serial.print(temp/340.00+36.53);
  /*
    Serial.print(" | gX = "); Serial.print(convert_int16_to_str((gyro_x)));
    Serial.print(" | gY = "); Serial.print(convert_int16_to_str((gyro_y)));
    Serial.print(" | gZ = "); Serial.println(convert_int16_to_str((gyro_z)));
  */
  Serial.println(index);
  //Serial.println("");



}

void sendToRemote() {
  // Write gabage bytes
  Serial1.write( "accel" );
  // Write accel data
  Serial1.write( (char*)accelBuffer );
  //Flush buffer
  //BTSerial.flush();
}
void initBuffer() {
  index = 2;
  for (int i = index; i < ACCEL_COUNT; i++) {
    accelBuffer[i] = 0x00;
  }
  accelBuffer[0] = 0xfe;
  accelBuffer[1] = 0xfd;
  accelBuffer[122] = 0xfd;
  accelBuffer[123] = 0xfe;
  accelBuffer[124] = 0x00;
}
//----------
void esp2866()
{  
  DHT1.read11(DHTPIN);
  int temperature = DHT1.temperature;        // Reading Temperature Value
 
  char buf[16];
  String strTemp = dtostrf(temperature, 4, 1, buf);
  int humidity = DHT1.humidity;
  char buf1[16];
  String strHumid = dtostrf(humidity, 4, 1, buf1);
 
  String cmd = "AT+CIPSTART=\"TCP\",\"";          // Establishing TCP connection
  cmd += "184.106.153.149";                       // api.thingspeak.com
  cmd += "\",80";                                 // port 80
  ser.println(cmd);
  Serial.println(cmd);                            // AT+CIPSTART="TCP",184.106.153.149,"80"
  
  if(ser.find("Error")){
    Serial.println("AT+CIPSTART error");
    return;
  }
  
  String getStr = "GET /update?api_key=";         // prepare GET string
  getStr += apiKey;
  getStr +="&field1=";
  getStr += String(strTemp); // Temperature Data
  getStr +="&field2=";
  getStr += String(strHumid); 
  getStr += "\r\n\r\n";
  
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());                // Total Length of data
  ser.println(cmd);
  Serial.println(cmd);
  if(ser.find(">")){
    ser.print(getStr);
    Serial.print(getStr);
  }
  else{
    ser.println("AT+CIPCLOSE");                  // closing connection
    // alert user
   Serial.println("AT+CIPCLOSE");
  }
 
  delay(1000);
 }
 void oled_display(){
    
     if(x>127)  
  {
    oled.clearDisplay();
    x=0;
    lastx=x;
  }

  ThisTime=millis();
  int value=analogRead(0);
  oled.setTextColor(WHITE);
  int y=60-(value/16);
  oled.writeLine(lastx,lasty,x,y,WHITE);
  lasty=y;
  lastx=x;
  // calc bpm

  if(value>UpperThreshold)
  {
    if(BeatComplete)
    {
      BPM=ThisTime-LastTime;
      BPM=int(60/(float(BPM)/1000));
      BPMTiming=false;
      BeatComplete=false;
      tone(8,1000,250);
    }
    if(BPMTiming==false)
    {
      LastTime=millis();
      BPMTiming=true;
    }
  }
  if((value<LowerThreshold)&(BPMTiming))
    BeatComplete=true;
    
    // display bpm
  oled.writeFillRect(0,50,128,16,BLACK);
  oled.setCursor(0,50);
  oled.print(BPM+60);
  //Serial.println(BPM-20);
  
  oled.display();
  x++;
}
