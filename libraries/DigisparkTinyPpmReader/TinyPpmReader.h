#ifndef TINY_PPM_READER
#define TINY_PPM_READER 1
/* A tiny interrupt driven RC PPM frame reader library using pin change interrupt
   Features:
   - Uses any input supporting interrupt pin change
   - Supported devices: see TinyPinChange library
   - Positive and negative PPM modulation supported (don't care)
   - Up to 9 RC channels supported
   RC Navy 2015
   http://p.loussouarn.free.fr
   01/02/2015: Creation
*/
#include <Arduino.h>
#include <TinyPinChange.h>

#define TINY_PPM_READER_CH_MAX  9

/* Public function prototypes */
void     TinyPpmReader_Init(uint8_t PpmInputPin);
uint8_t  TinyPpmReader_DetectedChannelNb(void);
uint16_t TinyPpmReader_Width_us(uint8_t Ch);
uint8_t  TinyPpmReader_IsSynchro(void);
void     TinyPpmReader_Suspend(void);
void     TinyPpmReader_Resume(void);

#endif
