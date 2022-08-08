#define OLEDDISPLAY
//#define RTCDEBUG
//#define MQTTDEBUG

//#define DS18EBUG
//#define WINDDIRDEBUG
//#define WINDSPDDEBUG
//#define LUXDEBUG
//#define UVDEBUG 

//#define LOOPDEBUG

#include "WiFi.h"
#include <WiFiClient.h>
#include <SPIFFS.h>
#include "ESPAsyncWebServer.h"
#include <ESPmDNS.h>
#include <Update.h>
#include <time.h>
#include <Wire.h>
#include <PubSubClient.h>
#include "ESP32Ping.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <RTClib.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#ifdef OLEDDISPLAY
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#endif

// Define and set up GPIO pins:
const byte pinPower = 13;                         // Detect power source (Digital)
const byte pinNetworkSelect = 17;                 // Select WiFi mode
const byte pinOneWire = 23;                       // One wire bus pin (for D18B20 temperature sensor)
const byte pinLux = 32;                           // Photoresistor (Analog)
const byte pinVane = 33;                          // Wind Vane (Analog)
const byte pinAnemo = 34;                         // Anemometer (Digital)
const byte pinBucket = 35;                        // Bucket (Digital)
const byte pinUV = 36;                            // UV Sensor (Analog)

// Define I2C addresses:
const byte addrBME = 0x77;                        // BME280 I2C address
const byte addrRTC = 0xD0;                        // DS1307 RTC address
#ifdef OLEDDISPLAY
const int DisplayAddr = 0x3C;                     // OLED I2C address
#endif

//Set up constants
const String hostname = "WxStation";              // Wifi hostname
const char* WifiSSID = "IT Hz When IP";           // STA mode Wifi SSID
const char* WifiPassword = "antibioticshelp";     // STA mode Wifi Password
const char* APSSID = "AccioNetwork";              // AP mode Wifi SSID
IPAddress local_IP(10, 0, 0, 151);                // Static IP address
IPAddress gateway(10, 0, 0, 1);                   // Gateway IP address
IPAddress subnet(255, 255, 0, 0);                 // Network mask
IPAddress DNS1 = (10, 0, 0, 2);                   // Network DNS server
WiFiClient WxWiFiClient;                          // WiFi client object
AsyncWebServer server(80);                        // Web Server port

const char* mqttBroker = "10.0.0.2";              // MQTT broker address
const int mqttPort = 1883;                        // MQTT broker port
const char* mqttCtrlTopic = "WxStn/Cmd";          // MQTT control topic
const char* mqttDataTopic = "WxStn/Tele";         // MQTT data topic
PubSubClient MQTTclient(WxWiFiClient);            // MQTT client object

const char* ntpServer1 = "10.0.0.2";              // NTP server address
const char* ntpServer2 = "ca.pool.ntp.org";       // Backup NTP server address
const long gmtOffset_sec = -7 * 3600;             // NTP timezone offset in seconds
const int daylightOffset_sec = 3600;              // NTP daylight savings offset

#ifdef OLEDDISPLAY
const int SCREEN_WIDTH = 128;               // OLED display width, in pixels
const int SCREEN_HEIGHT = 64;               // OLED display height, in pixels
const int OLED_RESET = -1;                  // OLED reset pin (-1 because it doesn't exist on this board)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#endif

Adafruit_BME280 bme;                              // BME object via I2C
RTC_DS1307 rtc;                                   // RTC object via I2C
Ds1307SqwPinMode mode = DS1307_OFF;               // DS1307 square wave pin mode  (Options: DS1307_OFF, DS1307_ON, DS1307_SquareWave1HZ)
DateTime now;                                     // RTC DateTime object
const float ds18CalOffset = -4;                   // DS18B20 temperature calibration
OneWire oneWire(pinOneWire);                      // 1-wire bus object
DallasTemperature sensors(&oneWire);              // Dallas sensors object
DeviceAddress ds18Addr;                           // DS18B20 1-wire address
const int luxThreshold = 2048;                    // Photoresistor day/night threshold
const int luxHysteresis = 1024;                   // Photoresistor threshold hysteresis
const byte reedDebounceTime = 166;                // Reed switch debouncing time in ms

// Set up data variables
String header;                                    // Variable to store the HTTP request

