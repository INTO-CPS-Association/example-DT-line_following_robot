/*
 Controlling a servo position using a potentiometer (variable resistor)
 by Michal Rinott <http://people.interaction-ivrea.it/m.rinott>

 modified on 8 Nov 2013
 by Scott Fitzgerald
 http://www.arduino.cc/en/Tutorial/Knob
 */
#include <Arduino.h>
#include <Servo.h>
//#include <SoftwareSerial.h>

//http://learn.parallax.com/tutorials/robot/shield-bot/robotics-board-education-shield-arduino/chapter-2-shield-lights-servo-11
Servo leftServo;  // create servo object to control a servo
Servo rightServo;

// left rs A1 23
const int RS_LEFT_PIN = A1;
//right rs A0 24
const int RS_RIGHT_PIN = A0;
//left servo 5
const int SERVO_LEFT_PIN = 5;
//right servo 6
const int SERVO_RIGHT_PIN = 6;
//Stop-go button pin
const int STOP_GO_PIN = 2;

#define MASTER_MODE_PIN A2

//IO
//8 is connected to ATMega1284p pin 17 D3
const int SERVO_LEFT_ENABLE_PIN = 8;
//9 is connected to ATMega1284p pin 16 D2
const int SERVO_RIGHT_ENABLE_PIN = 9;

//software serial
//Arduino TX 10 to ATMega1284p RX pin 14
const int SW_SERIAL_TX_PIN = 10;
//Arduino RX 11 to ATMega1284p TX pin 15
const int SW_SERIAL_RX_PIN = 11;

#define I2C_MODE 1

#include <Wire.h>

/*
 * Set right motor speed
 * i2cset -y 1 0x29 0x04 501 w
 * Set left motor speed
 * i2cset -y 1 0x29 0x03 501 w
 * Get left sensor
 * i2cget -y 1 0x29 0 w
 * $0x0400
 * Get right sensor
 * i2cget -y 1 0x29 2 w
 * $0x012d
 * */

/********************************************************************/

#define  SLAVE_ADDRESS           0x29  //slave address,any number from 0x01 to 0x7F
#define  REG_MAP_SIZE            6
#define  MAX_SENT_BYTES          4
#define  IDENTIFICATION          0x0D

#define LEFT_SENSOR_ADDRESS 0
#define RIGHT_SENSOR_ADDRESS 2
#define LEFT_MOTOR_ADDRESS 3
#define RIGHT_MOTOR_ADDRESS 4
#define MASTER_MODE_ADDRESS 4
void storeData();

/********* Global  Variables  ***********/
byte registerMap[REG_MAP_SIZE];
byte registerMapTemp[REG_MAP_SIZE];
byte receivedCommands[MAX_SENT_BYTES];
byte newDataAvailable = 0;

//on arduino uno this is 2 bytes
int leftMotorSpeed = 0;
int rightMotorSpeed = 0;
int rightSensor = 301;
int leftSensor = 1024;
byte masterMode = 0;

/******************************************/

//const int fullSpeedClockwise = 1300;
//1530
//const int fullSpeedCounterclockwise = 1700;
const int deadStill = 1500;

const int fullSpeedClockwise = 1300 + 180; //deadStill - 15;
const int fullSpeedCounterclockwise = 1700 - 180; // deadStill + 15;

void setup() {
	pinMode(SERVO_LEFT_ENABLE_PIN, INPUT);
	pinMode(SERVO_RIGHT_ENABLE_PIN, INPUT);

	pinMode(RS_LEFT_PIN, INPUT);
	pinMode(RS_RIGHT_PIN, INPUT);
	pinMode(MASTER_MODE_PIN, INPUT_PULLUP);

	leftServo.attach(SERVO_LEFT_PIN);
	rightServo.attach(SERVO_RIGHT_PIN);

	Serial.begin(9600);

	Wire.begin(SLAVE_ADDRESS);
	Wire.onRequest(requestEvent);
	Wire.onReceive(receiveEvent);
	registerMap[13] = IDENTIFICATION; // ID register
}

