/*
 * SerialProtocol.cpp
 *
 *  Created on: 16 Jan 2020
 *      Author: Kenneth Lausdahl
 */

#include "SerialProtocol.h"

#define SERIAL_PROTOCOL_ESCAPE 0x7d
#define SERIAL_PROTOCOL_FRAME_FLAG 0x7e
#define SERIAL_PROTOCOL_ESCAPE_VALUE 0x20

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

void send(SerialWriterFunc sender, SERIAL_PROTOCOL_MSG_TYPE *serialMessage) {
	sendSerialMessage(sender, (uint8_t *) serialMessage,
			sizeof(SERIAL_PROTOCOL_MSG_TYPE));
}

size_t serial_buffer_index = -1;
uint8_t serial_buffer[sizeof(SERIAL_PROTOCOL_MSG_TYPE)];

bool receive(SerialReadBytesFunc reader, SERIAL_PROTOCOL_MSG_TYPE *data) {

	uint8_t val;
	uint8_t decodedVal;
	bool escapeNext = false;

	unsigned long now = millis();
	// Serial.println("r");
	while (millis() - now < 10) {

		bool dataAvaliable = (*reader)(&val, 1);
		if (!dataAvaliable) {
			delay(1);
			continue;

		}

		if (val == SERIAL_PROTOCOL_FRAME_FLAG) {
			serial_buffer_index = 0;
			continue;
		}

		if (val == SERIAL_PROTOCOL_ESCAPE) {
			escapeNext = true;
			continue;
		} else if (escapeNext) {
			decodedVal = val ^ SERIAL_PROTOCOL_ESCAPE_VALUE;
			escapeNext = false;
		} else {
			decodedVal = val;
		}

		if (serial_buffer_index == sizeof(SERIAL_PROTOCOL_MSG_TYPE)) {

			serial_buffer_index = -1;
			uint8_t checksum = 0;

			for (size_t i = 0; i < sizeof(SERIAL_PROTOCOL_MSG_TYPE); i++) {
				checksum ^= serial_buffer[i];
				// char hex2[5];
			}

			if (checksum == decodedVal) {
				*data = *((SERIAL_PROTOCOL_MSG_TYPE *) serial_buffer);
				return true;
			}
			return false;
		} else if (serial_buffer_index < sizeof(SERIAL_PROTOCOL_MSG_TYPE)) {
			serial_buffer[serial_buffer_index++] = decodedVal;
		}
	}
	return false;
}
