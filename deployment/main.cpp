#include "Arduino.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdio.h>

#include "modeldescription.h"

#define DATA_BITS_PER_SECONDS(t) (8*(t))
#define DATA_BROADCAST_INTERVAL_MS 10
#define DATA_BOUDRATE 115200

#define checkBoudRate(typeSize, intervalInMs) intervalInMs>0 && DATA_BOUDRATE > (DATA_BITS_PER_SECONDS(typeSize)*(1000/intervalInMs))

struct measurement_pair {

	uint8_t vref;
	float value;

}__attribute ((__packed__));

struct measurement_bundle {

	uint8_t msg_type;
	float time;
	measurement_pair values[4];

}__attribute ((__packed__));

#define SERIAL_PROTOCOL_ESCAPE 0x7d
#define SERIAL_PROTOCOL_FRAME_FLAG 0x7e
#define SERIAL_PROTOCOL_ESCAPE_VALUE 0x20
typedef size_t (*SerialWriterFunc)(uint8_t val);
void sendSerialMessage(SerialWriterFunc sender, uint8_t *data, int length) {
	(*sender)(SERIAL_PROTOCOL_FRAME_FLAG);
	uint8_t checksum = 0;
	for (int i = 0; i < length; i++) {
		uint8_t c = data[i];
		checksum ^= c;

		if (c == SERIAL_PROTOCOL_ESCAPE || c == SERIAL_PROTOCOL_FRAME_FLAG) {
			(*sender)(SERIAL_PROTOCOL_ESCAPE);
			(*sender)(c ^ SERIAL_PROTOCOL_ESCAPE_VALUE);

		} else {
			(*sender)(c);
		}
	}
	(*sender)(checksum);
	(*sender)(0x0A);
}

void send(SerialWriterFunc sender, measurement_bundle *serialMessage) {
	sendSerialMessage(sender, (uint8_t *) serialMessage,
			sizeof(measurement_bundle));
}

size_t swSend(uint8_t val) {
	return Serial3.write(val);
}

extern "C" {

#include "Fmu.h"
//#include "FmuModel.h"
#include "Controller.h"

}
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

measurement_bundle m;

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

void fmuLoggerCache(void *componentEnvironment, fmi2String instanceName,
		fmi2Status status, fmi2String category, fmi2String format, ...) {
	return;
//	Serial.println(format);
	va_list args;
	size_t len;
	char *space;

	va_start(args, format);
	len = vsnprintf(0, 0, format, args);
	va_end(args);
	if ((space = (char*) malloc(len + 1)) != 0) {
		va_start(args, format);
		vsnprintf(space, len + 1, format, args);
		va_end(args);
		Serial.println(space);
		free(space);
	}

}

fmi2Component instReturn = NULL;
double now = 0;
double step = 0.00001;

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

void setup() {

	pinMode(ALIVE_PIN, OUTPUT);
	Serial.begin(115200);
	Serial3.begin(DATA_BOUDRATE);
	while (!Serial)
		;
	Serial.println(F("booting..."));
	//Serial.println(sizeof(measurement_bundle));

	Serial.println(F("setting pin mode"));
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

	Serial.println(F("setting motor direction motors"));
	// turn motor A FW
	digitalWrite(in1, HIGH);
	digitalWrite(in2, LOW);

	// turn motor B FW
	digitalWrite(in3, HIGH);
	digitalWrite(in4, LOW);

	// turn on

	Serial.println(F("starting motors low speed"));
	analogWrite(enA, 50);
	analogWrite(enB, 50);

	for (int i = 0; i < 9; i++)
		fmiBuffer.realBuffer[i] = 0.0;

	fmiBuffer.realBuffer[FMI_LEFTVAL] = 0.0;
	fmiBuffer.realBuffer[FMI_RIGHTVAL] = 0.0;
	fmiBuffer.realBuffer[FMI_SERVOLEFTOUT] = -9.99;
	fmiBuffer.realBuffer[FMI_SERVORIGHTOUT] = -9.99;
	fmiBuffer.realBuffer[FMI_THRESHOLD] = 400.0;
	fmiBuffer.realBuffer[FMI_FORWARDROTATE] = FR;
	fmiBuffer.realBuffer[FMI_BACKWARDROTATE] = BR;
	fmiBuffer.realBuffer[FMI_FORWARDSPEED] = FS;

	fmi2CallbackFunctions callback = { &fmuLoggerCache, NULL, NULL, NULL, NULL };

	Serial.println(F("Instantiating: "));
	Serial.println(availableMemory());
	instReturn = fmi2Instantiate("S", fmi2CoSimulation, FMI_GUID, "", &callback,
			fmi2True, fmi2True);
	Serial.println(F("Instantiate called"));
	Serial.println(availableMemory());
	Serial.println(instReturn == NULL);
	Serial.println(F("setup done"));

	//callback.logger(NULL, "some name", fmi2OK, "logAll", "called &periodic_taskg_System_controller__Z12control_loopEV\n");
}

int count = 0;