char stampTime[30] = "Startup";                   // Report count number(in place of a timestamp?)
float windGust = 0;                               // Gust speed (3 second reading)
int windCount[200];                               // 3 second reading array for averaging
float windSpeed[] = {0, 0, 0, 0, 0, 0};           // Windspeed (km/h) (averaged over rolling 10 minutes)
char windDir[4];                                  // Wind Direction (combined quadrants: 16 possibilities)
float rainHr[24];                                 // Rainfall (array for hourly - last 24)
float rainDaily = 0;                              // Rainfall (rolling 24-hour total)
float temperature = -40.0;                        // Temperature (deg C)
float pressureRaw = 75.0;                         // Barometric Pressure (kPa)
const int altitude = 670;                         // Sensor altitude for sea level pressure calculation
float pressureSeaLvl = 80;                        // Barometric pressure at sea level
float humidity = 50.0;                            // Humidity (%)
float dewpoint = 0.0;                             // Calculated Dewpoint
byte uvIndex = 1;                                 // UV Index
int rawLux;                                       // Raw analog intensity reading
char lightLvl[7];                                 // Light intensity (to detect day or night)

// Function globals
byte flags = 128;
int anemoCount = 0;
int timeHr = 0;
int timeMin = 0;
int timeSec = 0;
struct tm timeinfo;

void initWiFi() {                                 // Set up Wifi Connection
  pinMode(pinNetworkSelect, INPUT_PULLUP);
  if (digitalRead(pinNetworkSelect) == 1)  {
    WiFi.mode(WIFI_STA);
    WiFi.config(local_IP, gateway, subnet, DNS1);
    WiFi.setHostname(hostname.c_str()); //define hostname
    WiFi.begin(WifiSSID, WifiPassword);
    Serial.print("Connecting to WiFi...");
    for (int count = 1; count <= 10; count ++)  {
      if (WiFi.status() != WL_CONNECTED)  {
      delay(1000);
      Serial.print(".");
      }
      else {
        Serial.println(" Connected in station mode.");
        Serial.print("IP Address is "); Serial.println(WiFi.localIP());
        break;
      }
    }
    if (WiFi.status() != WL_CONNECTED)  {Serial.println("Failed.");}
  }
  else  {;
    WiFi.mode(WIFI_AP);
    WiFi.softAP(APSSID);Serial.print("Initializing Access Point...");
    delay(1000);
    Serial.println(" Active.");
    Serial.print("IP Address is "); Serial.println(WiFi.softAPIP());
  }
}

void initMQTT() {                               // Set up MQTT
  MQTTclient.setServer(mqttBroker, mqttPort);
  MQTTclient.setKeepAlive(90);                      // Keep connection alive for 90 seconds
  Serial.print("Connecting to MQTT broker...");
  while (!MQTTclient.connected()) {
    Serial.print("...");
    if (MQTTclient.connect("WxStnClient")) {
      Serial.println("Connected");
    }
    else {
      Serial.print("Failed - Error state "); Serial.println(MQTTclient.state());
      delay(1000);
    }
  }
}

void adjustRTC(const char* NTPaddress)  {
  char timebuf[5];

  Serial.print("...");
  getLocalTime(&timeinfo);
  strftime(stampTime, 30, "%Y/%m/%d; %H:%M:%S", &timeinfo);
  #ifdef RTCDEBUG 
  Serial.print("Initial Time: "); Serial.println(stampTime);
  #endif
  configTime(gmtOffset_sec, daylightOffset_sec, NTPaddress);     // Set up time
  getLocalTime(&timeinfo);
  strftime(stampTime, 30, "%Y/%m/%d; %H:%M:%S", &timeinfo);
  #ifdef RTCDEBUG 
  Serial.print("Server Time: "); Serial.println(stampTime);
  #endif
  strftime(timebuf, 5, "%Y", &timeinfo);
  int y = atoi(timebuf);
  strftime(timebuf, 3, "%m", &timeinfo);
  int mo = atoi(timebuf);
  strftime(timebuf, 3, "%d", &timeinfo);
  int d = atoi(timebuf);
  strftime(timebuf, 3, "%H", &timeinfo);
  int h = atoi(timebuf);
  strftime(timebuf, 3, "%M", &timeinfo);
  int mi = atoi(timebuf);
  strftime(timebuf, 3, "%S", &timeinfo);
  int s = atoi(timebuf);
  rtc.adjust(DateTime(y, mo, d, h, mi, s));

  DateTime now = rtc.now();
  timestamp();
  Serial.print("New Time: "); Serial.print(timeHr); Serial.print(":"); if (timeMin < 10) {Serial.print("0");} Serial.println(timeMin);
}

