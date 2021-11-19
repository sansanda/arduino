#include "Arduino.h"
//The setup function is called once at startup of the sketch

unsigned int counter = 0;

void setup()
{
	// Add your initialization code here
	// initialize serial communication at 9600 bits per second:
	Serial.begin(9600);


}

// The loop function is called in an endless loop
void loop()
{
	if (counter<=0)
	{
		digitalWrite(11, HIGH);
		digitalWrite(12, LOW);


		delay(2000);

		Serial.println(analogRead(A0));

		digitalWrite(11, LOW);
		digitalWrite(12, HIGH);

	}

	counter++;



}