uint8_t lServoCmd;
uint8_t rServoCmd;

void loop() {

	lServoCmd = digitalRead(SERVO_LEFT_ENABLE_PIN);
	rServoCmd = digitalRead(SERVO_RIGHT_ENABLE_PIN);
	masterMode = digitalRead(MASTER_MODE_PIN);

	rightSensor = analogRead(RS_RIGHT_PIN);
	leftSensor = analogRead(RS_LEFT_PIN);

	storeData();
	newDataAvailable = 1;

	//Serial.print("Master mode = ");
	//Serial.println(masterMode);
	//delay(500);

	if (masterMode == I2C_MODE) {
		leftServo.writeMicroseconds(leftMotorSpeed);
		rightServo.writeMicroseconds(rightMotorSpeed);
//		Serial.print("L = ");
//		Serial.print(leftMotorSpeed);
//		Serial.print(" R = ");
//		Serial.println(rightMotorSpeed);
	} else {

		if (lServoCmd == 0) {
//			Serial.println("Servo Left = CCW");
			leftServo.writeMicroseconds(fullSpeedCounterclockwise);
		} else if (lServoCmd > 0) {
//			Serial.println("Servo Left = CW");
			leftServo.writeMicroseconds(fullSpeedClockwise);
		}

		if (rServoCmd == 0) {
			//Serial.println("Servo Right = CCW");
			rightServo.writeMicroseconds(fullSpeedCounterclockwise);
		} else if (rServoCmd > 0) {
//			Serial.println("Servo Right = CW");
			rightServo.writeMicroseconds(fullSpeedClockwise);
		}
	}
}

void storeInt(byte startIndex, int* ptr) {
	byte arrayIndex = startIndex;
	byte * bytePointer = (byte*) ptr; //latitude is 4 bytes
	for (int i = 0; i < 2; i++) {
		registerMapTemp[arrayIndex] = bytePointer[i]; //increment pointer to store each byte
		arrayIndex++;
	}
}

void reStoreInt(byte address, int* ptr) {

	int tmp = receivedCommands[1];
	tmp += (receivedCommands[1 + 1] << 8);
	*ptr = tmp;
	//char buffer[200];
	//sprintf(buffer, "Setting address %d = %d", address, *ptr);
	//Serial.println(buffer);
}

void storeData() {

	storeInt(0, &leftSensor);
	storeInt(2, &rightSensor);
	registerMapTemp[4] = masterMode;
}

void requestEvent() {
	if (newDataAvailable) {
		for (int c = 0; c < (REG_MAP_SIZE - 1); c++) {
			registerMap[c] = registerMapTemp[c];
		}
	}

	newDataAvailable = 0;

	//Set the buffer to send all 14 bytes
	Wire.write(registerMap + receivedCommands[0], REG_MAP_SIZE);
}

void receiveEvent(int bytesReceived) {
	for (int a = 0; a < bytesReceived; a++) {
		if (a < MAX_SENT_BYTES) {
			receivedCommands[a] = Wire.read();
		} else {
			Wire.read(); // if we receive more data then allowed just throw it away
		}
	}

	if (bytesReceived == 1 && (receivedCommands[0] < REG_MAP_SIZE)) {
		return;
	}

	if (bytesReceived == 1 && (receivedCommands[0] >= REG_MAP_SIZE)) {
		receivedCommands[0] = 0x00;
		return;
	}

	switch (receivedCommands[0]) {
	case RIGHT_MOTOR_ADDRESS:
		reStoreInt(receivedCommands[0], &leftMotorSpeed);
		return;
	case LEFT_MOTOR_ADDRESS:
		reStoreInt(receivedCommands[0], &rightMotorSpeed);
		return;
	default:
		return; // ignore the commands and return
	}

}

