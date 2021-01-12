# MQTT Outdoor Climate
## Overview

In this project you will see how to build a battery powered MQTT client on base of an ESP8266 to measure high accurate weather data. The features for my needs are:
* measure typical (outdoor) weather data like temperature, humidity and pressure with high accuracy.
* power saving design because of battery powered. Battery life should last at least 1/2 year, better one year.

## Hardware design
### Schematic
![Schematic](/hardware/Schematic.png)

### ESP8266 controller board
In fact you could choose and `ESP-01` board. But due to some modifications/reworks to be done on the board I preferred to take the slightly bigger `Wemos D1 mini` board, with which the reworks are easier to be managed.
![WemosD1mini](/hardware/WemosD1mini.png)

### Step-Up converter
My descision was to take a 3.3V step-up converter with a low quiescent current (40uA). The regulator NCP1402 generates 3.3 V from voltages as low as 0.8V and delivers up to 200mA, which is enough to supply this schematic. This is perfectly when using 2 AAA 1.5V battery cells. The cells will be used with a high efficiency grade until their death.
![Pololu](/hardware/Pololu.png)

### Sensor
My descision was to take the Bosch BME280 all-in-one sensor, which incorporates a temperature, humidity and pressure sensor in one housing. The sensor is connected via I²C interface. The Adafruit BME280 library is used to talk with the BME280.
![BME280](/hardware/BME280.png)

## Power saving hardware measures
1. Enabling deep-sleep mode by connecting RST-pin to D0 (GPIO16). In deep-sleep mode the ESP8266 will not consume more then 40uA.
2. Disabling the USB to serial converter chip CH340C by cutting the pins 4 and 16. This chip consumes permanently several milliAmps.
3. Also removing the LDO ME6211 (see screenshot) is required, which has a relative high quiesent current.
4. Disable permanent powering of the sensor module (BME280). The sensor module will only be powered for a short time by the ESP8266 (pin D6).
5. Using a step-up converter (NCP1402) with a relative low quiesent current (40uA), which allows also to use 2 single 1,5V AAA cells.

## Power saving software measures
1. Going to deep-sleep mode after one measurement for about 10 minutes. In deep-sleep mode the total schematic consumes about 80uA.
2. Keeping the booting and measurement short. This is managed by taking allways at startup the last stored/used Wifi setting (if not changed). All in all connecting to Wifi, connecting to the Wifi broker and publishing one measurement dataset takes  about 600 ms. During these 600ms to schematic draws round about 100mA.

## SW-update
Due to the fact that the USB to serial converter chip CH340C has been disabled, the only update over an external USB to serial UART interface (e.g. FT232R) is possible. This interface shall be connected to the RX and TX pins of the ESP8266. Be sure the interface can be switched to 3.3V voltage levels. During SW-update the flash button needs to be pressed, which enables the flash mode of the ESP8266.

![FT232R](/hardware/FT232R.png)
