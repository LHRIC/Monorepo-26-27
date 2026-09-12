#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

/*
 *  Buffer used for serial communication
 */
extern uint8_t serial_buffer[128];

/*
 *  Initializes Serial Data Output
 *  PB2 is CLK
 *  PB1 is DATA OUT
 */
void init_serial(void);

/*
 *  Resets the serial ptr to start with the start of the buffer
 */
void reset_serial_ptr(uint8_t size);

/*
 *  Disables the serial peripheral
 */
void disable_serial(void);

/*
 *  Re-activates the serial peripheral
 */
void enable_serial(void);

#endif
