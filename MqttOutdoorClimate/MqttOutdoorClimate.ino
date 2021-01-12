#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <Ticker.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <PubSubClient.h>
#include "i2cbase.h"
 
#define WIFI_SSID            "YOUR_WIFI_SSID"
#define WIFI_PASSWORD        "YOUR_WIFI_PASSWORD"
#define ALTITUDE             431.0F        // your meters above sea level
#define TEMP_COMPENSATION    0.2F          // your individual compensation
#define MAX_LOOP_TIME_MS     5000UL        // max. 5 seconds
#define WATCHDOG_TIMEOUT_MS  6000UL        // 6 seconds
#define TIME_TO_SLEEP        600           // wakeup every 10min

Ticker sleepTicker;
Adafruit_BME280 bme;
IPAddress mqtt_server(192, 168, 1, 10);    // your boker IP address
WiFiClient espClient;
PubSubClient mqttClient(espClient);

static char temperature_str[10];
static char humidity_str[10];
static char pressure_str[10];

static uint32_t startTime;

bool isValidHumidity(float humidity)
{
  return ((!isnan(humidity)) && (humidity >= 0.0) && (humidity <= 100.0));
}

bool isValidTemperature(float temperature)
{
  return ((!isnan(temperature)) && (temperature >= -100.0) && (temperature <= 212.0));
}

bool isValidPressure(float pressure)
{
  return ((!isnan(pressure)) && (pressure >= 300.0) && (pressure <= 1100.0));
}

float seaLevelForAltitude(float altitude, float atmospheric, float temperature)
{
  return (atmospheric / pow(1.0F - ((0.0065F * altitude) / (273.15F + temperature)), 5.255F));
}

void sleepyTime(void)
{
  uint32_t elapsed = millis() - startTime;
  Serial.printf("Going to sleep. Cycle took %d ms\n", elapsed);
  // if this sleep happened because of timeout, clear the Wifi state.
  if (elapsed >= MAX_LOOP_TIME_MS)
  {
    WiFi.disconnect();
  }
  ESP.deepSleep(TIME_TO_SLEEP * 1e6, WAKE_RF_DEFAULT);
}

void mqttConnect(void)
{  
  // Loop until we're reconnected  
  while (!mqttClient.connected())
  {  
    Serial.print("Attempting MQTT connection...");  
    // Attempt to connect  
    if (mqttClient.connect("MQTTOutsideClimate","YOUR_USERNAME","YOUR_PASSWORD"))
    {  
      Serial.println("connected");  
    }
    else
    {  
      Serial.print("failed, rc=");  
      Serial.print(mqttClient.state());  
      Serial.println(" try again in 1 second");  
      // Wait 1 second before retrying  
      delay(1000);  
    }  
  }  
}  
 
void setup(void)
{
  // watchdog for shutdown if WIFI or MQTT server is down
  startTime = millis();
  sleepTicker.once_ms(WATCHDOG_TIMEOUT_MS, &sleepyTime);

  // power on bme280
  pinMode(D6, OUTPUT);
  digitalWrite(D6, HIGH);

  // UART init
  Serial.begin(115200);
  Serial.println();

  // BME280 init
  bme.begin(0x76);
  bme.setTemperatureCompensation(TEMP_COMPENSATION);

  // Use the i2cbase library to directly modify some of the
  // BME280 configuration registers.
  i2cBase bconfig(0x76);
  // 1x sampling for humidity.
  bconfig.write8(0xF2, 0x01);
  // 1x sampling for temperature and pressure, sleep mode on.
  bconfig.write8(0xF4, 0x24);
  // With sleep mode on, change to 1s sampling.
  bconfig.write8(0xF5, 0xA0);
  // 8x sampling for temperature and pressure, normal mode.
  bconfig.write8(0xF4, 0x27);

  // Wifi
  if (WiFi.SSID() != WIFI_SSID)
  {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    WiFi.persistent(true);
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
  } 
  Serial.print("Connecting to WiFi.");
  while (WiFi.status() != WL_CONNECTED)
  {  
    Serial.print(".");
    delay(100);  
  }  
  Serial.println(" Done");
  Serial.printf("SSID: %s\n", WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // MQTT
  mqttClient.setServer(mqtt_server, 1883);
  delay(100);
  mqttConnect();   
  mqttClient.loop();

  // measurements
  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0F;
  while (!((isValidTemperature(temperature)) && (isValidHumidity(humidity)) && (isValidPressure(pressure)))) 
  {
    delay(100);
    temperature = bme.readTemperature();
    humidity = bme.readHumidity();
    pressure = bme.readPressure() / 100.0F;
  }
      
  pressure = seaLevelForAltitude(ALTITUDE, pressure, temperature);
          
  snprintf(temperature_str, 10, "%.1f", temperature);
  snprintf(humidity_str, 10, "%.1f", humidity);
  snprintf(pressure_str, 10, "%.1f", pressure);
  Serial.printf("Temperature: %.1f°C Humidity: %.1f%% Pressure: %.1fhPa\n", temperature, humidity, pressure);
          
  mqttClient.publish("/outdoor/temperature", temperature_str, true);
  mqttClient.publish("/outdoor/humidity", humidity_str, true);
  mqttClient.publish("/outdoor/pressure", pressure_str, true);

  delay(200); 
  
  // going to sleep
  sleepyTime();
  delay(100);
}

void loop(void)
{    
  delay(100);
} 
