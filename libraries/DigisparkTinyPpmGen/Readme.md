TinyPpmGen library
==================

**TinyPpmGen** is an interrupt-driven RC PPM generator library using Timer0 Output Compare Interrupt. As this library uses hardware resources, the timing is very accurate.

Some examples of use cases:
-------------------------
* **Standalone RC PPM generator**
* **Channel substitution in an existing PPM frame**

Supported Arduinos:
------------------
* **ATtiny167 (Standalone or Digispark pro)**
* **ATtiny85 (Standalone or Digispark)**
* **ATmega368 (UNO)**

Tip and Tricks:
--------------
Develop your project on an arduino UNO, and then shrink it by loading the sketch in an ATtiny or Digispark (pro).

API/methods:
-----------
* **TinyPpmGen_Init(uint8_t _PpmModu_, uint8_t _ChNb_)**
With:
	* **_PpmModu_**: **TINY_PPM_GEN_POS_MOD** or **TINY_PPM_GEN_NEG_MOD** for respectiveley positive and negative PPM modulation
	* **_ChNb_**: The number of RC channel to transport in the PPM frame (1 to 8)
* **TinyPpmGen_SetChWidth_us(uint8_t _ChIdx_, uint16_t _Width_us_)**
With:
	* **_ChIdx_**: the RC channel (1 to _ChNb_)
	* **_Width_us_**: the pulse width in µs

Design considerations:
---------------------
As this library relies on Timer0 Output Compare Interrupt, the PPM output pin is imposed by hardware depending of the target:

* **ATtiny167**  (Digispark pro: PPM output -> PA2 -> arduino pin#8)
* **ATtiny85**   (Digispark:     PPM output -> PB0 -> arduino pin#0)
* **ATmega328P** (Arduino UNO:   PPM output -> PD6 -> arduino pin#6)

Contact
-------

If you have some ideas of enhancement, please contact me by clicking on: [RC Navy](http://p.loussouarn.free.fr/contact.html).

