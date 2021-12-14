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
#include <avr/sleep.h>
#include <LowPower.h>


//Pines para el control de puente en H
#define IN1 2
#define IN2 3
#define BRIDGE_ACTIVATION_PIN 4

//Pines utilizados para el sensor de humedad
#define D7 7
#define D8 8
#define A0 14


//Variables de control de los niveles de humedad
double moistureThresholdMin = 25.0; 		//60.0
double moistureThresholdMax = 40.0; 		//75.0
double brokenMoistureSensor_Threshold = 0.0;//3.0. Cuando la humedad medida está por debajo de este valor se considera que algo ha ocurrido con el sensor.
		 	 	 	 	 	 	 	 	 	// Por ejemplo el sensor puede estar desconectado o roto el cable. Un valor tan bajo siempre indica una corriente
											// muy baja por el divisor de tension que forman la R=1k y el sensor de humedad del suelo.

//Variables para la medidad de la humedad
double 			lastMoistureMeasure;
unsigned long 	takeCareOfPlantPeriod_in_secs 	= 3600; //3600

//Variables para el control del riego de la planta
boolean 		enableWatering		= true;
unsigned long 	wateringTime_ms 	= 2000;
unsigned int 	wateringTimes 		= 1;
unsigned long	delayBetweenWateringTimes_ms = 1000;


//valores de la regresion lineal de la funcion que relaciona la lectura del arduino (x = digital) con el porcentaje de humedad del suelo (y)
//Cuando el sustrato se encuentra practicamente seco, el valor que lee el arduino en el divisor de tension es 900.
//Cuando el sensor se introduce en agua el valor que lee el arduino es 0.
float b = 100.0;
float m = -b/900.0;


//Timer variables
unsigned int prescale = 64;
byte prescale_mode = 0b011;
unsigned long cpu_clock_freq = 16000000; //In Hz
unsigned long time_counter_in_secs = 0;

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

	//Configure timer1
	configureTimer1ClockSource(1024);
	initTimer1(4);
	startTimer1();


	LowPower.powerSave(SLEEP_MODE_PWR_SAVE, ADC_OFF, BOD_OFF, TIMER2_OFF);
	//detachInterrupt(0);


}

// The loop function is called in an endless loop
void loop()
{
}


//************************************
//CONFIGURACION TIMER E INTERRUPCIONES
//************************************

//Atmega 328p (Arduino Uno, Nano)
// Frecuencias
// Timer0   62500 Hz
// Timer1   31250 Hz
// Timer2   31250 Hz
// Prescalers
// Timer0   1 8 64 256 1024
// Timer1   1 8 64 256 1024
// Timer2   1 8 32 64 128 256 1024
// Valores por defecto
// Timer0 64   977Hz
// Timer1 64   490Hz
// Timer2 64   490Hz
// Consecuencias
// Timer0   delay() y millis()
// Timer1   Librería servo
// Timer2


void configureTimer1ClockSource(unsigned int _prescale){

	byte mode;


	switch(_prescale) {
		case 0:     prescale_mode = 0b000; prescale=_prescale; break; //prescaler a 0  Timer1 OFF.
		case 1: 	prescale_mode = 0b001; prescale=_prescale; break; //prescaler a 1. Timer1 clock source T1 = 16.000.000 Hz (0.0625us)
		case 8: 	prescale_mode = 0b010; prescale=_prescale; break; //prescaler a 8. Timer1 clock source T1 = 2.000.000 Hz (0.5us)
	    case 64: 	prescale_mode = 0b011; prescale=_prescale; break; //prescaler a 64. Timer1 clock source T1 = 250.000 Hz (4us)
	    case 256:	prescale_mode = 0b100; prescale=_prescale; break; //prescaler a 256. Timer1 clock source T1 = 62.500 Hz (16us)
	    case 1024: 	prescale_mode = 0b101; prescale=_prescale; break; //prescaler a 1024. Timer1 clock source T1 = 15.625 Hz (64us)
	    default: return;
	}

}

