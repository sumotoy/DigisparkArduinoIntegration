#ifndef TINY_PPM_GEN
#define TINY_PPM_GEN 1
/* A tiny interrupt driven RC PPM frame generator library using compare match of the timer used for ms in the arduino core
   Features:
   - Uses Output Compare Channel A of the 8 bit Timer 0 (When used, disables PWM associated to Timer 0 -> Pin marked as "OC0A" shall be used as PPM Frame output (no other choice) 
   - Can generate a PPM Frame containing up to 8 RC Channels (600 -> 2000 us) or 7 RC Channels (600 -> 2400 us)
   - Positive or Negative Modulation supported
   - Constant PPM Frame period: 20 ms
   - No need to wait 20 ms to set the pulse width order for the channels, can be done at any time
   - Synchronisation indicator for digital data transmission over PPM
   - Blocking fonctions such as delay() can be used in the loop() since it's an interrupt driven PPM generator
   - Supported devices:
       - ATtiny167  (Digispark pro: PPM output -> PA2 -> arduino pin#8)
       - ATtiny85   (Digispark:     PPM output -> PB0 -> arduino pin#0)
       - ATmega328P (Arduino UNO:   PPM output -> PD6 -> arduino pin#6)
   RC Navy 2015
   http://p.loussouarn.free.fr
   31/01/2015: Creation
*/
#include <Arduino.h>

#define TINY_PPM_GEN_POS_MOD          HIGH
#define TINY_PPM_GEN_NEG_MOD          LOW


/* Public function prototypes */
uint8_t TinyPpmGen_Init(uint8_t PpmModu, uint8_t ChNb);
void    TinyPpmGen_SetChWidth_us(uint8_t ChIdx, uint16_t Width_us);
uint8_t TinyPpmGen_IsSynchro(void);

#endif