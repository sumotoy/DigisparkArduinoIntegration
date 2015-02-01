TinyPpmReader library
=====================

**TinyPpmReader** is an interrupt-driven **RC PPM reader** library relying on **TinyPinChange** library: this means the PPM frame input pin shall support pin change interrupt.

This PPM reader can extract up to 9 RC channels and supports positive and negative PPM modulation.

Some examples of use cases:
-------------------------
* **Standalone RC PPM reader**
* **Channel substitution in an existing PPM frame** (in conjunction with  **TinyPpmGen** library)
* **Digital data transmission over PPM** (in conjunction with  **TinyPpmGen** library)

Supported Arduinos:
------------------
* **ATtiny167 (Standalone or Digispark pro)**
* **ATtiny85 (Standalone or Digispark)**
* **ATmega368P (UNO)**

Tip and Tricks:
--------------
Develop your project on an arduino UNO, and then shrink it by loading the sketch in an ATtiny or Digispark (pro).

API/methods:
-----------
* **TinyPpmReader_Init(uint8_t _PpmInputPin_)**
With:
	* **_PpmInputPin_**: The PPM input pin. The modulation can be _Positive_ or _Negative_: it doesn't matter, since sampling on rising edges or on falling edges is equivalent. 

* **TinyPpmReader_DetectedChannelNb()**: returns the number of detected RC channels in the PPM frame.
* **uint16_t TinyPpmReader_Width_us(uint8_t _ChIdx_)**:
With:
	* **_ChIdx_**: The Channel index (from 1 to Detected Channel Number).
	* Returns the requested channel pulse width in µs

* **TinyPpmReader_IsSynchro()**:
	* PPM Synchronization indicator: indicates that the largest pulse value (Synchro) has just been received. This is a "clear on read" fonction (no need to clear explicitely the indicator).

* **TinyPpmReader_Suspend()**: supends the PPM acquisition. This can be useful whilst displaying results through a software serial port which disables interrupts during character transmission.

* **TinyPpmReader_Resume()**: resumes the PPM acquisition.


Design considerations:
---------------------
As this library relies on **TinyPinChange** library, see **TinyPinChange** library to check for the supported input pins depending of the target: for example, not all the arduino MEGA pins are support pin change interrupt.


Contact
-------

If you have some ideas of enhancement, please contact me by clicking on: [RC Navy](http://p.loussouarn.free.fr/contact.html).

