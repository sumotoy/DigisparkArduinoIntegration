/*
TinyPpmGenSweep sketch
by RC Navy (http://p.loussouarn.free.fr/arduino/arduino.html) 2015
This sketch generates an RC PPM frame transporting 4 RC channels.
The 3 first channels have fixed pulse width (tunable), and the 4th channel sweeps between 1000 and 2000 us.
It can be extended up to 8 RC channels.
This sketch can work with a Digispark pro, Digispark, Arduino UNO...

PPM output pin is imposed by hardware and is target dependant:
- ATtiny167:  (Digispark pro: PPM output -> PA2 -> arduino pin#8)
- ATtiny85:   (Digispark:     PPM output -> PB0 -> arduino pin#0)
- ATmega328P: (Arduino UNO:   PPM output -> PD6 -> arduino pin#6)

This example code is in the public domain.
*/
#include <TinyPpmGen.h>

#define CH_MAX_NB  4

#define STEP_US    5

#define PULSE_WIDTH_MIN_US    1000
#define PULSE_WIDTH_MAX_US    2000

uint16_t Width_us = PULSE_WIDTH_MAX_US;
uint16_t Step_us  = STEP_US;

void setup()
{
  TinyPpmGen_Init(TINY_PPM_GEN_POS_MOD, CH_MAX_NB); /* Change TINY_PPM_GEN_POS_MOD to TINY_PPM_GEN_NEG_MOD for NEGative PPM modulation */
  TinyPpmGen_SetChWidth_us(1, 500);  /* RC Channel#1 */
  TinyPpmGen_SetChWidth_us(2, 1000); /* RC Channel#2 */
  TinyPpmGen_SetChWidth_us(3, 1500); /* RC Channel#3 */
  TinyPpmGen_SetChWidth_us(4, 2000); /* RC Channel#4 */
}

void loop()
{
  TinyPpmGen_SetChWidth_us(CH_MAX_NB, Width_us); /* Sweep RC Channel#4 */
  Width_us += Step_us;
  if(Width_us > PULSE_WIDTH_MAX_US) Step_us = -STEP_US;
  if(Width_us < PULSE_WIDTH_MIN_US) Step_us = +STEP_US;
  delay(10);
}