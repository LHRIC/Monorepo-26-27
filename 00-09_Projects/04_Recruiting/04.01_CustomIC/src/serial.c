#include "serial.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>

uint8_t serial_buffer[128];
uint8_t current_serial_data;
uint8_t serial_buffer_size;

void init_serial(void) {
   DDRB |= (1 << PB1);   // PB1 as output
   DDRB &= ~(1 << PB2);  // PB2 as input
   PORTB &= ~(1 << PB2); // Ensure no pull-up

   // Set up in 3-wire mode with external clock source and counter overflow
   // interrupts
   USICR = (1 << USIOIE) | (0 << USIWM1) | (1 << USIWM0) | (1 << USICS1) |
           (0 << USICS0) | (0 << USICLK);
}

void reset_serial_ptr(uint8_t size) {
   current_serial_data = 0;
   serial_buffer_size = size;
   USIDR = serial_buffer[current_serial_data];
   USISR = (1 << USIOIF);
}

ISR(USI_OVF_vect) {
   current_serial_data++;
   if (current_serial_data >= serial_buffer_size) {
      current_serial_data = 0;
   }
   USIDR = serial_buffer[current_serial_data];
   USISR = (1 << USIOIF);
}
