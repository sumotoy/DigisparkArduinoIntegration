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
#include <TinyPpmGen.h>

#ifndef MS_TIMER_TICK_EVERY_X_CYCLES /* SYMBOL introduced to the modified arduino distribution for Digispark */
#define MS_TIMER_TICK_EVERY_X_CYCLES 64 /* Default value for regular arduino distribution */
#endif

#if (MS_TIMER_TICK_EVERY_X_CYCLES < 16/*clockCyclesPerMicrosecond()*/)
#error MS_TIMER_TICK_EVERY_X_CYCLES too low!
#endif

#define MS_TIMER_TICK_DURATION_US  (MS_TIMER_TICK_EVERY_X_CYCLES / clockCyclesPerMicrosecond())

#if (F_CPU == 16500000)
#define PPM_US_TO_TICK(Us)         (((Us) + ((Us) >> 5)) / MS_TIMER_TICK_DURATION_US) /* Correction for Digispark at 16.5 MHz */
#else
#define PPM_US_TO_TICK(Us)         ((Us) / MS_TIMER_TICK_DURATION_US)
#endif

#ifdef __AVR_ATtiny167__
#define PPM_OCR_PORT               PORTA
#define PPM_OCR_PIN                PINA
#define PPM_OCR_PIN_MSK            (1 << 2) /* PA2 */
#define PPM_OCR_DDR                DDRA
#else
#ifdef  __AVR_ATtiny85__
#define PPM_OCR_PORT               PORTB
#define PPM_OCR_PIN                PINB
#define PPM_OCR_PIN_MSK            (1 << 0) /* PB0 */
#define PPM_OCR_DDR                DDRB
#else
/* Last supported target is assumed to be Atmega328p (UNO) */
#define PPM_OCR_PORT               PORTD
#define PPM_OCR_PIN                PIND
#define PPM_OCR_PIN_MSK            (1 << 6) /* PD6 */
#define PPM_OCR_DDR                DDRD
#endif
#endif

#ifndef TIMSK0
#define TIMSK0                     TIMSK
#endif

#define COMP_VECT                  TIMER0_COMPA_vect
#define PPM_OCR                    OCR0A
#define TIM_CTRL_REG1              TCCR0A
#define TIM_CTRL_REG2              TCCR0B
#define TIM_IT_MSK_REG             TIMSK0
#define OCIE_MASK                  _BV(OCIE0A)
#define OCR_FORCE_MASK             _BV(FOC0A)

#define PPM_OCR_INT_ENABLE()       TIM_IT_MSK_REG |= OCIE_MASK
#define PPM_OCR_INT_DISABLE()      TIM_IT_MSK_REG &= ~OCIE_MASK

#define PPM_OCR_FORCE()            TIM_CTRL_REG2 |= OCR_FORCE_MASK

#ifdef WGM02
#define TIM_MODE_NORMAL()         (TIM_CTRL_REG1 &= ~(_BV(WGM01) | _BV(WGM00)));(TIM_CTRL_REG2 &= ~(_BV(WGM02)))
#else
#define TIM_MODE_NORMAL()         (TIM_CTRL_REG1 &= ~(_BV(WGM01) | _BV(WGM00)))
#endif

#define TOGGLE_PPM_PIN_DISABLE()  (TIM_CTRL_REG1 &= ~(_BV(COM0A1) | _BV(COM0A0)))
#define TOGGLE_PPM_PIN_ENABLE()   (TIM_CTRL_REG1 |= _BV(COM0A0))

#define FULL_OVF_MASK             0x7F
#define HALF_OVF_MASK             0x80
#define HALF_OVF_VAL              128

#define PPM_NEUTRAL_US            2000

#define PPM_FRAME_PERIOD_US       20000

#define PPM_US_GUARD              64 /* Time to be sure to have time to prepare remaing ticks */

/*
Positive PPM    .-----.                         .-----.
                |     |                         |     |
                |  P  |  F     F     F    H   R |  P  |
                |     |                         |     |
             ---'     '-----x-----x-----x---x---'     '-----
                <------------------------------->
                        Channel Duration
                        
Negative PPM ---.     .-----x-----x-----x---x---.     .-----
                |     |                         |     |
                |  P  |  F     F     F    H   R |  P  |
                |     |                         |     |
                '-----'                         '-----'
                <------------------------------->
                        Channel Duration
Legend:
======
 P: Pulse header duration in ticks (Nb of Ticks corresponding to 256 us)
 F: Full timer overflow (256 Ticks)
 H: Half timer overflow (128 Ticks)
 R: Remaining Ticks to complete the full Channel duration
*/

