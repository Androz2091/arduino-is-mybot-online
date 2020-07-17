# Arduino, is my bot online?

It opens a websocket connection with Discord to receive `PRESENCE_UPDATE` events. Once the target become offline, it makes a red led blinks, and once it comes back online, it turns on the green led.

## How to install

* Set up your project using the schematic below.
* Download the code.
* Rename the `secrets_sample.h` file to `secrets.h` and fill it.
* Upload it to your Arduino.
* You're done!

Once the watched bot will be offline, the red led will blink.

## Used components

* Arduino esp8266 mod (node mcu 1.0)
* Breadboard
* x2 color leds (one green, one red)
* x2 resistors (220 ohm)
* x3 wires

## Schematic

![schematic](./schematic.png)
