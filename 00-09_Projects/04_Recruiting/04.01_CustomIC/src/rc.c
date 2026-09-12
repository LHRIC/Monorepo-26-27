#include "rc.h"

#include <avr/cpufunc.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "delay.h"

volatile uint32_t TIMER0_overflow_counter = 0;

volatile bool rc_measurement_flag = false;

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

   ACSR &= ~(1 << ACD);  // Enable Comparator
   timer1_delay_us(100); // Give bandgap settle time
}

/*
 *  Initializes the Timer0 for use measuring an RC Timer Constant
 */
void timer0_init(void) {
   GTCCR |= (1 << TSM); // Synchronize Timer
   TCCR0A &= ~(0xFF);   // Normal Mode
   // Use CLK_IO / 8 -> (1Mhz)
   TCCR0B = (TCCR0B & ~0x07) | (0 << CS02) | (1 << CS01) | (0 << CS00);
   TIMSK |= (1 << TOIE0); // Enable Overflow Interrupt
}

/*
 *  Starts the Timer0 Timer
 *  Interrupts must be enabled
 */
static void timer0_start_counting(void) {
   TIMER0_overflow_counter = 0;
   GTCCR |= (1 << TSM); // Ensure Timer is Synchronized
                        // Restore clock source
   TCCR0B = (TCCR0B & ~0x07) | (0 << CS02) | (1 << CS01) | (0 << CS00);
   TCNT0 = 0;            // Reset the Counter
   GTCCR |= (1 << PSR0); // Reset the Prescaler
   GTCCR &= ~(1 << TSM); // Disable Synchronization; start timer
}

void rc_measurement_init(void) {
   timer0_init();
   comparator_init();

   // Set up ADC
   ADCSRA &= ~(1 << ADATE | 1 << ADIE | 0x7); // Disable settings
   ADMUX &= ~(1 << REFS1 | 1 << REFS0);       // Use 3.3v as reference

   // NOTE:
   // This is awfull. I'm not sure why this is necessary, I think it might be a
   // weird bug with the silicon. The issue is, the analog comparator triggers
   // at the wrong voltage. This is fixed if all of the digital input buffers
   // are disabled. Why? I have no fucking idea. This took me so long to realize
   // and I only know about it because I accidentally disabled all of the
   // buffers when only trying to disable one. If you ever find out why this is
   // the case, email me a johnsaenz@utexas.edu
   DIDR0 |=
       (1 << ADC0D) | (1 << ADC1D) | (1 << ADC2D) | (1 << ADC3D); // Disable
                                                                  // all
                                                                  // digital
                                                                  // buffers

   DDRB |= (1 << PB3);   // Initialize PB3 as output
   PORTB &= ~(1 << PB3); // Drive PB3 low
}

void rc_measurement_begin(void) {
   rc_measurement_flag = false;
   ACSR |= (1 << ACI);  // Clear any Interrupt
   ACSR |= (1 << ACIE); // Enable Interrupts for comparator
   timer0_start_counting();
   PORTB |= (1 << PB3);
}

void rc_discharge_cap(void) {
   ACSR &= ~(1 << ACIE); // Disable Comparator Interrupts
   // ACSR &= ~(1 << ACD);    // Disable Comparator
   ACSR |= (1 << ACI);     // Clear any Interrupt
   ADCSRA &= ~(1 << ADEN); // Ensure ADC is disabled
   ADCSRB &= ~(1 << ACME); // Disable Multiplexer input to comparator
   ADCSRA |= (1 << ADEN);  // Enable ADC
   PORTB &= ~(1 << PB3);   // Ensure PB3 is low

   // Loop till cap is mostly discharged
   while (true) {
      ADCSRA |= (1 << ADIF); // Clear stale flag
      ADCSRA |= (1 << ADSC); // Start ADC Conversion
      while (!(ADCSRA & (1 << ADIF)))
         _NOP();
      uint16_t data = ADCL;
      data += ADCH << 8;
      if (data <= 4) {
         ADCSRA |= (1 << ADIF);
         break;
      }
      ADCSRA |= (1 << ADIF);
   }

   ADCSRA &= ~(1 << ADEN); // Ensure ADC is disabled
   ADCSRB |= (1 << ACME);  // Re-enable Multiplexer input to comparator
   // ACSR |= (1 << ACD);     // Re-enable comparator
   ACSR |= (1 << ACI);   // Clear any Interrupt
   timer1_delay_us(100); // Give bandgap settle time
}

bool rc_measurement_check_valid(void) { return rc_measurement_flag; }

uint32_t rc_measurement_get(void) {
   return (TIMER0_overflow_counter << 8) | TCNT0;
}

ISR(TIMER0_OVF_vect) { TIMER0_overflow_counter++; }

ISR(ANA_COMP_vect) {
   ACSR &= ~(1 << ACIE); // Disable Interrupts
   DIDR0 &= ~(1 << ADC1D);
   TCCR0B &= ~(0x07);          // Stop Timer
   rc_measurement_flag = true; // Notify
   PORTB &= ~(1 << PB3);       // Start Discharging Capacitor
}
