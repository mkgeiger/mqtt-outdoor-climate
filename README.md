# MQTT Outdoor Climate
## Overview
In this project you will see how to build a battery powered MQTT client on base of an ESP8266 to measure high accurate weather data. The features for my needs are:
* measure typical (outdoor) weather data like temperature, humidity and pressure with high accuracy.
* power saving design because of battery powered. Battery life should last at least a half year, better more than one year.

## Hardware design
### Schematic
![Schematic](/hardware/Schematic.png)

### ESP8266 controller board
In fact you could choose and `ESP-01` board. But due to some modifications/reworks to be done to the board I preferred to take the slightly bigger `Wemos D1 mini` board, with which the reworks are a lot easier to be handled.
![WemosD1mini](/hardware/WemosD1mini.png)

### Step-Up converter
My descision was to take a 3.3V step-up converter with a low quiescent current (40μA measured). The regulator NCP1402 generates 3.3V from voltages as low as 1.5V (measured) and delivers up to 200mA, which is enough to supply this circuit. This is perfectly when using 2 AAA 1.5V battery cells. The cells will be used with a high efficiency grade until their death.
![Pololu](/hardware/Pololu.png)

### Sensor
My descision was to take the Bosch BME280 all-in-one sensor, which incorporates a temperature, humidity and pressure sensor in one housing. The sensor is connected via I²C interface. The Adafruit BME280 + Sensor libraries are used to talk with the BME280.
![BME280](/hardware/BME280.png)

## Power saving hardware measures
1. Enabling deep-sleep mode by connecting RST-pin to D0 (GPIO16). In deep-sleep mode the ESP8266 will not consume more then 40μA (measured).
2. Disabling the USB to serial converter chip CH340C by cutting the pins 4 and 16 (see screenshot). This chip consumes permanently several 10mA.
3. Also removing the LDO ME6211 (see screenshot) is required, which has a relative high quiescent current.
4. Disable permanent powering of the sensor module BME280. The sensor module will only be powered for a short time during measurements by the ESP8266 (pin D6).
5. Using a step-up converter (e.g. NCP1402) with a relative low quiescent current (40μA measured), which allows also to use 2 single 1,5V AAA cells.

## Power saving software measures
1. Going to deep-sleep mode after one measurement for about 10 minutes. In deep-sleep mode the complete circuit consumes about 80μA (measured).
2. Keeping the startup sequence and measurements short. This is managed by taking allways at startup the last stored/used Wifi settings (if not changed). All in all connecting to Wifi, connecting to the MQTT broker and publishing one measurement dataset takes about 600ms (measured). During these 600ms the circuit draws round about 100mA (measured).

## SW-update
Due to the fact that the USB to serial converter chip CH340C has been disabled, the only update over an external USB to serial UART interface (e.g. FT232R) is possible. This interface shall be connected to the RX and TX pins of the ESP8266. Be sure the interface can be switched to 3.3V voltage levels, otherwise you could damage your ESP8266. During SW-update the flash button needs to be pressed, which enables the flash mode of the ESP8266.

![FT232R](/hardware/FT232R.png)

## SW installation and SW build
Following steps need to be done first:
1. install Arduino IDE 1.8.1x
2. download and install the ESP8266 board supporting libraries with this URL: http://arduino.esp8266.com/stable/package_esp8266com_index.json
3. select the `Lolin(Wemos) D1 R2 & mini` board
4. install the `PubSubClient` library, `Adafruit BME280` library and the `Adafruit Unified Sensor` library
5. config (see next chapter), compile and flash

## SW configuration
I implemented this software without a nice Webserver interface over which you could modify the settings. You need to adapt following settings in the code and recompile:
* #define WIFI_SSID            "YOUR_WIFI_SSID"       // your Wifi password
* #define WIFI_PASSWORD        "YOUR_WIFI_PASSWORD"   // your Wifi SSID
* #define ALTITUDE             431.0F                 // this is your altitude above sea level in meters
* #define TEMP_COMPENSATION    0.2F                   // your individual temperature compensation, you can calibrate the temperature with the value
* #define TIME_TO_SLEEP        600                                          // your sleep time in seconds
* IPAddress mqtt_server(192, 168, 1, 10);                                   // your broker IP address
* mqttClient.connect("MQTTOutsideClimate","YOUR_USERNAME","YOUR_PASSWORD")  // your broker MQTT user and password

## Correction of pressure value
As the sensor is calibrated at sea level (0m) it will show a very low pressure value at your location, which is way below the real pressure at your location and which would falsely indicate something like a hurricane. This value needs to be corrected with the international barometric formula (see https://en.wikipedia.org/wiki/Pressure_altitude). This is done in function `seaLevelForAltitude()` which has the parameters for actual outdoor temperature, the athmospheric pressure (the raw sensor pressure value) and the altitude at your location.
