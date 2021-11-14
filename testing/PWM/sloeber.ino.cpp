#ifdef __IN_ECLIPSE__
//This is a automatic generated file
//Please do not modify this file
//If you touch this file your change will be overwritten during the next build
//This file has been generated on 2021-11-14 22:57:47

#include "Arduino.h"
#include "Arduino.h"

void initPortAsOutput(const byte pinNumber,                       const bool initialState);
void initPortAsInput( const byte pinNumber,                       const byte mode);
void initTimer1(unsigned const long cpu_freq,                 unsigned const int  prescaler,                 unsigned const long freq,                 const byte          dutycicle);
void startTimer1(unsigned int prescaler);
ISR(TIMER1_OVF_vect) ;
void startBurst(const byte          pinNumber,                 unsigned const long cpu_freq,                 unsigned const int  timer_prescaler,                 unsigned const long desired_freq,                 const byte          desired_dutycycle                ) ;
void startBurstTrain( unsigned const int    nBursts,                       unsigned const long*  pulsesPerBurst,                       unsigned const long   delayBetweenBursts,                       const byte            pinNumber,                       unsigned const long   cpu_freq,                       unsigned const int    timer_prescaler,                       unsigned const long   desired_freq,                       const byte            desired_dutycycle                     ) ;
void setup() ;
void loop() ;

#include "testPWM.ino"


#endif