typedef struct {
  uint8_t Ovf;
  uint8_t Rem;
}OneChSt_t;

typedef struct {
  OneChSt_t Cur;
  OneChSt_t Next;
}ChSt_t;

/* Global variables */
static volatile ChSt_t* _Ch = NULL;
static volatile uint8_t _Synchro = 0;
static volatile uint8_t _StartOfFrame = 1;
static volatile uint8_t _Idx = 0;
static volatile uint8_t _ChMaxNb;

OneTinyPpmGen TinyPpmGen = OneTinyPpmGen();

/* Public functions */
OneTinyPpmGen::OneTinyPpmGen(void) /* Constructor */
{
  
}

uint8_t OneTinyPpmGen::begin(uint8_t PpmModu, uint8_t ChNb)
{
  boolean Ok = false;
  uint8_t Idx;

  _ChMaxNb = (ChNb > 8)?8:ChNb; /* Max is 8 Channels for a PPM period of 20 ms */
  _Ch = (ChSt_t *)malloc(sizeof(ChSt_t) * (_ChMaxNb + 1)); /* + 1 for Synchro Channel */
  Ok = (_Ch != NULL);
  if (Ok)
  {
    for(Idx = 1; Idx <= _ChMaxNb; Idx++)
    {
      OneTinyPpmGen::setChWidth_us(Idx, PPM_NEUTRAL_US); /* Set all channels to Neutral */
    }
    /* Set Pin as Output according to the PPM modulation level */
    PPM_OCR_DDR |= PPM_OCR_PIN_MSK; /* Set pin as output */
    if(PpmModu == TINY_PPM_GEN_NEG_MOD)
    {
      PPM_OCR_PIN |= PPM_OCR_PIN_MSK; /*Set pin to high */
    }
    else
    {
      PPM_OCR_PIN &= ~PPM_OCR_PIN_MSK; /*Set pin to low */
    }
    _Idx = _ChMaxNb; /* To reload values at startup */
    TIM_MODE_NORMAL();
    TOGGLE_PPM_PIN_ENABLE();
    if(PpmModu == TINY_PPM_GEN_NEG_MOD) PPM_OCR_FORCE(); /* Force Output Compare to initialize properly the output */
    PPM_OCR_INT_ENABLE();
  }
  return(Ok);
}