void timestamp() {
  char strBuf[3];
  int timeYr;
  int timeMo;
  int timeDay;

  DateTime now = rtc.now();
  timeYr = now.year();
  timeMo = now.month();
  timeDay = now.day();
  timeHr = now.hour();
  timeMin = now.minute();
  timeSec = now.second();

  itoa(timeYr, strBuf, 10);
  strcpy(stampTime, strBuf);
  strcat(stampTime, "/");
  
  if (timeMo < 10)  {strcat(stampTime, "0");}
  itoa(timeMo, strBuf, 10);
  strcat(stampTime, strBuf);
  strcat(stampTime, "/");
  
  if (timeDay < 10)  {strcat(stampTime, "0");}
  itoa(timeDay, strBuf, 10);
  strcat(stampTime, strBuf);
  strcat(stampTime, ", ");
  
  if (timeHr < 10)  {strcat(stampTime, "0");}
  itoa(timeHr, strBuf, 10);
  strcat(stampTime, strBuf);
  strcat(stampTime, ":");
  
  if (timeMin < 10)  {strcat(stampTime, "0");}
  itoa(timeMin, strBuf, 10);
  strcat(stampTime, strBuf);
  strcat(stampTime, ":");
  
  if (timeSec < 10)  {strcat(stampTime, "0");}
  itoa(timeSec, strBuf, 10);
  strcat(stampTime, strBuf);
  #ifdef RTCDEBUG 
  Serial.print("Timestamp: "); Serial.println(stampTime);
  delay(200);
  #endif
}

void initSens() {
 Serial.println("Initializing Sensors");

  // Set up DS18B20 sensor
  sensors.begin();
  
  #ifdef DS18EBUG
  Serial.print("DS18B20 sensor found at address ");
  sensors.getAddress(ds18Addr, 0);
  for (uint8_t i = 0; i < 8; i++)
  {
    if (ds18Addr[i] < 16);
    Serial.print(ds18Addr[i], HEX);
  }
  Serial.println("h");
  #endif
  sensors.setResolution(ds18Addr, 11);
  readDS18B20();

  // Set up BME280
  bool status = bme.begin(addrBME);
  byte count = 1;
  while (!status) {
    if (count <= 5) {
      Serial.println(F("Could not find a valid BME280 sensor, Trying again in 5 seconds."));
      count++;
      delay(5000);
    }
    else {
      Serial.println("BME/BMP Sensor not found. Continuing with faked readings.");
      break;
    }
  }
  /* Default settings from datasheet. */
  /*bme.setSampling(sensor_sampling::MODE_FORCED,                  // Operating Mode
                    sensor_sampling::SAMPLING_X1,      // Temp. oversampling
                    sensor_sampling::SAMPLING_X1,     // Pressure oversampling
                    sensor_sampling::SAMPLING_X1,       // Humidity oversampling
                    sensor_filter::FILTER_OFF,               // Filtering
                    standby_duration::STANDBY_MS_1000);    // Standby time*/
  readBME();
  if ((isnan(temperature)) || (isnan(humidity)) || (isnan(pressureRaw))) {
    Serial.println("Invalid BME read, using faked values");
  }

  // Set up wind vane sensor
  pinMode(pinVane, INPUT);
  adcAttachPin(pinVane);
  analogSetAttenuation(ADC_11db);

  measWindDir();
  #ifdef WINDDIRDEBUG
  Serial.print("Initial wind direction reading: "); Serial.println(windDir);
  #endif

  // Set up anemometer
  pinMode (pinAnemo, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinAnemo), AnemoInterrupt, FALLING);
  for (count = 0; count < 199; count++)  {
    #ifdef WINDSPDDEBUG
    Serial.print("windCount["); Serial.print(count); Serial.print("]: "); Serial.print(count); Serial.print(" - "); Serial.print(windCount[count]); Serial.println("/0");
    #endif
    windCount[count] = 0;
  }
  windSpeed[0]=0;
  #ifdef WINDSPDDEBUG 
  Serial.print("Initial windspeed reading: "); Serial.print(windSpeed[0]); Serial.println("km/h");
  #endif

  //Set up rain bucket gauge
  pinMode (pinBucket, INPUT_PULLUP);
  count = 0;
  while (count <= 23) {
    rainHr[count] = 0;
    count++;
  }
  attachInterrupt(digitalPinToInterrupt(pinBucket), RainInterrupt, FALLING);

  //Set up ML8511
  pinMode (pinUV, INPUT);
  measUV();

  // Set up photoresistor
  pinMode (pinLux, INPUT);
  measLux();
}

