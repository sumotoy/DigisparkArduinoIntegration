TinySoftPwm library
===================

**TinySoftPwm** is a library designed to generate PWM signals by software.

Some examples of use cases:
-------------------------
* **RGB strip LED Controller**
* **DC Motor controller**
* **Digital to Analog converter**

Supported Arduinos:
------------------
* **ATtiny85 (Standalone or Digispark)** (up to 6 software PWM supported)
* **ATtiny167 (Digispark pro)** (up to 13 software PWM supported)

API/methods:
-----------
* TinySoftPwm_begin()
* TinySoftPwm_analogWrite()
* TinySoftPwm_process()


Design considerations:
---------------------
In order to reduce **program** and **RAM** memories, PWM pins shall be declared in the **TinySoftPwm.h** file. All the required amount of **program** and **RAM** memories are allocated at compilation time.

The **TinySoftPwm_process()** method shall be called periodically:

* using micros() in the loop(): in this case, asynchronous programmation shall be used: no call to blocking functions such as delay() is permtitted.
* or better using periodic interruption.


Contact
-------

If you have some ideas of enhancement, please contact me by clicking on: [RC Navy](http://p.loussouarn.free.fr/contact.html).

