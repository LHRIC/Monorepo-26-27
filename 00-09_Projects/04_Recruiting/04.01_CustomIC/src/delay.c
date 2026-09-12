#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>

// Requires F_CPU to be defined (e.g. 8000000UL)
#define F_CPU 80000000

// Picks the smallest prescaler that lets the tick count fit in 8 bits
static uint8_t timer1_select_prescaler(uint32_t ticks_needed,
                                       uint16_t *prescaler_out) {
   static const uint16_t prescalers[] = {
       1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384};
   for (uint8_t i = 0; i < 15; i++) {
      if (ticks_needed / prescalers[i] <= 255) {
         *prescaler_out = prescalers[i];
         return i + 1; // CS13:CS10 bit pattern (1-15)
      }
   }
   *prescaler_out = 16384;
   return 15;
}

// Blocking delay, accurate to within one prescaler tick. Max ~255 * 16384
// timer ticks per call; for longer delays, call in a loop (see
// timer1_delay_ms).
void timer1_delay_us(uint16_t us) {
   TCCR1 = 0;            // Stop timer
   GTCCR |= (1 << PSR1); // Reset Timer1 prescaler
   TCNT1 = 0;            // Reset counter
   TIFR |= (1 << OCF1A); // Clear any pending compare match flag

   uint32_t ticks = ((uint32_t)F_CPU / 1000000UL) * us;
   if (ticks == 0)
      ticks = 1;

   uint16_t prescaler;
   uint8_t cs_bits = timer1_select_prescaler(ticks, &prescaler);

   uint32_t count = ticks / prescaler;
   if (count == 0)
      count = 1;
   if (count > 255)
      count = 255;

   OCR1A = (uint8_t)count;
   OCR1C = (uint8_t)count;        // CTC1 clears TCNT1 on match with OCR1C
   TCCR1 = (1 << CTC1) | cs_bits; // Start timer in CTC mode

   while (!(TIFR & (1 << OCF1A)))
      ;

   TCCR1 = 0;            // Stop timer
   TIFR |= (1 << OCF1A); // Clear flag
}

void timer1_delay_ms(uint16_t ms) {
   while (ms--) {
      timer1_delay_us(1000);
   }
}