void setup() {    // put your setup code here, to run once:
  // Set up interfaces
  Serial.begin(115200);                                         // Start serial output for monitor
  initWiFi();                                              // Start WiFi connection in Station mode
  
  #ifdef OLEDDISPLAY
  // Initialize OLED display
  OLEDInit();
  #endif

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS!");
    return;
  }
  else  {
    Serial.println("SPIFFS file system mounted.");
  }

  // Initialize Real-time Clock
  rtc.begin();
  Serial.println("Attempting to reach NTP server(s).");
  if (Ping.ping(ntpServer1))  {                                 // Attempt to correct RTC time dift if any
    Serial.print("Updating RTC from local NTP server");
    adjustRTC(ntpServer1);
  }
  else if (Ping.ping(ntpServer2))  {
    Serial.print("Unable to reach local NTP server, updating RTC from remote server");
    adjustRTC(ntpServer2);
  }
  else  {
    Serial.println("Unable to reach local or remote NTP servers, using stored time.");
  }

  // Initialize mDNS
  if (MDNS.begin("hostname")) {
    Serial.println("MDNS responder started");
    }

  webserverInit();

  server.begin();
  Serial.println("HTTP server started");

  // Initialize MQTT communication
  if (digitalRead(pinNetworkSelect) == 1)  {initMQTT();}

  // Initialize sensors and make inital readings
  initSens();

  // Transmit initial readings via MQTT
  Serial.println("");
  pubTelemetry("Time");
  pubTelemetry("Wind");
  pubTelemetry("Rain");
  pubTelemetry("Temp");
  pubTelemetry("Press");
  pubTelemetry("Humid");
  pubTelemetry("DewPt");
  pubTelemetry("Light");
  Serial.println("");

  Serial.println("Setup complete. Starting gathering loop");
  Serial.println("");
}

void loop() {
  // put your main code here, to run repeatedly:
  static unsigned long gustTime = 0; // 3sec duration
  static unsigned long windTime = 0; // 10 min avg
  static unsigned long oldTimer = 0;
  static float oldRainMin = 0;
  static float oldRainHr = 0;

  timestamp();

  if (millis() >= (gustTime + 3000)) {                  //3 seconds
    gustTime = millis();
    measWindSpd(3);
    #ifdef OLEDDISPLAY
    UpdateOLED();
    #endif
  }

  if (millis() >= (windTime + 600000)) {                // 10 minutes
    windTime = millis();
    measWindSpd(10);
  }

  if (timeSec >= 1) {
    bitClear(flags, 7);
  }
  if ((timeSec == 0) && (!bitRead(flags, 7))) {         // Every minute
    bitSet(flags, 7);
    #ifdef LOOPDEBUG
    Serial.println("Minute change detected");
    #endif
    if ((isnan(temperature)) || (isnan(humidity))) {
      readBME();
    }
    measWindDir();
    measLux();
    
    if (timeMin == 0) {                                 // Every Hour
      #ifdef LOOPDEBUG 
      Serial.println("Hour change detected");
      #endif
      readDS18B20();
      readBME();
      measUV();
      for (int count = 23; count > 0; count--) {
        rainHr[count - 1] = rainHr[count];
      }
      int index = int(timeHr);
      rainHr[index] = 0;
      if ((rainHr[timeHr] <= oldRainHr - 0.5) || (oldRainHr + 0.5 <= rainHr[timeHr]))  {
        bitSet(flags, 1);  //Rain flag
      }
    }
  }

  if ((flags - 128) > 0) {
    #ifdef MQTTDEBUG 
    Serial.print("Starting flag Value: "); Serial.println(flags-128);
    #endif
    Serial.println("");
    pubTelemetry("Time");
    if (bitRead(flags, 0))  {
      pubTelemetry("Wind");
      bitClear(flags, 0);
    }
    if (bitRead(flags, 1))  {
      pubTelemetry("Rain");
      bitClear(flags, 1);
    }
    if (bitRead(flags, 2))  {
      pubTelemetry("Temp");
      bitClear(flags, 2);
    }
    if (bitRead(flags, 3))  {
      pubTelemetry("Press");
      bitClear(flags, 3);
    }
    if (bitRead(flags, 4))  {
      pubTelemetry("Humid");
      bitClear(flags, 4);
    }
    if (bitRead(flags, 5))  {
      pubTelemetry("DewPt");
      bitClear(flags, 5);
    }
    if (bitRead(flags, 6))  {
      pubTelemetry("Light");
      bitClear(flags, 6);
    }

    #ifdef MQTTDEBUG 
    Serial.print("Ending flag Value: ");Serial.println(flags-128);
    #endif
    Serial.println(" ");
  }
}
