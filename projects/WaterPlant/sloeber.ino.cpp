#ifdef __IN_ECLIPSE__
//This is a automatic generated file
//Please do not modify this file
//If you touch this file your change will be overwritten during the next build
//This file has been generated on 2021-12-22 12:28:29

#include "Arduino.h"
#include "Arduino.h"
#include <avr/sleep.h>
#include <LowPower.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>

void setup() ;
void loop() ;
void configureTimer1ClockSource(unsigned int _prescale);
void stopTimer1() ;
void startTimer1() ;
void initTimer1(unsigned long time_in_secs);
ISR(TIMER1_COMPA_vect) ;
void takeCareOfPlant() ;
bool isSoilMoistureSensorBroken(double moistureMeasure) ;
double checkMoisture(int positivePin, int negativePin, int channelLecture) ;
unsigned int readMoistureSensor(int positivePin, int negativePin, int channelLecture) ;
void waterPlant(int controlPin1, int controlPin2, unsigned long wateringTime_ms, unsigned int wateringTimes, unsigned long delayBetweenWateringTimes_ms) ;


#include "waterPlant.ino"

#endif
