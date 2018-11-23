//testing accelerometer and sending (x,y,z)values over bluetooth to a device

//using Wire library

#include "Wire.h" // This library allows you to communicate with I2C devices.
#include "SoftwareSerial.h"
const int MPU_ADDR = 0x68; // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.
int16_t acc_x, acc_y, acc_z; // variables for accelerometer raw data
int16_t gyro_x, gyro_y, gyro_z; // variables for gyro raw data
int16_t temp; // variables for temperature data
SoftwareSerial BTSerial(2, 3);

#define ACCEL_COUNT 125
byte accelBuffer[ACCEL_COUNT];
int index = 2;

char tmp_str[7]; // temporary variable used in convert function
char* convert_int16_to_str(int16_t i) { // converts int16 to string. Moreover, resulting strings will have the same length in the debug monitor.
  sprintf(tmp_str, "%6d", i);
  return tmp_str;
}
void setup() {
  Serial.begin(9600);
  Wire.begin();
  //Serial1.begin(9600);
  Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  initBuffer();
}
void loop() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
  Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
  Wire.requestFrom(MPU_ADDR, 7 * 2, true); // request a total of 7*2=14 registers
  read_sensor();

/*
  if (index >= ACCEL_COUNT - 3) {
    sendToRemote();
    initBuffer();
    Serial.println("------------- Send 20 accel data to remote");
  }
*/

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
  temp   = Wire.read() << 8 | Wire.read(); // reading registers: 0x41 (TEMP_OUT_H) and 0x42 (TEMP_OUT_L)
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
  Serial.print(acc_z, DEC);
  // the following equation was taken from the documentation [MPU-6000/MPU-6050 Register Map and Description, p.30]
  //Serial.print(" | tmp = "); Serial.print(temp/340.00+36.53);
  /*
    Serial.print(" | gX = "); Serial.print(convert_int16_to_str((gyro_x)));
    Serial.print(" | gY = "); Serial.print(convert_int16_to_str((gyro_y)));
    Serial.print(" | gZ = "); Serial.println(convert_int16_to_str((gyro_z)));
  */
  //Serial.println(index);
  Serial.println("");



}
/*
void sendToRemote() {
  
  // Write gabage bytes
  Serial1.write( "accel" );
  // Write accel data
  Serial1.write( (char*)accelBuffer );
  //Flush buffer
  //BTSerial.flush();
}*/
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
