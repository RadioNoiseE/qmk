# Keyboard firmware for Atmel AVR based BSSK R1

* Keyboard Maintainer: [Jing Huang](https://github.com/RadioNoiseE)
* Hardware Supported: BSSK R1 Keyboard
* Hardware Availability: Model F Labs LLC

This is an attempt to port the wcass firmware to the latest QMK.

Make example for this keyboard (after setting up your build environment):

    make beamspring:default

Flashing example for this keyboard:

    make beamspring:default:flash

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information.

## Bootloader

Enter the bootloader in 2 ways:

* **Bootmagic reset**: Hold down the key at (0,0) in the matrix (usually the top left key or Escape) and plug in the keyboard
* **Keycode in layout**: Press the key mapped to `QK_BOOT` by <kbd>Fn + B</kbd>
