#include "Arduino.h"

#define IN1 2
#define IN2 3

#define BRIDGE_ACTIVATION_PIN 4

#define D7 7
#define D8 8

#define A0 14


unsigned int moistureThresholdMin = 55;
unsigned int moistureThresholdMax = 80;

unsigned long checkingPeriod_ms = 10000; //60000
unsigned long wateringTime_ms = 1000;

//The setup function is called once at startup of the sketch

void setup()
{
	// Add your initialization code here
	// initialize serial communication at 9600 bits per second:
	Serial.begin(9600);

	//Definimos como modo salida los pines IN1 y IN2 (digitales) que servirán para controlar la salida 1 del doble puente en H que es el L298N
	pinMode(IN1, OUTPUT);
	pinMode(IN2, OUTPUT);

	//Definimos como modo salida los pines D7 y D8 (digitales) que servirán para polarizar el sensor de humedad del suelo (moisture sensor)
	pinMode(D7, OUTPUT);
	pinMode(D8, OUTPUT);

	//Definimos como modo salida el pin BRIDGE_ACTIVATION_PIN = 4 que serirá para alimentar la lógica del puente en H.
	pinMode(BRIDGE_ACTIVATION_PIN, OUTPUT);


	//Definimos como modo entrada el pin A0 (Analogico)
	pinMode(A0, INPUT);

	//Initializing outputs
	digitalWrite(IN1, LOW);
	digitalWrite(IN2, LOW);
	digitalWrite(D7, LOW);
	digitalWrite(D8, LOW);
	digitalWrite(BRIDGE_ACTIVATION_PIN, LOW);

}

// The loop function is called in an endless loop
void loop()
{
	Serial.println("Checking plant moisture value....");
	unsigned int  a0_lecture = readMoistureSensor(D7, D8, A0);
	Serial.println("Plant Moisture Lecture = " + String(a0_lecture));
	unsigned long moistureLecture = map(a0_lecture, 300L, 1024L, 100L, 0L);
	Serial.println("Plant Moisture in percent = " + String(moistureLecture));
	if ((moistureLecture>moistureThresholdMin and moistureLecture<moistureThresholdMax)) {

// 		Don't do anything
		Serial.print("Moisture value between max = " + String(moistureThresholdMin));
		Serial.println(" and Max = " + String(moistureThresholdMax) );

	} else {

		Serial.println("Moisture value is under the minimum = " + String(moistureThresholdMin));
		waterPlant(IN1,IN2,wateringTime_ms);
	}
	delay(checkingPeriod_ms); //check humidity every 60" or 60000ms
}

unsigned int readMoistureSensor(int positivePin, int negativePin, int channelLecture)
{
	unsigned int humidityLecture;

	delay(1000);

	digitalWrite(positivePin, HIGH);
	digitalWrite(negativePin, LOW);

	delay(1000);

	humidityLecture = analogRead(channelLecture);

	delay(100);

	digitalWrite(positivePin, LOW);
	digitalWrite(negativePin, HIGH);

	delay(1100);

	digitalWrite(D7, LOW);
	digitalWrite(D8, LOW);

	return humidityLecture;
}

void waterPlant(int controlPin1, int controlPin2, unsigned long wateringTime_ms)
{
	Serial.println("Proceding to water the plant....");

	digitalWrite(BRIDGE_ACTIVATION_PIN, HIGH); //Activación de la lógica de control del puente en H

	delay(200);

	digitalWrite (controlPin1, HIGH);
	digitalWrite (controlPin2, LOW);

	delay(wateringTime_ms);

	digitalWrite (controlPin1, LOW);
	digitalWrite (controlPin2, LOW);

	digitalWrite(BRIDGE_ACTIVATION_PIN, LOW); //Desactivación de la lógica de control del puente en H
}


