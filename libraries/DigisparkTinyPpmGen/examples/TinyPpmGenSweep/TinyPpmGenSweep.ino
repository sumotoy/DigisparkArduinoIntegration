/*
TinyPpmGenSweep sketch
by RC Navy (http://p.loussouarn.free.fr/arduino/arduino.html>) 2015
This sketch generates an RC PPM frame transporting 4 RC channels.
The 3 first channels have fixed pulse width (tunable), and the 4th channel sweeps between 1000 and 2000 us.
It can be extend up to 8 RC channels.
This sketch can work with and Arduino UNO, Digispark pro, Digispark, etc...
This example code is in the public domain.
*/
#include <TinyPpmGen.h>

#define CH_MAX_NB  4

#define STEP_US    5

#define PULSE_WIDTH_MIN_US    1000
#define PULSE_WIDTH_MAX_US    2000

uint16_t Width_us = PULSE_WIDTH_MIN_US;
uint16_t Step_us  = STEP_US;

void setup()
{
  TinyPpmGen_Init(TINY_PPM_GEN_POS_MOD, CH_MAX_NB);
  TinyPpmGen_SetChWidth_us(1, 500);
  TinyPpmGen_SetChWidth_us(2, 1000);
  TinyPpmGen_SetChWidth_us(3, 1500);
  TinyPpmGen_SetChWidth_us(4, 2000);
}

void loop()
{
  TinyPpmGen_SetChWidth_us(CH_MAX_NB, Width_us);
  Width_us += Step_us;
  if(Width_us > PULSE_WIDTH_MAX_US) Step_us = -STEP_US;
  if(Width_us < PULSE_WIDTH_MIN_US) Step_us = +STEP_US;
  delay(10);
}