int encode(uint8_t *checksum, uint8_t *value, size_t size) {
//	Serial.println("k");

	int index = 0;
	for (size_t i = 0; i < size; i++) {
//		Serial.println("k");
		uint8_t c = *(value + i);
//		Serial.println("k1");
		//Serial.print(c, HEX);
		//Serial.print(" ");
		*checksum ^= c;

		if (c == SERIAL_PROTOCOL_ESCAPE || c == SERIAL_PROTOCOL_FRAME_FLAG) {
			Serial3.write(SERIAL_PROTOCOL_ESCAPE);
			//delay(10);
			//	c2 = c;	// ^ SERIAL_PROTOCOL_ESCAPE_VALUE;
			//c2 ^= SERIAL_PROTOCOL_ESCAPE_VALUE;
			//				delay(1);
			Serial3.write(c ^ SERIAL_PROTOCOL_ESCAPE_VALUE);

		} else {
			Serial3.write(c);
		}
	}

	return index;
}

unsigned int lastBroadcast = 0;
// The loop function is called in an endless loop
void loop() {

	if (instReturn == NULL)
		return;

	fmiBuffer.realBuffer[FMI_LEFTVAL] = 500
			* !digitalRead(SENSOR_CENTER_LEFT_PIN);
	fmiBuffer.realBuffer[FMI_RIGHTVAL] = 500
			* !digitalRead(SENSOR_CENTER_RIGHT_PIN);

	Serial.print(".");
	fmi2DoStep(NULL, now, step, false);

	now = now + step;
	Serial.print(" ");
	Serial.print(now);

	// sync buffer with hardware
	double servoRight = fmiBuffer.realBuffer[FMI_SERVORIGHTOUT];
	double servoLeft = fmiBuffer.realBuffer[FMI_SERVOLEFTOUT];

	if (servoRight == -FS && servoLeft == FS) {
		//Go Strait
		direction(MOTOR_LEFT, true);
		direction(MOTOR_RIGHT, true);
	}

	if (servoRight == -FR && servoLeft == BR) {
		//go left
		turn(TURN_LEFT);
	}

	if (servoRight == -BR && servoLeft == FR) {
		//go right
		turn(TURN_RIGHT);
	}

	// alive indicator
	digitalWrite(ALIVE_PIN, alive);

	alive = !alive;

	unsigned int broadcastTimeNow = millis();

	static_assert(checkBoudRate(sizeof(measurement_bundle),DATA_BROADCAST_INTERVAL_MS),"Too many bits per second");

	if (broadcastTimeNow - lastBroadcast > DATA_BROADCAST_INTERVAL_MS) {
		lastBroadcast = broadcastTimeNow;

		m.time = broadcastTimeNow;
		m.values[0].vref = FMI_LEFTVAL;
		m.values[0].value = fmiBuffer.realBuffer[FMI_LEFTVAL];

		m.values[1].vref = FMI_RIGHTVAL;
		m.values[1].value = fmiBuffer.realBuffer[FMI_RIGHTVAL];

		m.values[2].vref = FMI_SERVOLEFTOUT;
		m.values[2].value = fmiBuffer.realBuffer[FMI_SERVOLEFTOUT];

		m.values[3].vref = FMI_SERVORIGHTOUT;
		m.values[3].value = fmiBuffer.realBuffer[FMI_SERVORIGHTOUT];

		send(&swSend, &m);

		size_t index = 0;
		Serial3.write(SERIAL_PROTOCOL_FRAME_FLAG);
		uint8_t checksum = 0;

//		uint8_t offset = index;
//		uint8_t value = 4;
//		Serial.println("A");
		/*
		 * Encode a struct like
		 *
		 * double time
		 * uint8_t id
		 * double value
		 * ...
		 * */
		//encode(&checksum, &now, sizeof(uint8_t));
		encode(&checksum, (uint8_t*) &now, sizeof(double));
		uint8_t id = FMI_LEFTVAL;
		encode(&checksum, &id, sizeof(uint8_t));
		encode(&checksum, (uint8_t*) &(fmiBuffer.realBuffer[FMI_LEFTVAL]),
				sizeof(double));

//		id = FMI_RIGHTVAL;
//		encode(&checksum, &id, sizeof(uint8_t));
//		encode(&checksum, (uint8_t*) &(fmiBuffer.realBuffer[FMI_RIGHTVAL]),
//				sizeof(double));
//
//		id = FMI_SERVOLEFTOUT;
//		encode(&checksum, &id, sizeof(uint8_t));
//		encode(&checksum, (uint8_t*) &(fmiBuffer.realBuffer[FMI_SERVOLEFTOUT]),
//				sizeof(double));
//
//		id = FMI_SERVORIGHTOUT;
//		encode(&checksum, &id, sizeof(uint8_t));
//		encode(&checksum, (uint8_t*) &(fmiBuffer.realBuffer[FMI_SERVORIGHTOUT]),
//				sizeof(double));

		Serial3.write(checksum);
		Serial3.write(0x0A);
		Serial3.flush();
	}

}

