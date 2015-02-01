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

static uint8_t  PpmFrameInputPin;
static uint8_t  VirtualPort;

static volatile uint8_t  Synchro = 0;
static volatile uint16_t ChWidthUs[TINY_PPM_READER_CH_MAX];
static volatile uint8_t  ChIdx = (TINY_PPM_READER_CH_MAX + 1), ChIdxMax = 0; /* +1 for startup condition */
static volatile uint16_t PrevEdgeUs;

/* Private function prototype */
static void RcChannelCollectorIsr();


/* Public functions */
void TinyPpmReader_Init(uint8_t PpmInputPin)
{
  PpmFrameInputPin = PpmInputPin;
  TinyPinChange_Init();
  VirtualPort  = TinyPinChange_RegisterIsr(PpmFrameInputPin, RcChannelCollectorIsr);
  for(uint8_t Idx = 0; Idx < TINY_PPM_READER_CH_MAX; Idx++)
  {
    ChWidthUs[Idx] = NEUTRAL_US;
  }
  TinyPinChange_EnablePin(PpmFrameInputPin);
}

uint8_t TinyPpmReader_DetectedChannelNb(void)
{
  uint8_t ChannelNb;
  
  cli();
  ChannelNb = ChIdxMax;
  sei();
  if(ChannelNb > TINY_PPM_READER_CH_MAX) ChannelNb = 0;
  return(ChannelNb);
}

uint16_t TinyPpmReader_Width_us(uint8_t ChIdx)
{
  uint16_t Width_us;
  
  cli();
  Width_us = ChWidthUs[ChIdx];
  sei();
  
  return(Width_us);
}

uint8_t TinyPpmReader_IsSynchro(void)
{
  uint8_t Ret;
  
  Ret = Synchro;
  Synchro = 0;
  
  return(Ret);
}

void TinyPpmReader_Suspend(void)
{
  TinyPinChange_DisablePin(PpmFrameInputPin);
  ChIdx = (TINY_PPM_READER_CH_MAX + 1);
}

void TinyPpmReader_Resume(void)
{
  PrevEdgeUs = (uint16_t)(micros() & 0xFFFF);
  TinyPinChange_EnablePin(PpmFrameInputPin);
}

/* Private function */
static void RcChannelCollectorIsr(void)
{
  uint16_t CurrentEdgeUs, PulseDurationUs;

  if(TinyPinChange_FallingEdge(VirtualPort, PpmFrameInputPin))
  {
    CurrentEdgeUs   = (uint16_t)(micros() & 0xFFFF);
    PulseDurationUs = (uint16_t)(CurrentEdgeUs - PrevEdgeUs);
    PrevEdgeUs      = CurrentEdgeUs;
    if(PulseDurationUs >= SYNCHRO_TIME_MIN_US)
    {
      ChIdxMax = ChIdx;
      ChIdx    = 0;
      Synchro  = 1; /* Synchro detected */
    }
    else
    {
      if(ChIdx < TINY_PPM_READER_CH_MAX)
      {
        ChWidthUs[ChIdx] = PulseDurationUs;
        ChIdx++;
      }
    }
  }
}
