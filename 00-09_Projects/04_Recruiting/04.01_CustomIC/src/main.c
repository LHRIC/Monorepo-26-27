#include <avr/cpufunc.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
#include <avr/iotn85.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <util/delay.h>

#include "delay.h"
#include "rc.h"
#include "serial.h"

#define RC_CORRECT_UPPER_BOUND 140000
#define RC_CORRECT_LOWER_BOUND 130000

void lock_data_mode(void);

void unlock_data_mode(void);

int main() {
   init_serial();
   disable_serial();
   sei();

   // Check Configuration Mode
   MCUCR &= ~(1 << PUD); // Disable Pullups
   // Initialize as input
   DDRB &= ~(1 << PB0);
   PORTB |= (1 << PB0);

   bool mode_pin = (PINB & (1 << PB0)) >> PB0;
   if (mode_pin) {
      unlock_data_mode();
   } else {
      lock_data_mode();
   }

   while (true) {
      _NOP();
   }
}

void lock_data_mode(void) {
   enable_serial();
   serial_buffer[0] = 0xDE;
   serial_buffer[1] = 0xAD;
   serial_buffer[2] = 0xBE;
   serial_buffer[3] = 0xEF;
   reset_serial_ptr(4);
   while (true)
      _NOP();
}

void unlock_data_mode(void) {
   rc_measurement_init();

   // Loop test cap until unlock
   while (true) {
      rc_discharge_cap();
      rc_measurement_begin();

      while (!rc_measurement_check_valid())
         ;
      uint32_t measurement = rc_measurement_get();

      if (measurement >= RC_CORRECT_LOWER_BOUND &&
          measurement <= RC_CORRECT_UPPER_BOUND)
         break;

      timer1_delay_ms(500);
   }

   enable_serial();
   uint8_t size =
       snprintf(serial_buffer, 128, "Congrats! You won trial workday!");
   reset_serial_ptr(size);
   while (true)
      _NOP();
}
