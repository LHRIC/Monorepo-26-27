#include "HardwareSerial.h"
#include <Arduino.h>
#include <stdint.h>

#define CLK_PIN 2
#define DATA_PIN 3

void clk_pulse() {
   digitalWrite(CLK_PIN, 1);
   digitalWrite(CLK_PIN, 0);
}

uint8_t read_byte() {
   uint8_t byte = 0;
   for (int i = 7; i >= 0; i--) {
      int bit = digitalRead(DATA_PIN);
      byte |= bit << i;
      clk_pulse();
   }
   return byte;
}

void setup() {
   Serial.begin(9600);
   pinMode(LED_BUILTIN, OUTPUT);
   pinMode(CLK_PIN, OUTPUT);
   pinMode(DATA_PIN, INPUT);
   digitalWrite(CLK_PIN, LOW);
   delay(1000);
}

void loop() {
   char letter = read_byte();
   Serial.write(letter);
}
