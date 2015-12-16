//Modified from the Adafruit library code
//Original Code license:
//***************************************************
// Written by Limor Fried/Ladyada for Adafruit Industries.
// BSD license, all text above must be included in any redistribution
//****************************************************/

#include <stdint.h>
#include <math.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include "I2C_16.h"
#include "TMP006.h"

// bluetooth 3.0 pin
#define rxBlu3 4
#define txBlu3 5

// bluetooth 4.0 pin
#define rxBlu4 6 
#define txBlu4 7

// sound and humidity
#define sound 0
#define humidity 3

SoftwareSerial swSerial3(txBlu3, rxBlu3);  // blutooth 3.0 시리얼 연결
SoftwareSerial swSerial4(txBlu4, rxBlu4);  // 블루투스 4.0 시리얼 연결


// 자이로센서 변수
const int MPU=0x68;  //MPU 6050 의 I2C 기본 주소
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;

// 온도센서 변수
uint8_t sensor1 = 0x40; // I2C address of TMP006, can be 0x40-0x47
uint16_t samples = TMP006_CFG_8SAMPLE; // # of samples per reading, can be 1/2/4/8/16
float pre_temp = 1.0;

void setup()
{  Serial.begin(9600); 
  Wire.begin();      // Wire 라이브러리 초기화
  Wire.beginTransmission(MPU); //MPU로 데이터 전송 시작
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // MPU-6050 시작 모드로
  Wire.endTransmission(true);
  pinMode(13,OUTPUT);
  swSerial3.begin(9600);   // 블루투스 3.0 시리얼 가동
  swSerial4.begin(9600);// 블루투스 4.0 시리얼 가동
  delay(1000);
  swSerial4.write("AT+CONNECT=74F07DC98B39\r");
  config_TMP006(sensor1, samples);
}

void loop()
{
  
  // 자이로데이터
  Wire.beginTransmission(MPU);    // 데이터 전송시작
  Wire.write(0x3B);               // register 0x3B (ACCEL_XOUT_H), 큐에 데이터 기록
  Wire.endTransmission(false);    // 연결유지
  Wire.requestFrom(MPU,14,true);  // MPU에 데이터 요청
  AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    

  // 온도데이터
  float object_temp = readObjTempC(sensor1); // 대상 온도
  float sensor_temp = readDieTempC(sensor1); // 주변 온도

  int humidity_val = analogRead(humidity);
  int sound_val = analogRead(sound);
  byte send_data= 0x00;
  byte send_data2= 'a';

  // 자이로데이터 
  if(AcX>-17000 && AcX < -15000){ // 아기가 뒤집혔을 떄
    send_data= 'H';
    
  }else if(object_temp>=38.0 && object_temp <43.0){ // 아기 온도가 38이상 43도 이하로 측정될 경우
    if(abs(object_temp-sensor_temp)<6.0 && abs(object_temp-pre_temp)<0.5){ //주변 온도와 6도 이하로 차이나고 그 전 온도보다 0.5도 이하로 올랐을 때
      send_data= 'A';
    }
  }else if(humidity_val > 530){ // 기저귀가 축축할 때
    send_data=  'I';
  }else if(sound_val<590 || sound_val>710){ // 아기가 울 때
    send_data= 'G';
  }

  if(send_data!=0){
    swSerial3.write(send_data);
    swSerial3.write("\0");
    swSerial4.write('X');
    swSerial4.write('\r');
    swSerial4.write(send_data);
    swSerial4.write('\r');
    send_data = 0  ;
  }
   delay(1000*1); // 1000 == 1초 딜레이
}
