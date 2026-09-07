#include <avr/cpufunc.h>
#include <avr/interrupt.h>
#include <avr/iotn85.h>
#include <stdbool.h>
#include <stdint.h>

volatile uint32_t TIMER0_overflow_counter = 0;

volatile bool rc_measurement_flag = false;

void lock_data_mode(void);

void unlock_data_mode(void);

uint8_t serial_buffer[128];
uint8_t current_serial_data;
uint8_t serial_buffer_size;

/*
 *  Initializes Serial Data Output
 *  PB2 is CLK
 *  PB1 is DATA OUT
 */
void init_serial(void) {
   DDRB |= (1 << PB1);   // PB1 as output
   DDRB &= ~(1 << PB2);  // PB2 as input
   PORTB &= ~(1 << PB2); // Ensure no pull-up

   // Set up in 3-wire mode with external clock source and counter overflow
   // interrupts
   USICR = (1 << USIOIE) | (0 << USIWM1) | (1 << USIWM0) | (1 << USICS1) |
           (0 << USICS0) | (0 << USICLK);
}

/*
 *  Resets the serial ptr to start with the start of the buffer
 */
void reset_serial_ptr(uint8_t size) {
   current_serial_data = 0;
   serial_buffer_size = size;
   USIDR = serial_buffer[current_serial_data];
   USISR = (1 << USIOIF);
}

/*
 *  Initializes the Comparator for use measuring an RC Time constant
 */
void comparator_init(void) {
   ACSR |= (1 << ACD);     // Ensure Comparator is disabled
   ACSR &= ~(1 << ACIE);   // Disable Interrupts
   ACSR |= (1 << ACBG);    // Use bandgap as positive input
   ADCSRA &= ~(1 << ADEN); // Disable ADC
   ADCSRB |= (1 << ACME);  // Enable Multiplexer
   // Select ADC2 for negative input
   ADMUX = (ADMUX & ~0x0F) | (1 << MUX1) | (0 << MUX0);
   // Use Falling Edge as interrupt source
   ACSR = (ACSR & ~0b11) | (1 << ACIS1) | (0 << ACIS0);

   ACSR &= ~(1 << ACD); // Enable Comparator
   // TODO: Maybe add bandgap settle time
   ACSR |= (1 << ACIE); // Enable Interrupts
}

/*
 *  Initializes the Timer0 for use measuring an RC Timer Constant
 */
void timer0_init(void) {
   GTCCR |= (1 << TSM); // Synchronize Timer
   TCCR0A &= ~(0xFF);   // Normal Mode
   // Use CLK_IO (1Mhz)
   TCCR0B = (TCCR0B & ~0x07) | (0 << CS02) | (0 << CS01) | (1 << CS00);
   TIMSK |= (1 << TOIE0); // Enable Overflow Interrupt
}

/*
 *  Starts the Timer0 Timer
 *  Interrupts must be enabled
 */
void timer0_start_counting(void) {
   TIMER0_overflow_counter = 0;
   GTCCR |= (1 << TSM);  // Ensure Timer is Synchronized
   TCNT0 = 0;            // Reset the Counter
   GTCCR |= (1 << PSR0); // Reset the Prescaler
   GTCCR &= ~(1 << TSM); // Disable Synchronization; start timer
}

/*
 *  Initializes Required Peripherals for measuring an RC Time Constant.
 */
void rc_measurement_init(void) {
   timer0_init();
   comparator_init();

   DDRB |= (1 << PB3);   // Initialize PB3 as output
   PORTB &= ~(1 << PB3); // Drive PB3 low
}

/*
 *  Begins an RC measurement
 */
void rc_measurement_begin(void) {
   rc_measurement_flag = false;
   timer0_start_counting();
   PORTB |= (1 << PB3);
}

/*
 *  Checks if the RC measurement has completed and is valid.
 */
bool rc_measurement_check_valid(void) { return rc_measurement_flag; }

/*
 *  Get the most recent RC measurement
 */
uint32_t rc_measurement_get(void) { return TIMER0_overflow_counter; }

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

ISR(TIMER0_OVF_vect) { TIMER0_overflow_counter++; }

ISR(ANA_COMP_vect) {
   GTCCR |= (1 << TSM);        // Synchronize Timer
   rc_measurement_flag = true; // Notify
   PORTB &= ~(1 << PB3);       // Start Discharging Capacitor
}

ISR(USI_OVF_vect) {
   current_serial_data++;
   if (current_serial_data >= serial_buffer_size) {
      current_serial_data = 0;
   }
   USIDR = serial_buffer[current_serial_data];
   USISR = (1 << USIOIF);
}