void stopTimer1()
{
	configureTimer1ClockSource(0);
	TCCR1B = (TCCR1B & 0b11111000) | prescale_mode;
}

void startTimer1()
{
	TCCR1B = (TCCR1B & 0b11111000) | prescale_mode;
}

void initTimer1(unsigned long time_in_secs){

	//Puesta a cero del contador del timer1
	TCNT1  = 0;

	//Reseteo de los bits del Timer/Counter Control Register
	TCCR1A = 0;
	TCCR1B = 0;

	// Set output compare mode as disconected
	TCCR1A = TCCR1A & 0b00111111;

	//Configure the operation mode (CTC Mode)
	TCCR1B |= (1 << WGM12);

    //Configure the output compare register A
	OCR1A = time_in_secs * (cpu_clock_freq/prescale) ;

	//Serial.print("OCR1A = ");
	//Serial.println(OCR1A);
	//Enable Timer1 Output Compare Interrupt
	TIMSK1 = TIMSK1 | (1<<OCIE1A);

}


//Timer1 Counter compare with OCR1A ISR
//Esta llamada se produce cada vez que el contador del timer1 alcanza el valor

ISR(TIMER1_COMPA_vect)
{

	stopTimer1();
	TCNT1 = 0;                    //reset the timer 1 counter

	time_counter_in_secs = time_counter_in_secs + 4;
	if (time_counter_in_secs>takeCareOfPlantPeriod_in_secs)
	{
		time_counter_in_secs = 0;
		takeCareOfPlant();
	}

	configureTimer1ClockSource(1024);
	initTimer1(4);
    startTimer1();

	LowPower.powerSave(SLEEP_MODE_PWR_SAVE, ADC_OFF, BOD_OFF, TIMER2_OFF);
	//detachInterrupt(0);

}


//****************************************
//FIN CONFIGURACION TIMER E INTERRUPCIONES
//****************************************

void takeCareOfPlant()
{
	lastMoistureMeasure = checkMoisture(D7, D8, A0); // @suppress("Invalid arguments")

	if (isSoilMoistureSensorBroken(lastMoistureMeasure)) // @suppress("Invalid arguments")
	{
		//The sensor is broken. We cannot continue
	}
	else
	{
		if ((lastMoistureMeasure>moistureThresholdMin)) {
			//Don't do anything
			Serial.println("Moisture above = " + String(moistureThresholdMin));
		}
		else
		{
			Serial.println("Moisture value is under the minimum = " + String(moistureThresholdMin));
			if (enableWatering)
			{
				while (lastMoistureMeasure<moistureThresholdMax)
				{

					waterPlant(IN1,IN2,wateringTime_ms, wateringTimes, delayBetweenWateringTimes_ms); // @suppress("Invalid arguments")
					lastMoistureMeasure = checkMoisture(D7, D8, A0); // @suppress("Invalid arguments")
					if (isSoilMoistureSensorBroken(lastMoistureMeasure)) // @suppress("Invalid arguments")
					{
						//The sensor is broken. We cannot continue
						break;
					}
				}
			}
			else
			{
				Serial.println("Watering is disabled. Then, we won't water the plant");
			}

		}
	}
}

bool isSoilMoistureSensorBroken(double moistureMeasure)
{
	bool BrokenMoistureSensor = false;

	if (lastMoistureMeasure<brokenMoistureSensor_Threshold)
	{
		//No conduce corriente o esta es muy baja. Posible rotura del sensor o del cable o fallo en la conexion con el Arduino
		Serial.println("OJO!!!! POSIBLE ROTURA DEL CABLE DEL SENSOR O FALLO EN LA CONEXION CON EL ARDUINO.");
		BrokenMoistureSensor = true;
	}

	return BrokenMoistureSensor;
}
double checkMoisture(int positivePin, int negativePin, int channelLecture)
{

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

	humidityLecture = analogRead(channelLecture); //devuelve un valor entre 0 y 1024 propio de un adc de 10 bits

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


