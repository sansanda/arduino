/*

*/

#include <SPI.h>
#include <Adafruit_GFX.h>
#include "LedControl.h"

int NumMatrix = 4;
/* 
 * Now we create a new LedControl. 
 * We use pins 12,11 and 10 on the Arduino for the SPI interface
 * Pin 12 is connected to the DATA IN-pin of the first MAX7221
 * Pin 11 is connected to the CLK-pin of the first MAX7221
 * Pin 10 is connected to the LOAD(/CS)-pin of the first MAX7221 	
 * There will only be a single MAX7221 attached to the arduino 
 */
LedControl lc1=LedControl(12,11,10,NumMatrix);


int moisture;
int analogPinA0 =A0;

int IN1 = 3; //D3 
int IN2 = 2; //D2

long elapsedTimeInms = 0;
long elapsedTimeInSeconds = 0;

int percentage;
int highLevel_Moisture = 1023; //0%
int lowLevel_Moisture = 456;   //100%
int moisturePercentatge = 0;
bool isWaterPumpOn = false;
bool needWater = false;

/*
  This method will scroll all the hexa-decimal
 numbers and letters on the display. You will need at least
 four 7-Segment digits. otherwise it won't really look that good.
 */
void scrollDigits() {
  for(int i=0;i<13;i++) {
    lc1.setDigit(1,7,i,false);
    lc1.setDigit(1,7,i+1,false);
    lc1.setDigit(1,7,i+2,false);
    lc1.setDigit(1,7,i+3,false);
    delay(1000);
  }
  lc1.clearDisplay(0);
  delay(1000);
}

/*
 This method will display the characters for the
 word "Arduino" one after the other on the matrix. 
 (you need at least 5x7 leds to see the whole chars)
 */
void writeArduinoOnMatrix() {
  int delaytime = 1000;
  /* here is the data for the characters */
  byte a[5]={B01111110,B10001000,B10001000,B10001000,B01111110};
  byte r[5]={B00111110,B00010000,B00100000,B00100000,B00010000};
  byte d[5]={B00011100,B00100010,B00100010,B00010010,B11111110};
  byte u[5]={B00111100,B00000010,B00000010,B00000100,B00111110};
  byte i[5]={B00000000,B00100010,B10111110,B00000010,B00000000};
  byte n[5]={B00111110,B00010000,B00100000,B00100000,B00011110};
  byte o[5]={B00011100,B00100010,B00100010,B00100010,B00011100};

  /* now display them one by one with a small delay */
  lc1.setRow(0,0,a[0]);
  lc1.setRow(0,1,a[1]);
  lc1.setRow(0,2,a[2]);
  lc1.setRow(0,3,a[3]);
  lc1.setRow(0,4,a[4]);
  delay(delaytime);
  lc1.setRow(0,0,r[0]);
  lc1.setRow(0,1,r[1]);
  lc1.setRow(0,2,r[2]);
  lc1.setRow(0,3,r[3]);
  lc1.setRow(0,4,r[4]);
  delay(delaytime);
  lc1.setRow(0,0,d[0]);
  lc1.setRow(0,1,d[1]);
  lc1.setRow(0,2,d[2]);
  lc1.setRow(0,3,d[3]);
  lc1.setRow(0,4,d[4]);
  delay(delaytime);
  lc1.setRow(0,0,u[0]);
  lc1.setRow(0,1,u[1]);
  lc1.setRow(0,2,u[2]);
  lc1.setRow(0,3,u[3]);
  lc1.setRow(0,4,u[4]);
  delay(delaytime);
  lc1.setRow(0,0,i[0]);
  lc1.setRow(0,1,i[1]);
  lc1.setRow(0,2,i[2]);
  lc1.setRow(0,3,i[3]);
  lc1.setRow(0,4,i[4]);
  delay(delaytime);
  lc1.setRow(0,0,n[0]);
  lc1.setRow(0,1,n[1]);
  lc1.setRow(0,2,n[2]);
  lc1.setRow(0,3,n[3]);
  lc1.setRow(0,4,n[4]);
  delay(delaytime);
  lc1.setRow(0,0,o[0]);
  lc1.setRow(0,1,o[1]);
  lc1.setRow(0,2,o[2]);
  lc1.setRow(0,3,o[3]);
  lc1.setRow(0,4,o[4]);
  delay(delaytime);
  lc1.setRow(0,0,0);
  lc1.setRow(0,1,0);
  lc1.setRow(0,2,0);
  lc1.setRow(0,3,0);
  lc1.setRow(0,4,0);
  delay(delaytime);
}


