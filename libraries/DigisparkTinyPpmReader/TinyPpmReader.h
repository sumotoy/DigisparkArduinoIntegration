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
   06/04/2015: RcTxPop support added (allows to create a virtual serial port over a PPM channel)
   09/11/2015: No need to create the TinyPpmReader object anymore, unused _PinMask variable removed
*/
#include <Arduino.h>
#include <TinyPinChange.h>
#include <RcRxPop.h>

#define TINY_PPM_READER_CH_MAX  9

/* Macro for PPM Reader client */
#define TINY_PPM_READER_CLIENT(ClientIdx) (1 << (ClientIdx)) /* Range: 0 to 7 */

/* Public function prototypes */
class TinyPpmReaderClass : public RcRxPop
{
  public:
    TinyPpmReaderClass();
    uint8_t  attach(uint8_t PpmInputPin);
    uint8_t  detectedChannelNb(void);
    uint16_t width_us(uint8_t Ch);
    uint16_t ppmPeriod_us(void);
    uint8_t  isSynchro(uint8_t SynchroClientMsk = TINY_PPM_READER_CLIENT(7)); /* Default value: 8th Synchro client -> 0 to 6 free for other clients*/
    void     suspend(void);
    void     resume(void);
    static void rcChannelCollectorIsr(void);
    /* RcRxPop support */
    virtual uint8_t  RcRxPopIsSynchro();
    virtual uint16_t RcRxPopGetWidth_us(uint8_t Ch);
  private:
    // static data
    uint8_t  _PpmFrameInputPin;
    int8_t   _VirtualPort;
    volatile uint8_t  _Synchro;
    volatile uint16_t _ChWidthUs[TINY_PPM_READER_CH_MAX];
    volatile uint8_t  _ChIdx;
    volatile uint8_t  _ChIdxMax;
    volatile uint16_t _PrevEdgeUs;
    volatile uint16_t _StartPpmPeriodUs;
    volatile uint16_t _PpmPeriodUs;
};

extern TinyPpmReaderClass TinyPpmReader; /* Object externalisation */

#endif
