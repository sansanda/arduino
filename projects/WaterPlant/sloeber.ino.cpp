#ifdef __IN_ECLIPSE__
//This is a automatic generated file
//Please do not modify this file
//If you touch this file your change will be overwritten during the next build
//This file has been generated on 2021-12-12 23:50:27

#include "Arduino.h"
#include "Arduino.h"

void setup() ;
void loop() ;
bool isSoilMoistureSensorBroken(double moistureMeasure) ;
double checkMoisture(int positivePin, int negativePin, int channelLecture) ;
unsigned int readMoistureSensor(int positivePin, int negativePin, int channelLecture) ;
void waterPlant(int controlPin1, int controlPin2, unsigned long wateringTime_ms, unsigned int wateringTimes, unsigned long delayBetweenWateringTimes_ms) ;

#include "waterPlant.ino"


#endif
