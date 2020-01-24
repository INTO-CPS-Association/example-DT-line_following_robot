#include "Arduino.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdio.h>

int enA = 2;
int in1 = 3;
int in2 = 4;
// motor two

int in3 = 5;
int in4 = 6;
int enB = 7;

#define SENSOR_CENTER_LEFT_PIN A7
#define SENSOR_CENTER_CENTER_PIN A6
#define SENSOR_CENTER_RIGHT_PIN A5

#define MOTOR_LEFT 0
#define MOTOR_RIGHT 1

void direction(int motor, bool fw) {

	int k1 = 0;
	int k2 = 0;

	if (motor == MOTOR_LEFT) {
		k1 = in1;
		k2 = in2;
	} else if (motor == MOTOR_RIGHT) {
		k1 = in3;
		k2 = in4;
	} else {
		return;
	}

	if (fw) {
		digitalWrite(k1, HIGH);
		digitalWrite(k2, LOW);
	} else {
		digitalWrite(k1, LOW);
		digitalWrite(k2, HIGH);
	}
}

#define TURN_RIGHT 0
#define TURN_LEFT 1
void turn(bool dir) {
	if (dir == TURN_RIGHT) {
		direction(MOTOR_LEFT, true);
		direction(MOTOR_RIGHT, false);
	} else {
		direction(MOTOR_LEFT, false);
		direction(MOTOR_RIGHT, true);
	}
}

int alive = 0;

#define FR 5.0
#define FS 4.0
#define BR 1.0

#define ALIVE_PIN 13

int availableMemory() {
	int size = 8192;
	byte *buf;
	while ((buf = (byte *) malloc(--size)) == NULL)
		;
	free(buf);
	return size;
}

void showMenu() {
	Serial.println("Menu:");
	Serial.println("\ts: Stop");
	Serial.println("\tg: Go");
	Serial.println("\tr: Turn Right");
	Serial.println("\tl: Turn Left");
	Serial.println("\ts: Stop");
	Serial.println("\tf: Forward Strait");
	Serial.println("\ti: Sensor Info");
	Serial.println("\th: Help");
}

void setup() {

	pinMode(ALIVE_PIN, OUTPUT);
	Serial.begin(115200);
	while (!Serial)
		;
	Serial.println("booting...");

	Serial.println("setting pin mode");
	// set all the motor control pins to outputs
	pinMode(enA, OUTPUT);
	pinMode(enB, OUTPUT);
	pinMode(in1, OUTPUT);
	pinMode(in2, OUTPUT);
	pinMode(in3, OUTPUT);
	pinMode(in4, OUTPUT);

//	pinMode(13,OUTPUT);
	pinMode(SENSOR_CENTER_RIGHT_PIN, INPUT);
	pinMode(SENSOR_CENTER_CENTER_PIN, INPUT);
	pinMode(SENSOR_CENTER_LEFT_PIN, INPUT);

	Serial.println("setting motor direction motors");
	// turn motor A FW
	digitalWrite(in1, HIGH);
	digitalWrite(in2, LOW);

	// turn motor B FW
	digitalWrite(in3, HIGH);
	digitalWrite(in4, LOW);

	// turn on

//	Serial.println("starting motors low speed");
	digitalWrite(enA, 0);
	digitalWrite(enB, 0);

	showMenu();

}

char incomingByte = 0;

// The loop function is called in an endless loop
void loop() {

	if (Serial.available() > 0) {
		// read the incoming byte:
		incomingByte = Serial.read();

		if (incomingByte == 's') {
			Serial.println("Stopping both motors");
			analogWrite(enA, 0);
			analogWrite(enB, 0);
		} else if (incomingByte == 'g') {
			Serial.println("Starting both motors");
			direction(MOTOR_LEFT, true);
			direction(MOTOR_RIGHT, true);
			analogWrite(enA, 50);
			analogWrite(enB, 50);

		} else if (incomingByte == 'r') {
			Serial.println("Turn right");
			turn(TURN_RIGHT);
		} else if (incomingByte == 'l') {
			Serial.println("Turn left");
			turn(TURN_LEFT);
		} else if (incomingByte == 'f') {
			Serial.println("Forward Strait");
			direction(MOTOR_LEFT, true);
			direction(MOTOR_RIGHT, true);
		} else if (incomingByte == 'i') {
			Serial.print("Sensor Status: Left= ");
			Serial.print(500 * !digitalRead(SENSOR_CENTER_LEFT_PIN));
			Serial.print(" right= ");
			Serial.println(500 * !digitalRead(SENSOR_CENTER_RIGHT_PIN));
		} else if (incomingByte == 'h') {
			showMenu();
		}

	}
}

