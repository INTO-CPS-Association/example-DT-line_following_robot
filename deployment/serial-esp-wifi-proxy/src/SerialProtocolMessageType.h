#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifndef serial_protocol_message_type
#define serial_protocol_message_type

struct measurement_pair {

	uint8_t vref;
	float value;

}__attribute ((__packed__));

struct measurement_bundle {

	uint8_t msg_type;
    float time;
	measurement_pair values[4];

}__attribute ((__packed__));



#define SERIAL_PROTOCOL_MSG_TYPE measurement_bundle

#endif /*serial_protocol_message_type*/