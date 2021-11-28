/**
 *
 * Version 1.0 del proyecto para automatizar el riego de una planta.
 *
 * Actualmente se basa en las siguentes particularidades:
 *
 * 1. CIRCUITO DEL SENSOR DE HUMEDAD
 * Como sensor de humedad utiliza dos barras metálicas de unos 200mm de largo y separadas 10mm. Practicamente se comporta como un elemento curra resistencia
 * varia en funcion de la huemdad del suelo. Habría que determinar si esta variación es lineal pero de moemnto ya nos vale.
 * Este sensor se coloca en serie con una R de 1K formando un divisor resistivo.
 * Los extremos de este divisor resistivo se conectan cada uno a un pin digital del arduino (D7 y D8) para su polarización bipolar (circulación de corrient en ambos sentidos)
 * y así evitar la electrólisis cada vez que se procede a la medida de voltaje la toma media.
 * La toma media de este sensor se conecta a la entrada analogica del arduino, la A0, para la lectura de la tensión en el punto medio. Esta tension será indicativo de la humedad
 * del suelo de la planta.
 *
 * 2. CIRCUITO DE ALIMENTACIÓN Y CONTRO DEL RIEGO
 * Utilizamos una bomba periestaltica cuyo eje se acciona mediante un motor CC de 6V alimentado a través de un puente en H con una pila de 9V.
 *
 * 3. ALIMENTACION DEL ARDUINO
 * Por lo pronto la alimentacion del arduino es via usb
 *
 * 4. ASPECTOS A MEJORAR EN LAS PROXIMAS VERSIONES
 *
 * 		4.1 Uso de timer, interrupciones y rutinas para la automatizacion de la medida de la humedad del suelo y el riego
 * 		4.2 Añadir un display para que, cuando apretemos un boton nos muestre la lectura actual de la humedad del suelo y los diferentes parametros del control
 * 		los valores de min y max.
 * 		4.3 Añadir la opcion de modificar los parametros de control de manera externa mediante un teclado.
 *
 * Este sensor se pincha en la tierra hasta la mitad aprox.
 */

#include "Arduino.h"

//Pines para el control de puente en H
#define IN1 2
#define IN2 3
#define BRIDGE_ACTIVATION_PIN 4

//Pines utilizados para el sensor de humedad
#define D7 7
#define D8 8
#define A0 14


//Variables de control de los niveles de humedad
double moistureThresholdMin = 60.0; 	//60.0
double moistureThresholdMax = 75.0; 	//75.0


//Variables para la medidad de la humedad
double 			lastMoistureMeasure;
unsigned long 	moistureMeasuringPeriod_ms 	= 3600000; //3600000

//Variables para el control del riego de la planta
unsigned long 	wateringTime_ms 	= 2000;
unsigned int 	wateringTimes 		= 1;
unsigned long	delayBetweenWateringTimes_ms = 10000;


//valores de la regresion lineal de la funcion que relaciona la lectura del arduino (x = digital) con el porcentaje de humedad del suelo (y)
float m = -0.1683;
float b = 164.468;

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
	lastMoistureMeasure = checkMoisture(D7, D8, A0); // @suppress("Invalid arguments")

	if ((lastMoistureMeasure>moistureThresholdMin)) {

// 		Don't do anything
		Serial.println("Moisture above = " + String(moistureThresholdMin));

	} else {

		Serial.println("Moisture value is under the minimum = " + String(moistureThresholdMin));

		while (lastMoistureMeasure<moistureThresholdMax)
		{

			waterPlant(IN1,IN2,wateringTime_ms, wateringTimes, delayBetweenWateringTimes_ms); // @suppress("Invalid arguments")
			lastMoistureMeasure = checkMoisture(D7, D8, A0); // @suppress("Invalid arguments")
		}

	}

	delay(moistureMeasuringPeriod_ms); //check humidity every moistureMeasuringPeriod_ms
}

double checkMoisture(int positivePin, int negativePin, int channelLecture){

	Serial.println("Checking plant moisture value....");
	unsigned int  a0_lecture = readMoistureSensor(D7, D8, A0); // @suppress("Invalid arguments")
	Serial.println("Plant Moisture Lecture = " + String(a0_lecture));

	double moistureLecture = (m * a0_lecture) + b;

	Serial.println("Plant Moisture in percent = " + String(moistureLecture));

	return moistureLecture;
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

void waterPlant(int controlPin1, int controlPin2, unsigned long wateringTime_ms, unsigned int wateringTimes, unsigned long delayBetweenWateringTimes_ms)
{
	Serial.println("Proceding to water the plant....");

	for (int i = 0;i<wateringTimes;i++)
	{
		digitalWrite(BRIDGE_ACTIVATION_PIN, HIGH); //Activación de la lógica de control del puente en H

		delay(200);

		digitalWrite (controlPin1, HIGH);
		digitalWrite (controlPin2, LOW);

		delay(wateringTime_ms);

		digitalWrite (controlPin1, LOW);
		digitalWrite (controlPin2, LOW);

		digitalWrite(BRIDGE_ACTIVATION_PIN, LOW); //Desactivación de la lógica de control del puente en H

		delay(delayBetweenWateringTimes_ms);
	}

}


