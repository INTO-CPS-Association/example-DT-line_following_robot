/*
 * lineFollowerSetup:
 *	Create a new wiringPi device node for the Line Follower on the Pi's
 *	I2C interface.
 *********************************************************************************
 */
#include <byteswap.h>
#include <stdio.h>
#include <stdint.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>

#include "io.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * lineFollowerAnalogRead:
 *	Pin is the channel to sample on the device.
 *	Channel 0 is the left sensor,
 *	channel 1 is the right sensor.
 */
static int lineFollowerAnalogRead (struct wiringPiNodeStruct *node, int pin)
{
	int chan = pin - node->pinBase;
	int16_t  result;
	
	// We only have 2 sensors (0,1)
	if (chan > 1)
		return 0;
	
	uint16_t select = (chan << 1);

	result = wiringPiI2CReadReg16 (node->fd, select);
	
	return (int) result;
}

static void lineFollowerAnalogWrite(struct wiringPiNodeStruct *node, int pin, int value)
{
	int chan = pin - node->pinBase;
	
	// We only have 2 motors (0,1)
	//if (chan < 2 )
	//	return;
	
	uint16_t select = chan;

	wiringPiI2CWriteReg16 (node->fd, select, (int16_t) value);
}

int lineFollowerSetup (const int pinBase, int i2cAddr)
{
  struct wiringPiNodeStruct *node ;
  int fd ;

  if ((fd = wiringPiI2CSetup (i2cAddr)) < 0)
    return FALSE ;

  node = wiringPiNewNode (pinBase, 8) ;

  node->fd           = fd ;
  // optional configuration data to remember
  node->data0        = 0;
  node->data1        = 0;
  node->data2        = 0;
  node->analogRead   = lineFollowerAnalogRead;
  node->analogWrite  = lineFollowerAnalogWrite;

  return TRUE ;
}

#ifdef __cplusplus
}
#endif