bool testMotor(int in1, int in2)
{
  // Motor gira en un sentido
  digitalWrite (in1, HIGH);
  digitalWrite (in2, LOW); 
  delay(4000);
  // Motor no gira
  digitalWrite (in1, LOW); 
  delay(500);
  // Motor gira en sentido inverso
  digitalWrite (in2, HIGH);
  delay(4000);
  // Motor no gira
  digitalWrite (in2, LOW); 
  delay(5000);    
}

bool turnMotorLeft(int in1, int in2)
{
  // Motor gira en un sentido
  digitalWrite (in1, HIGH);
  digitalWrite (in2, LOW);  
}
bool turnMotorRight(int in1, int in2)
{
  // Motor gira en un sentido
  digitalWrite (in1, LOW);
  digitalWrite (in2, HIGH);  
}
bool stopMotor(int in1, int in2)
{
  // Motor gira en un sentido
  digitalWrite (in1, LOW);
  digitalWrite (in2, LOW);  
}

bool waterThePlant()
{
  Serial.print("Need Water? ");
  if (needWater) 
  {
    Serial.print("Yes.");
    turnMotorLeft(IN1,IN2);
    Serial.print("Starting Water Pump.");
    delay(20000); 
    stopMotor(IN1,IN2);
    Serial.print("Stopping Water Pump.");
    delay(200000);  
  }
  else {Serial.print("No.");    
  } 
  Serial.print("\n"); 
}

//If previousNeed = true we come from low humidity state
//If previousNeed = false we come from high humidity state

bool needsWater(int moisturePercentatge, int lowLevelLimit, int highLevelLimit, bool previousNeed)
{
  bool needsWAter = true;
  if (moisturePercentatge<lowLevelLimit) 
    return needsWAter;
  if (moisturePercentatge>highLevelLimit) 
    return !needsWAter;
  if (moisturePercentatge>lowLevelLimit & moisturePercentatge<highLevelLimit)
  {
    if (previousNeed) 
      return needsWAter;
    else
      return !needsWAter;
  }
  return !needsWAter;
}

int measureMoisture()
{
  moisture = analogRead(analogPinA0);
  Serial.print(moisture);
  moisturePercentatge = map(moisture,highLevel_Moisture,lowLevel_Moisture,0,100);  
  Serial.print("Moisture: ");
  Serial.print(moisturePercentatge);
  Serial.print("%\n");
  return moisturePercentatge;
}
void setup() {
  Serial.begin(9600);
  pinMode (IN1, OUTPUT);    
  pinMode (IN2, OUTPUT); 
  for (int i=0; i< NumMatrix ; i++)
  {
    lc1.shutdown(i,false);  // Activar la matriz
    lc1.setIntensity(i,8);  // Poner el brillo a un valor intermedio
    lc1.clearDisplay(i);    // Y borrar todo
  }
    
}


void loop() {
  //testMotor(IN1,IN2);
   
  //if ((elapsedTimeInSeconds%5)==0) //Execute code every 5 seconds
  //{
    moisturePercentatge = measureMoisture();
  //  needWater = needsWater(moisturePercentatge,40,70, needWater);
    //if (needWater) waterThePlant(); 
  //}
  delay(1000);
  //writeArduinoOnMatrix();
  //elapsedTimeInSeconds = elapsedTimeInSeconds + 1;
  
}
