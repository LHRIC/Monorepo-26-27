#include <avr/cpufunc.h>
#include <avr/interrupt.h>
#include <avr/iotn85.h>
#include <stdbool.h>
#include <stdint.h>

#include "rc.h"
#include "serial.h"

void lock_data_mode(void);

void unlock_data_mode(void);

uint8_t scratch_bufer[128];

int main() {
   /*
   // Check Configuration Mode
   MCUCR &= ~(1 << PUD); // Disable Pullups
   // Initialize as input
   DDRB &= ~(1 << PB3);
   PORTB |= (1 << PB3);

   bool mode_pin = (PINB & (1 << PB3)) >> PB3;
   if (mode_pin) {
      unlock_data_mode();
   } else {
      lock_data_mode();
   }
   */

   init_serial();
   // rc_measurement_init();
   sei();
   // rc_measurement_begin();
   // while (!rc_measurement_check_valid())
   //    ;
   //
   // uint32_t measurement = rc_measurement_get();
   // serial_buffer[0] = (measurement & 0xFF);
   // serial_buffer[1] = (measurement & 0xFF00) >> 8;
   // serial_buffer[2] = (measurement & 0xFF0000) >> 16;
   // serial_buffer[3] = (measurement & 0xFF000000) >> 24;
   // reset_serial_ptr();
   serial_buffer[0] = 'H';
   serial_buffer[1] = 'e';
   serial_buffer[2] = 'l';
   serial_buffer[3] = 'l';
   serial_buffer[4] = 'o';
   serial_buffer[5] = '\n';
   reset_serial_ptr(6);

   while (true) {
      _NOP();
   }
}

void lock_data_mode(void) {}

void unlock_data_mode(void) {}