void OneTinyPpmGen::setChWidth_us(uint8_t Ch, uint16_t Width_us)
{
  uint16_t TickNb, SumTick = 0, SynchTick;
  uint8_t  Ch_Next_Ovf, Ch_Next_Rem, Ch0_Next_Ovf, Ch0_Next_Rem;

  if((Ch >= 1) && (Ch <= _ChMaxNb))
  {
    TickNb = PPM_US_TO_TICK(Width_us - 256 + (MS_TIMER_TICK_DURATION_US / 2)); /* Convert in rounded Timer Ticks. 256: Should be normally around 300 us, but works fine with 256 us */
    Ch_Next_Ovf = (TickNb & 0xFF00) >> 8;
    Ch_Next_Rem = (TickNb & 0x00FF);
    if(Ch_Next_Rem < PPM_US_TO_TICK(PPM_US_GUARD))
    {
      Ch_Next_Ovf |= HALF_OVF_MASK;
      Ch_Next_Rem += HALF_OVF_VAL;
    }
    if(Ch_Next_Rem > (256 - PPM_US_TO_TICK(PPM_US_GUARD)))
    {
      Ch_Next_Ovf++;
      Ch_Next_Ovf |= HALF_OVF_MASK;
      Ch_Next_Rem -= HALF_OVF_VAL;
    }
    /* Update Synchro Time */
    for(uint8_t Idx = 1; Idx <= _ChMaxNb; Idx++)
    {
      if(Idx != Ch)
      {
        SumTick += PPM_US_TO_TICK(256) + ((_Ch[Idx].Cur.Ovf & FULL_OVF_MASK) << 8) + _Ch[Idx].Next.Rem;
        if(_Ch[Idx].Cur.Ovf & HALF_OVF_MASK) SumTick -= HALF_OVF_VAL;
      }
      else
      {
        SumTick += ((Ch_Next_Ovf & FULL_OVF_MASK) << 8) + Ch_Next_Rem;
        if(Ch_Next_Ovf & HALF_OVF_MASK) SumTick -= HALF_OVF_VAL;
      }
    }
    SynchTick = PPM_US_TO_TICK(PPM_FRAME_PERIOD_US) - SumTick - PPM_US_TO_TICK(256);
    Ch0_Next_Ovf = (SynchTick & 0xFF00) >> 8;
    Ch0_Next_Rem = (SynchTick & 0x00FF);
    if(Ch0_Next_Rem < PPM_US_TO_TICK(PPM_US_GUARD))
    {
      Ch0_Next_Ovf |= HALF_OVF_MASK;
      Ch0_Next_Rem += HALF_OVF_VAL;
    }
    if(Ch0_Next_Rem > (256 - PPM_US_TO_TICK(PPM_US_GUARD)))
    {
      Ch0_Next_Ovf++;
      Ch0_Next_Ovf |= HALF_OVF_MASK;
      Ch0_Next_Rem -= HALF_OVF_VAL;
    }
    /* Update requested Channel AND Synchro to keep constant the period (20ms) */
    PPM_OCR_INT_DISABLE();
    _Ch[0].Next.Ovf = Ch0_Next_Ovf;
    _Ch[0].Next.Rem = Ch0_Next_Rem;
    _Ch[Ch].Next.Ovf = Ch_Next_Ovf;
    _Ch[Ch].Next.Rem = Ch_Next_Rem;
    PPM_OCR_INT_ENABLE(); 
  }
}

uint8_t OneTinyPpmGen::isSynchro(uint8_t SynchroClientMsk /*= TINY_PPM_GEN_CLIENT(7)*/)
{
  uint8_t Ret;
  
  Ret = !!(_Synchro & SynchroClientMsk);
  _Synchro &= ~SynchroClientMsk; /* Clear indicator for the Synchro client */
  
  return(Ret);
}

SIGNAL(COMP_VECT)
{
  if(_StartOfFrame)
  {
     /* Modify PPM_OCR only if Tick > 1 us */
#if (MS_TIMER_TICK_DURATION_US > 1)
    PPM_OCR += PPM_US_TO_TICK(256);
#endif
   /* Next Channel or Synchro */
    _Idx++;
    if(_Idx > _ChMaxNb)
    {
      /* End of PPM Frame */
      _Idx = 0;
      /* Reload new Channel values including Synchro */
      for(uint8_t Idx = 0; Idx <= _ChMaxNb; Idx++)
      {
        _Ch[Idx].Cur.Ovf = _Ch[Idx].Next.Ovf;
        _Ch[Idx].Cur.Rem = _Ch[Idx].Next.Rem;
      }
      _Synchro = 0xFF; /* OK: Widths loaded */
    }
    /* Generate Next Channel or Synchro */
    _Ch[0].Cur.Ovf = _Ch[_Idx].Cur.Ovf;
    _Ch[0].Cur.Rem = _Ch[_Idx].Cur.Rem;
    _StartOfFrame = 0;
  }
  else
  {
    /* Do not change PPM_OCR to have a full Ovf */
    if(_Ch[0].Cur.Rem)
    {
      PPM_OCR += _Ch[0].Cur.Rem;  /* Remain to generate */
      _Ch[0].Cur.Rem = 0;
    }
    else
    {
      if(_Ch[0].Cur.Ovf)
      {
        if(_Ch[0].Cur.Ovf & HALF_OVF_MASK)
        {
          _Ch[0].Cur.Ovf &= FULL_OVF_MASK;   /* Clear Half ovf indicator */
          PPM_OCR += HALF_OVF_VAL; /* Half Overflow to generate */
        }
        _Ch[0].Cur.Ovf--;
      }
    }
    if(!_Ch[0].Cur.Ovf && !_Ch[0].Cur.Rem)
    {
      TOGGLE_PPM_PIN_ENABLE();
      _StartOfFrame = 1;
    }
    else
    {
      TOGGLE_PPM_PIN_DISABLE();      
    }
  }
}
