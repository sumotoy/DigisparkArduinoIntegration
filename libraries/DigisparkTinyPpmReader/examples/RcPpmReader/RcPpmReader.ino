/*
RcPpmReader sketch
by RC Navy (http://p.loussouarn.free.fr/arduino/arduino.html) 2015
This sketch reads an RC PPM frame extracts the numbers of channels and their pulse witdhs.
This sketch can work with a Digispark pro, Digispark, Arduino UNO...
The PPM input shall support pin change interrupt.
This example code is in the public domain.
*/
#include <TinyPinChange.h>
#include <TinyPpmReader.h>

#define PPM_INPUT_PIN  2

void setup()
{
  TinyPpmReader_Init(PPM_INPUT_PIN);
  Serial.begin(115200);
}

void loop()
{
  TinyPpmReader_Suspend(); /* Not needed if an hardware serial is used to display results */
  Serial.print(F("ChNb="));Serial.println((int)TinyPpmReader_DetectedChannelNb());
  for(uint8_t Idx = 0; Idx < TinyPpmReader_DetectedChannelNb(); Idx++)
  {
    Serial.println(TinyPpmReader_Width_us(Idx));
  }
  TinyPpmReader_Resume(); /* Not needed if an hardware serial is used to display results */
  delay(500);
}