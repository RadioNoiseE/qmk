## Keyboard firmware for Atmel AVR based BSSK R1

* Keyboard Maintainer: [Jing Huang](https://github.com/RadioNoiseE)
* Hardware Supported: BSSK R1 Keyboard
* Hardware Availability: Model F Labs LLC

This is an attempt to port the wcass firmware to the latest QMK.

Note that magic boot is currently problematic since it functions in
the early stages and our own matrix scan override hasn't been
activated yet.

Also the raw hid isn't working and there is no plan at the time to fix
it.
