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
#include <RcRxPop.h>

#define PPM_INPUT_PIN  2

TinyPpmReader MyPpmReader; /* Create a TinyPpmReader object */

void setup()
{
  Serial.begin(115200);
  MyPpmReader.attach(PPM_INPUT_PIN); /* Attach MyPpmReader to PPM_INPUT_PIN pin */
}

void loop()
{
  MyPpmReader.suspend(); /* Not needed if an hardware serial is used to display results */
  Serial.print(F("* Period="));Serial.print((int)MyPpmReader.ppmPeriod_us());Serial.println(F(" us *"));
  Serial.print(F("ChNb="));Serial.println((int)MyPpmReader.detectedChannelNb());
  for(uint8_t Idx = 1; Idx <= MyPpmReader.detectedChannelNb(); Idx++) /* From Channel 1 to Max detected */
  {
    Serial.print(F("Ch"));Serial.print(Idx);Serial.print(F("="));Serial.print(MyPpmReader.width_us(Idx));Serial.println(F(" us"));
  }
  MyPpmReader.resume(); /* Not needed if an hardware serial is used to display results */
  delay(500);
}
