/*
 * SerialProtocol.h
 *
 *  Created on: 16 Jan 2020
 *      Author: Kenneth Lausdahl
 */

#ifndef PROTOCOL_PROTOCOL_SRC_SERIALPROTOCOL_H_
#define PROTOCOL_PROTOCOL_SRC_SERIALPROTOCOL_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __AVR__
#include <Arduino.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>

#else

#define delay(x)

#endif

#include "SerialProtocolMessageType.h"

#ifndef SERIAL_PROTOCOL_MSG_TYPE
#error A serial protocol message type must be defined!
#endif

typedef size_t (*SerialWriterFunc)(uint8_t val);

typedef size_t (*SerialReadBytesFunc)(uint8_t *buffer, size_t length);

bool receive(SerialReadBytesFunc reader, SERIAL_PROTOCOL_MSG_TYPE *data);

void send(SerialWriterFunc sender, SERIAL_PROTOCOL_MSG_TYPE* serialMessage);

#endif /* PROTOCOL_PROTOCOL_SRC_SERIALPROTOCOL_H_ */
