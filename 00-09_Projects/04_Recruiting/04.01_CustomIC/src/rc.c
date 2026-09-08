#include "rc.h"

#include <avr/interrupt.h>
#include <avr/io.h>

volatile uint32_t TIMER0_overflow_counter = 0;

volatile bool rc_measurement_flag = false;

/*
 *  Initializes the Comparator for use measuring an RC Time constant
 */
static void comparator_init(void) {
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
static void timer0_init(void) {
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
static void timer0_start_counting(void) {
   TIMER0_overflow_counter = 0;
   GTCCR |= (1 << TSM);  // Ensure Timer is Synchronized
   TCNT0 = 0;            // Reset the Counter
   GTCCR |= (1 << PSR0); // Reset the Prescaler
   GTCCR &= ~(1 << TSM); // Disable Synchronization; start timer
}

void rc_measurement_init(void) {
   timer0_init();
   comparator_init();

   DDRB |= (1 << PB3);   // Initialize PB3 as output
   PORTB &= ~(1 << PB3); // Drive PB3 low
}

void rc_measurement_begin(void) {
   rc_measurement_flag = false;
   timer0_start_counting();
   PORTB |= (1 << PB3);
}

bool rc_measurement_check_valid(void) { return rc_measurement_flag; }

uint32_t rc_measurement_get(void) { return TIMER0_overflow_counter; }

ISR(TIMER0_OVF_vect) { TIMER0_overflow_counter++; }

ISR(ANA_COMP_vect) {
   GTCCR |= (1 << TSM);        // Synchronize Timer
   rc_measurement_flag = true; // Notify
   PORTB &= ~(1 << PB3);       // Start Discharging Capacitor
}
