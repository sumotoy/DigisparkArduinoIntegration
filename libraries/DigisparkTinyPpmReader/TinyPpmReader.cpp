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
#include <TinyPpmReader.h>


/*
EXAMPLE OF POSITIVE AND NEGATIVE PPM FRAME TRANSPORTING 2 RC CHANNELS
=====================================================================

 Positive PPM
       .-----.                 .-----.         .-----.                                  .-----.                 .-----.         .-----. 
       |     |                 |     |         |     |                                  |     |                 |     |         |     |
       |     |                 |     |         |     |                                  |     |                 |     |         |     |
       |     |                 |     |         |     |                                  |     |                 |     |         |     |
    ---'     '-----------------'     '---------'     '----------------//----------------'     '-----------------'     '---------'     '----
       <-----------------------><--------------><---------------------//---------------><-----------------------><-------------->
               Channel#1           Channel#2                       Synchro                       Channel#1           Channel#2
             <-----------------------><--------------><---------------------//---------------><-----------------------><-------------->
                     Channel#1           Channel#2                       Synchro                       Channel#1           Channel#2
                        
 Negative PPM
    ---.     .-----------------.     .---------.     .----------------//----------------.     .-----------------.     .---------'     .----       
       |     |                 |     |         |     |                                  |     |                 |     |         |     |
       |     |                 |     |         |     |                                  |     |                 |     |         |     |
       |     |                 |     |         |     |                                  |     |                 |     |         |     |
       '-----'                 '-----'         '-----'                                  '-----'                 '-----'         '-----' 
       <-----------------------><--------------><---------------------//---------------><-----------------------><-------------->
               Channel#1           Channel#2                       Synchro                       Channel#1           Channel#2
             <-----------------------><--------------><---------------------//---------------><-----------------------><-------------->
                     Channel#1           Channel#2                       Synchro                       Channel#1           Channel#2

 The channel durations (Pulse width) are located between 2 rising edges.
 Please, note the same channel durations (Pulse width) are obtained between 2 falling edges.
 The Synchro pulse shall be longer than the longer RC Channel pulse width.
*/

#define NEUTRAL_US           1500
#define SYNCHRO_TIME_MIN_US  3000

TinyPpmReader *TinyPpmReader::first;

/* Public functions */
TinyPpmReader::TinyPpmReader(void) /* Constructor */
{
}

uint8_t TinyPpmReader::attach(uint8_t PpmInputPin)
{
  uint8_t Ret = 0;
  
  TinyPinChange_Init();
  _VirtualPort = TinyPinChange_RegisterIsr(PpmInputPin, TinyPpmReader::rcChannelCollectorIsr);
  if(_VirtualPort >= 0)
  {
    for(uint8_t Idx = 0; Idx < TINY_PPM_READER_CH_MAX; Idx++)
    {
      _ChWidthUs[Idx] = NEUTRAL_US;
    }
    _ChIdx    = (TINY_PPM_READER_CH_MAX + 1);
    _ChIdxMax = 0;
    _Synchro  = 0;
    next = first;
    first = this;
    _PpmFrameInputPin = PpmInputPin;
    _PinMask = TinyPinChange_PinToMsk(_PpmFrameInputPin);
    TinyPinChange_EnablePin(_PpmFrameInputPin);
    Ret = 1;
  }
  return(Ret);
}

uint8_t TinyPpmReader::detectedChannelNb(void)
{
  uint8_t ChannelNb;
  
  cli();
  ChannelNb = _ChIdxMax;
  sei();
  if(ChannelNb > TINY_PPM_READER_CH_MAX) ChannelNb = 0;
  return(ChannelNb);
}

uint16_t TinyPpmReader::width_us(uint8_t Ch)
{
  uint16_t Width_us = 0;
  if(Ch >= 1 && Ch <= TinyPpmReader::detectedChannelNb())
  {
    Ch--;
    cli();
    Width_us = _ChWidthUs[Ch];
    sei();
  }
  return(Width_us);
}

uint16_t TinyPpmReader::ppmPeriod_us(void)
{
  uint16_t PpmPeriod_us = 0;
  cli();
  PpmPeriod_us = _PpmPeriodUs;
  sei();
  return(PpmPeriod_us);
}

uint8_t TinyPpmReader::isSynchro(uint8_t SynchroClientMsk /*= TINY_PPM_READER_CLIENT(7)*/)
{
  uint8_t Ret;
  
  Ret = !!(_Synchro & SynchroClientMsk);
  if(Ret) _Synchro &= ~SynchroClientMsk; /* Clear indicator for the Synchro client */
  
  return(Ret);
}

void TinyPpmReader::suspend(void)
{
  TinyPinChange_DisablePin(_PpmFrameInputPin);
  _ChIdx = (TINY_PPM_READER_CH_MAX + 1);
}

void TinyPpmReader::resume(void)
{
  _PrevEdgeUs = (uint16_t)(micros() & 0xFFFF);
  TinyPinChange_EnablePin(_PpmFrameInputPin);
}

/* ISR */
void TinyPpmReader::rcChannelCollectorIsr(void)
{
  TinyPpmReader *PpmReader;
  uint16_t CurrentEdgeUs, PulseDurationUs;
  static uint8_t Period = false;
  
  for ( PpmReader = first; PpmReader != 0; PpmReader = PpmReader->next )
  {
    if(TinyPinChange_FallingEdge(PpmReader->_VirtualPort, PpmReader->_PpmFrameInputPin))
    {
      CurrentEdgeUs   = (uint16_t)(micros() & 0xFFFF);
      PulseDurationUs = (uint16_t)(CurrentEdgeUs - PpmReader->_PrevEdgeUs);
      PpmReader->_PrevEdgeUs      = CurrentEdgeUs;
      if(PulseDurationUs >= SYNCHRO_TIME_MIN_US)
      {
	PpmReader->_ChIdxMax = PpmReader->_ChIdx;
	PpmReader->_ChIdx    = 0;
	PpmReader->_Synchro  = 0xFF; /* Synchro detected */
	Period = !Period;
	if(Period) PpmReader->_StartPpmPeriodUs = CurrentEdgeUs;
	else       PpmReader->_PpmPeriodUs      = CurrentEdgeUs - PpmReader->_StartPpmPeriodUs;
      }
      else
      {
	if(PpmReader->_ChIdx < TINY_PPM_READER_CH_MAX)
	{
	  PpmReader->_ChWidthUs[PpmReader->_ChIdx] = PulseDurationUs;
	  PpmReader->_ChIdx++;
	}
      }
    }
  }
}
