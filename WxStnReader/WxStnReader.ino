#define OLEDDISPLAY
#define TFTDISPLAY

#define STAWIFI
#ifndef STAWIFI
#define APWIFI
#endif

//#define OLEDDEBUG
//#define TFTDEBUG
//#define MQTTDEBUG
//#define RTCDEBUG
//#define DS18DEBUG
//#define BME280DEBUG
//#define WINDDIRDEBUG
#define WINDSPDDEBUG
//#define LUXDEBUG
//#define UVDEBUG 

//#define SETUPDEBUG
//#define LOOPDEBUG

#include <Preferences.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <time.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <ESP32Ping.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <RTClib.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#ifdef OLEDDISPLAY
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#endif
#ifdef TFTDISPLAY
#include <Adafruit_ST7735.h>
//#include <analogWrite.h>
#endif

// Define and set up GPIO pins:
const byte pinVane = 12;                        // Wind Vane (Analog)      FAKED
const byte pinPower = 13;                       // Detect power source (Digital)
const byte pinOneWire = 16;                     // One wire bus pin (for D18B20 temperature sensor)
const byte pinDisplay = 26;                     // Display enable switch
const byte pinAnemo = 27;                       // Anemometer (Digital)
const byte pinBucket = 35;                      // Bucket (Digital)
const byte pinUV = 36;                          // UV Sensor (Analog)
const byte pinLux = 39;                         // Photoresistor (Analog)  FAKED

// Define I2C addresses:
const byte addrBME = 0x77;                      // BME280 I2C address
const byte addrRTC = 0xD0;                      // DS1307 RTC address

// OLED DISPLAY
//const int PinSDA = 21;                        //  Default I2C SDA (defined in library)
//const int PinSCL = 22;                        //  Default I2C SCL (defined in library)

// TFT DISPLAY
// Define SPI addresses
//const byte PinSCK = 18;                       // Connect to SPI clock (SCL on display) (defined in library)
//const byte PinMOSI = 23;                      // Connect to SPI MOSI (SDA on dsplay) (defined in library)
const byte pinRES = 19;                         // Connect to any digital out (Reset display) (RESET on display)
const byte pinDC = 4;                           // Connect to any digital out (Data/Command select) (D/C on display)
const byte pinCS = 17;                          // Connect to SPI CS (CS on display)
const byte pinBLK = 15;                         // Connect to any PWM (Backlight) (blk on display)
#ifdef TFTDISPLAY
const byte channelPWM = 0;                      // PWM channel for backlight
const int freqPWM = 5000;                       // PWM freqeuency for backlight
#endif

Preferences preferences;

typedef struct struct_settings  {               // Settings (bytes in preferences storage)
  uint8_t screen_bright = 255;                  // Screen brightness (1b)
} struct_settings;
struct_settings user_settings;

String hostname = "WxStation2";                 // WiFi hostname (11b)
char WifiSSID[33] = "IT Hz When IP";            // STA mode WiFi SSID (33b)
char WifiPassword[64] = "antibioticshelp";      // STA mode WiFi Password (64b)
char APSSID[33] = "AccioNetwork";               // AP mode Wifi SSID (33b)
char APPassword[64] = "";                       // AP mode WiFi password (64b)
IPAddress local_IP(10, 0, 0, 151);              // Static IP address (4b)
IPAddress gateway(10, 0, 0, 1);                 // Gateway IP address (4b)
IPAddress subnet(255, 255, 0, 0);               // Network mask (4b)
IPAddress DNS1 = (10, 0, 0, 2);                 // Local DNS server (4b)
IPAddress DNS2 = (8, 8, 8, 8);                  // Remote DNS server (4b)


WiFiClient WxWiFiClient;                        // WiFi client object
AsyncWebServer server(80);                      // Web Server port

IPAddress mqttBroker(10, 0, 0, 2);              // MQTT broker address
int mqttPort = 1883;                            // MQTT broker port (2b)
char mqttTopic[9] = "WxStn2";                   // MQTT base topic (9b)
char mqttCtrlTopic[33] = "/Cmd";                // MQTT control topic (33b)
char mqttDataTopic[33] = "/Tele";               // MQTT data topic (33b) = (307b total occupied for settings)
PubSubClient MQTTclient(WxWiFiClient);          // MQTT client object

typedef struct struct_time  {
  char* ntpServer1 = "10.0.0.2";                // NTP server address
  char* ntpServer2 = "ca.pool.ntp.org";         // Backup NTP server address
  long gmtOffset_sec = -7 * 3600;               // NTP timezone offset in seconds
  int daylightOffset_sec = 3600;                // NTP daylight savings offset
} struct_time;
struct_time NTP_RTC;

#ifdef OLEDDISPLAY
const int DisplayAddr = 0x3C;                   // OLED I2C address
const int SCREEN_WIDTH = 128;                   // OLED display width, in pixels
const int SCREEN_HEIGHT = 64;                   // OLED display height, in pixels
const int OLED_RESET = -1;                      // OLED reset pin (-1 because it doesn't exist on this board)
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#endif
#ifdef TFTDISPLAY
const int SCREEN_WIDTH_C = 128;                 // TFT display width, in pixels
const int SCREEN_HEIGHT_C = 160;                // TFT display height, in pixels
Adafruit_ST7735 tft = Adafruit_ST7735(pinCS, pinDC, pinRES);
const int CBLACK = 0x0000;                  
const int CBLUE = 0xF800;
const int CSKYBLUE = 0x867D;
const int CRED = 0xF800;
const int CGREEN = 0x07E0;
const int CDKGREEN = 0x2464;
const int CCYAN = 0x07FF;
const int CYELLOW = 0xFFE0;
const int CWHITE = 0xFFFF;
#endif

Adafruit_BME280 bme;                            // BME object via I2C
RTC_DS1307 rtc;                                 // RTC object via I2C
DateTime now;                                   // RTC DateTime object
Ds1307SqwPinMode mode = DS1307_OFF;             // DS1307 square wave pin mode  (Options: DS1307_OFF, DS1307_ON, DS1307_SquareWave1HZ)
OneWire oneWire(pinOneWire);                    // 1-wire bus object
DallasTemperature sensors(&oneWire);            // Dallas sensors object
DeviceAddress ds18Addr;                         // DS18B20 1-wire address

typedef struct struct_cal  {                    // Calibration settings
  float ds18CalOffset = 0;                      // DS18B20 temperature offset (1-point calibration)
  float ds18RawHi = 98.5;                       // DS18B20 calibration boiling water reading (2-point calibration)
  float ds18RawLo = 0.1;                        // DS18B20 temperature frozen water reading (2-point calibration)
  float ds18RefHi = 97.91;                      // DS18B20 temperature boiling water actual (2-point calibration)
  float ds18RefLo = 0.00;                       // DS18B20 temperature frozen water actual (2-point calibration)
  uint16_t luxThreshold = 2048;                 // Photoresistor day/night threshold
  uint16_t luxHysteresis = 1024;                // Photoresistor threshold hysteresis
} struct_cal;
struct_cal calibrations;
const uint8_t reedDebounceTime = 166;           // Reed switch debouncing time in ms

typedef struct struct_wx  {                     // Weather Data
  char rtcTime[30] = "Startup";                 // Timestamp (onboard/RTC time)
  char ntpTime[30] = "Startup";                 // Timestamp (NTP sourced time)
  char windDir[4];                              // Wind Direction (combined quadrants: 16 possibilities)
  float windGust = 0;                           // Gust speed (3 second reading)
  int interval = 0;                             // interval counter for windspeed
  uint16_t windCount[200];                      // 3 second reading array for averaging
  float windSpeed[6] = {0, 0, 0, 0, 0, 0};      // Windspeed (km/h) (averaged over rolling 10 minutes)
  float oldSpeed = 0;                           // Last speed for change detection
  float rainHr[24];                             // Rainfall (array for hourly - last 24)
  float rainDaily = 0;                          // Rainfall (rolling 24-hour total)
  float temperature = -40.0;                    // Temperature (deg C)
  float feelTemp = -40.0;                       // "Feels-like" temperature (deg C)
  float pressureRaw = 75.0;                     // Barometric Pressure (kPa)
  int8_t altitude = 670;                        // Sensor altitude for sea level pressure calculation
  float pressureSeaLvl = 80;                    // Barometric pressure at sea level
  float humidity = 50.0;                        // Humidity (%)
  float dewpoint = 0.0;                         // Calculated Dewpoint
  uint8_t uvIndex = 1;                          // UV Index
  uint16_t uvReading = 0;                       // Raw UV analog reading
  uint16_t rawLux = -1;                         // Raw analog intensity reading
  char lightLvl[7];                             // Light intensity (to detect day or night)
} struct_wx;
struct_wx weather;

// Function globals
unsigned int flags = 0x0000000111111111;         // (0:Wind, 1:Rain, 2:Temp, 3:Pressure, 4:Humidity, 5:Dewpoint, 6:Light, 7:External RTC, 8:OLED Display on, 9:TFT Display on)
int anemoCount = 0;
int timeHr = 0;
int timeMin = 0;
int timeSec = 0;
struct tm timeinfo;
float uvIndexV = 0;                             // Raw UV sensor voltage output

void prefSave()  {/*
  preferences.begin("prefsUser", false);
  uint8_t screen_bright = 255;                   // Screen brightness (1b)
  preferences.end();
  preferences.begin("prefsWiFi", false);
  char* WifiSSID = "IT Hz When IP";              // STA mode WiFi SSID (33b)
  char* WifiPassword = "antibioticshelp";        // STA mode WiFi Password (64b)
  char* APSSID = "AccioNetwork";                 // AP mode Wifi SSID (33b)
  char* APPassword = "";                         // AP mode WiFi password (64b)
  IPAddress WifiStaticIP = (10, 0, 0, 151);      // WiFi static IP Address to claim (4b)
  IPAddress WifiGateway = (10, 0, 0, 1);         // Wifi internet gateway (4b)
  IPAddress WifiSubnetMask = (255, 255, 0, 0);   // WiFi subnet mask (4b)
  IPAddress DNS1 = (10, 0, 0, 2);                // Local DNS server (4b)
  //IPAddress DNS2 = (8, 8, 8, 8);                 // Remote DNS server (4b)
  String hostname = "WxStation2";                // WiFi hostname (11b)
  preferences.end();
  preferences.begin("prefsMQTT", false);
  IPAddress MQTTBrokerAddress = (10, 0, 0, 2);   // MQTT broker address (4b)  
  int mqttPort = 1883;                           // MQTT broker port (2b)
  char mqttTopic[17] = "WxStn2";                 // MQTT base topic (17b)
  char mqttCtrlTopic[21] = "/Cmd";               // MQTT control topic (21b)
  char mqttDataTopic[22] = "/Tele";              // MQTT data topic (22b) = (292b total occupied for settings)
  preferences.end();*/
  Serial.println(" Saved.");
}

void prefLoad()  {
  byte charBuf;
  int intBuf;
  String strBuf;
  int IPBuf[4];

  preferences.begin("prefsUser", false);
  if (preferences.isKey("screen_bright")==true)  {
    Serial.print("Preferences found. Loading...");
    user_settings.screen_bright = preferences.getUChar("screen_bright");                   // Screen brightness (1b)
    preferences.end();
    preferences.begin("prefsWiFi", false);
    strBuf = preferences.getString("WifiSSID");                     // STA mode WiFi SSID (33b)
    strBuf.toCharArray(WifiSSID, 33);
    strBuf = preferences.getString("WifiPassword");                 // STA mode WiFi Password (64b)
    strBuf.toCharArray(WifiPassword, 64);
    strBuf = preferences.getString("APSSID");                       // AP mode Wifi SSID (33b)
    strBuf.toCharArray(APSSID, 33);
    strBuf = preferences.getString("APPassword");                   // AP mode WiFi password (64b)
    strBuf.toCharArray(APPassword, 64);/*
    preferences.getString("WifiStaticIP");      // WiFi static IP Address to claim (4b)
    IPBuf[0] = preferences.getUChar("WifiStaticIP0");
    IPBuf[1] = preferences.getUChar("WifiStaticIP1");
    IPBuf[2] = preferences.getUChar("WifiStaticIP2");
    IPBuf[3] = preferences.getUChar("WifiStaticIP3");
    WifiStaticIP = (IPBuf[1], IPBuf[2], IPBuf[3], IPBuf[4]);   // MQTT broker address
    preferences.getString("WifiGateway");         // Wifi internet gateway (4b)
    IPBuf[0] = preferences.getUChar("WifiGateway0");
    IPBuf[1] = preferences.getUChar("WifiGateway1");
    IPBuf[2] = preferences.getUChar("WifiGateway2");
    IPBuf[3] = preferences.getUChar("WifiGateway3");
    WifiGateway = (IPBuf[1], IPBuf[2], IPBuf[3], IPBuf[4]);   // MQTT broker address
    preferences.getString("WifiSubnetMask");   // WiFi subnet mask (4b)
    IPBuf[0] = preferences.getUChar("WifiSubnetMask0");
    IPBuf[1] = preferences.getUChar("WifiSubnetMask1");
    IPBuf[2] = preferences.getUChar("WifiSubnetMask2");
    IPBuf[3] = preferences.getUChar("WifiSubnetMask3");
    WifiSubnetMask = (IPBuf[1], IPBuf[2], IPBuf[3], IPBuf[4]);   // MQTT broker address
    preferences.getString("DNS1");                // Local DNS server (4b)
    IPBuf[0] = preferences.getUChar("DNS10");
    IPBuf[1] = preferences.getUChar("DNS11");
    IPBuf[2] = preferences.getUChar("DNS12");
    IPBuf[3] = preferences.getUChar("DNS13");
    DNS1 = (IPBuf[1], IPBuf[2], IPBuf[3], IPBuf[4]);   // MQTT broker address
    preferences.getString("DNS2");                 // Remote DNS server (4b)
    IPBuf[0] = preferences.getUChar("DNS20");
    IPBuf[1] = preferences.getUChar("DNS21");
    IPBuf[2] = preferences.getUChar("DNS22");
    IPBuf[3] = preferences.getUChar("DNS23");
    DNS2 = (IPBuf[1], IPBuf[2], IPBuf[3], IPBuf[4]);   // MQTT broker address
    preferences.getString("MQTTBrokerAddress");   // MQTT broker address (4b)  
    IPBuf[0] = preferences.getUChar("MQTTBrokerAddress0");
    IPBuf[1] = preferences.getUChar("MQTTBrokerAddress1");
    IPBuf[2] = preferences.getUChar("MQTTBrokerAddress2");
    IPBuf[3] = preferences.getUChar("MQTTBrokerAddress3");
    MQTTBrokerAddress = (IPBuf[1], IPBuf[2], IPBuf[3], IPBuf[4]);   // MQTT broker address*/
    mqttPort = preferences.getUChar("mqttPort");      // MQTT broker port
    hostname = preferences.getString("hostname");
    strBuf = preferences.getString("mqttCtrlTopic");
    strBuf.toCharArray(mqttCtrlTopic, 33);                // MQTT control topic
    strBuf = preferences.getString("mqttDataTopic");
    strBuf.toCharArray(mqttDataTopic, 33);                // MQTT data topic
    preferences.end();
    preferences.begin("prefsMQTT", false);/*
    IPBuf[0] = preferences.getUChar("MQTTBrokerAddress0");
    IPBuf[1] = preferences.getUChar("MQTTBrokerAddress1");
    IPBuf[2] = preferences.getUChar("MQTTBrokerAddress2");
    IPBuf[3] = preferences.getUChar("MQTTBrokerAddress3");
    MQTTBrokerAddress = (IPBuf[1], IPBuf[2], IPBuf[3], IPBuf[4]);   // MQTT broker address*/
    strBuf = preferences.getString("mqttTopic");                   // MQTT base topic
    strBuf.toCharArray(mqttDataTopic, 9);
    strBuf = preferences.getString("mqttCtrlTopic");                // MQTT control topic
    strBuf.toCharArray(mqttDataTopic, 33);
    strBuf = preferences.getString("mqttDataTopic");                // MQTT data topic
    strBuf.toCharArray(mqttDataTopic, 33);
    preferences.end();
    Serial.println(" Loaded.");
    }
  else {
    preferences.end();
    Serial.print("Preferences not found. Saving...");
    prefSave();
  }
}

void initWiFi() {                                 // Set up Wifi Connection
  #ifdef STAWIFI
  WiFi.mode(WIFI_STA);
  //WiFi.config(local_IP, gateway, subnet, DNS1, DNS2);
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
  #endif
  #ifdef APWIFI
  WiFi.mode(WIFI_AP);
  WiFi.softAP(APSSID);Serial.print("Initializing Access Point...");
  delay(1000);
  Serial.println(" Active.");
  Serial.print("IP Address is "); Serial.println(WiFi.softAPIP());
  #endif
}

void initMQTT() {                               // Set up MQTT 
  char mqttClient[23];
  
  strcpy(mqttCtrlTopic, mqttTopic);
  strcat(mqttCtrlTopic, "/ctrl");
  
  strcpy(mqttDataTopic, mqttTopic);
  strcat(mqttDataTopic, "/Data");
  
  strcpy(mqttClient, mqttTopic);
  strcat(mqttClient, "/Client");
  
  MQTTclient.setServer(mqttBroker, mqttPort);
  MQTTclient.setKeepAlive(90);                    // Keep connection alive for 90 seconds
  Serial.print("Connecting to MQTT broker...");
  while (!MQTTclient.connected()) {
    Serial.print("...");
    if (MQTTclient.connect(mqttClient)) {
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
  char timeCal[22];

  Serial.println("...");
  if ((bitRead(flags, 7))==true)  {
    DateTime now = rtc.now();
    Serial.print("Initial time: ");
    timestamp();
    #ifdef RTCDEBUG 
    Serial.print("Initial RTC Time: "); Serial.println(weather.rtcTime);
    #endif
  }
  configTime(NTP_RTC.gmtOffset_sec, NTP_RTC.daylightOffset_sec, NTPaddress);     // Set up NTP time
  getLocalTime(&timeinfo);                                // From NTP
  strftime(weather.ntpTime, 30, "%Y/%m/%d %H:%M:%S", &timeinfo);
  #ifdef RTCDEBUG 
  Serial.print("Server Time:  "); Serial.println(weather.ntpTime);  
  Serial.print("Calibrating Time: ");
  #endif
  strftime(timebuf, 5, "%Y", &timeinfo);
  int y = atoi(timebuf);
  Serial.print(y); Serial.print("/");
  strftime(timebuf, 3, "%m", &timeinfo);
  int mo = atoi(timebuf);
  Serial.print(mo); Serial.print("/");
  strftime(timebuf, 3, "%d", &timeinfo);
  int d = atoi(timebuf);
  Serial.print(d); Serial.print(" ");
  strftime(timebuf, 3, "%H", &timeinfo);
  int h = atoi(timebuf);
  timeHr = h;
  Serial.print(h); Serial.print(":");
  strftime(timebuf, 3, "%M", &timeinfo);
  int mi = atoi(timebuf);
  timeMin = mi;
  Serial.print(mi); Serial.print(":");
  strftime(timebuf, 3, "%S", &timeinfo);
  int s = atoi(timebuf);
  Serial.println(s);
  
  if ((bitRead(flags, 7))==true)  {
    rtc.adjust(DateTime(y, mo, d, h, mi, s));
    now = rtc.now();
  }
  timestamp();
  Serial.print("New Time: "); Serial.print(timeHr); Serial.print(":"); if (timeMin < 10) {Serial.print("0");} Serial.println(timeMin);
}

void timestamp() {
  char strBuf[5];
  int timeYr;
  int timeMo;
  int timeDay;

  #ifdef TIMEDEBUG
  Serial.println("Starting timestamp");
  #endif
  if ((bitRead(flags, 7))==true)  {
    DateTime now = rtc.now();
    timeYr = now.year();
    timeMo = now.month();
    timeDay = now.day();
    timeHr = now.hour();
    timeMin = now.minute();
    timeSec = now.second();
  }
  else  {    
    strftime(strBuf, 5, "%Y", &timeinfo);
    timeYr = atoi(strBuf);
    strftime(strBuf, 3, "%m", &timeinfo);
    timeMo = atoi(strBuf);
    strftime(strBuf, 3, "%d", &timeinfo);
    timeDay = atoi(strBuf);
    strftime(strBuf, 3, "%H", &timeinfo);
    timeHr = atoi(strBuf);
    strftime(strBuf, 3, "%M", &timeinfo);
    timeMin = atoi(strBuf);
    strftime(strBuf, 3, "%S", &timeinfo);
    timeSec = atoi(strBuf);
  }
  #ifdef TIMEDEBUG
  Serial.println("Values loaded");
  #endif

  itoa(timeYr, strBuf, 10);
  strcpy(weather.rtcTime, strBuf);
  strcat(weather.rtcTime, "/");
  
  if (timeMo < 10)  {strcat(weather.rtcTime, "0");}
  itoa(timeMo, strBuf, 10);
  strcat(weather.rtcTime, strBuf);
  strcat(weather.rtcTime, "/");
  
  if (timeDay < 10)  {strcat(weather.rtcTime, "0");}
  itoa(timeDay, strBuf, 10);
  strcat(weather.rtcTime, strBuf);
  strcat(weather.rtcTime, ", ");
  
  if (timeHr < 10)  {strcat(weather.rtcTime, "0");}
  itoa(timeHr, strBuf, 10);
  strcat(weather.rtcTime, strBuf);
  strcat(weather.rtcTime, ":");
  
  if (timeMin < 10)  {strcat(weather.rtcTime, "0");}
  itoa(timeMin, strBuf, 10);
  strcat(weather.rtcTime, strBuf);
  strcat(weather.rtcTime, ":");
  
  if (timeSec < 10)  {strcat(weather.rtcTime, "0");}
  itoa(timeSec, strBuf, 10);
  strcat(weather.rtcTime, strBuf);
  #ifdef RTCDEBUG 
  Serial.print("Timestamp: "); Serial.println(weather.rtcTime);
  #endif
}

void initSens() {
  unsigned long bootTime = millis();
  #ifdef TFTDISPLAY
  int cursorX = 64;

  tft.setTextColor(CBLACK);
  tft.println("Initializing Sensors:");
  #ifdef TFTDEBUG
  Serial.print("Cursor line set to: "); Serial.println(cursorX);
  #endif
  tft.println("[ ] Temp. sens. ready");
  tft.println("[ ] Multi sens. ready");
  tft.println("[ ] Wind vane ready");
  tft.println("[ ] Wind speed ready");
  tft.println("[ ] Rain sensor ready");
  tft.println("[ ] UV sensor ready");
  tft.println("[ ] Lux sensor ready");
  tft.setTextColor(CRED);
  #endif

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
  #ifdef TFTDISPLAY
  if (int(weather.temperature) != -40)  {
    drawCheck(6, cursorX);
  }
  else  {
    tft.setCursor(6, cursorX);
    tft.println("x");
  }
  delay(500);
  #endif

  // Set up BME280
  bool status = bme.begin(addrBME);
  bme.setSampling(Adafruit_BME280::MODE_FORCED,               // power mode
                  Adafruit_BME280::SAMPLING_X1,               // temperature sampling rate
                  Adafruit_BME280::SAMPLING_X1,               // pressure sampling rate
                  Adafruit_BME280::SAMPLING_X1,               // humidity sampling rate
                  Adafruit_BME280::FILTER_OFF,                // filtering mode
                  Adafruit_BME280::STANDBY_MS_1000);          // Standby time
  #ifdef BME280DEBUG
  Serial.print("-- Weather Station Scenario: "); Serial.println("Forced mode, 1x temperature / 1x humidity / 1x pressure oversampling, filtering off");
  #endif
  if (!status) {
    #ifdef BME280DEBUG
    Serial.println("BME/BMP Sensor not found. Continuing with faked readings.");
    #endif
    #ifdef TFTDISPLAY
    tft.setCursor(6, cursorX+8);
    tft.println("x");
    delay(500);
    #endif
  }
  else  {
    #ifdef BME280DEBUG
    Serial.println("BME/BMP sensor found. Getting readings.");
    #endif
    #ifdef TFTDISPLAY
    drawCheck(6, cursorX+8);
    delay(500);
    #endif
  }
  readBME();
  
  // Set up wind vane sensor
  pinMode(pinVane, INPUT);
  adcAttachPin(pinVane);
  analogSetAttenuation(ADC_11db);

  measWindDir();
  #ifdef TFTDISPLAY
  drawCheck(6, cursorX+16);
  delay(500);
  #endif
  #ifdef WINDDIRDEBUG
  Serial.print("Initial wind direction reading: "); Serial.println(weather.windDir);
  #endif

  // Set up anemometer
  pinMode (pinAnemo, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinAnemo), AnemoInterrupt, FALLING);
  for (int count = 0; count <= 199; count++)  {
    #ifdef WINDSPDDEBUG
    Serial.print("windCount["); Serial.print(count); Serial.print("]: "); Serial.print(count); Serial.print(" - "); Serial.print(weather.windCount[count]); Serial.println("/0");
    #endif
    weather.windCount[count] = 0;
  }
  weather.windSpeed[0]=0;
  #ifdef TFTDISPLAY
  drawCheck(6, cursorX+24);
  delay(500);
  #endif
  #ifdef WINDSPDDEBUG 
  Serial.print("Initial windspeed reading: "); Serial.print(weather.windSpeed[0]); Serial.println("km/h");
  #endif

  //Set up rain bucket gauge
  pinMode (pinBucket, INPUT_PULLUP);
  for (int count = 0; count <= 23; count++)  {
    weather.rainHr[count] = 0;
  }
  attachInterrupt(digitalPinToInterrupt(pinBucket), RainInterrupt, FALLING);
  #ifdef TFTDISPLAY
  drawCheck(6, cursorX+32);
  #endif

  //Set up ML8511
  pinMode (pinUV, INPUT);
  measUV();
  #ifdef TFTDISPLAY
  if (int(weather.uvReading) != 0)  {
    drawCheck(6, cursorX+40);
  }
  else  {
    tft.setCursor(6, cursorX+40);
    tft.println("x");
  }
  delay(500);
  #endif

  // Set up photoresistor
  pinMode (pinLux, INPUT);
  measLux();
  #ifdef TFTDISPLAY
  if (int(weather.rawLux) != -1)  {
    drawCheck(6, cursorX+48);
  }
  else  {
    tft.setCursor(6, cursorX+48);
    tft.println("x");
  }
  delay(500);
  #endif

  while (millis() < bootTime + 10000)  {delay(100);}
}

void displayInterrupt()  {
  bitSet(flags, 8);
  bitSet(flags, 9);
}

void setup() {    // put your setup code here, to run once:
  bool detectRTC;
  // Load preferences and settings
  prefLoad();
  
  // Set up interfaces
  Serial.begin(115200);                                         // Start serial output for monitor

  
  #ifdef STAWIFI
  Serial.println("Station Mode Active");
  #endif
  #ifdef APWIFI
  Serial.println("Access Point Mode Active");
  #endif

  initWiFi();                                              // Start WiFi connection in Station mode
  
  pinMode(pinDisplay, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(pinDisplay), displayInterrupt, RISING);
  #ifdef OLEDDISPLAY
  // Initialize OLED
  OLEDInit();
  #endif
  #ifdef TFTDISPLAY                                       // Initialize TFT display
  ledcSetup(channelPWM, freqPWM, 8);
  ledcAttachPin(pinBLK, channelPWM);
  TFTInit();
  #else
  pinMode(pinBLK, OUTPUT);
  digitalWrite(pinBLK, 0);
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
  #define WIRE Wire
  Serial.print("Checking for external RTC - ");
  WIRE.begin();
  WIRE.beginTransmission(addrRTC);
  byte error = WIRE.endTransmission();
  delay(1000);  

  if (error == 0)  {
    Serial.println("DS1307 RTC found.");
    bitSet(flags, 7);
    rtc.begin();
  }
  else  {
    Serial.println("DS1307 RTC not found.");
    bitClear(flags, 7);
  }
  Serial.println("Attempting to reach NTP server(s).");
  if (Ping.ping(NTP_RTC.ntpServer1))  {  
    #ifdef TFTDISPLAY
    tft.println("");                               // Attempt to correct RTC time dift if any
    tft.println("Get time (local NTP)");
    tft.println("");
    #endif
    #ifdef RTCDEBUG
    Serial.print("Updating RTC from local NTP server");
    #endif
    adjustRTC(NTP_RTC.ntpServer1);
  }
  else if (Ping.ping(NTP_RTC.ntpServer2))  {
    #ifdef TFTDISPLAY
    tft.println("Get time (remote NTP)");
    tft.println("");
    #endif
    #ifdef RTCDEBUG
    Serial.print("Unable to reach local NTP server, updating RTC from remote server");
    #endif
    adjustRTC(NTP_RTC.ntpServer2);
  }
  else  {
    #ifdef RTCDEBUG
    Serial.println("Unable to reach local or remote NTP servers, using stored time.");
    #endif
    #ifdef TFTDISPLAY
    tft.println("Time from RTC");
    tft.println("");
    #endif
  }


  // Initialize mDNS
  if (MDNS.begin("hostname")) {
    Serial.println("MDNS responder started");
    }

  webserverInit();

  server.begin();
  Serial.println("HTTP server started");

  // Initialize MQTT communication
  #ifdef STAWIFI
  initMQTT();

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
  #endif

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

  
  if (millis() >= (gustTime + 3000)) {                  //3 seconds
    #ifdef WINDSPDEBUG
    Serial.println("3 second reading");
    #endif
    timestamp();
    gustTime = millis();
    measWindSpd(3);
    #ifdef WINDSPDEBUG
    Serial.println("wind speed measured");
    #endif
    Serial.print("Display pin status: "); Serial.println(digitalRead(pinDisplay));
    if (digitalRead(pinDisplay)==1)  {
      if (bitRead(flags, 8)==0)  {      
        bitSet(flags, 8);
      }
      if (bitRead(flags, 9)==0)  {
        bitSet(flags, 9);
      }
    }
    #ifdef OLEDDISPLAY
    Serial.print("OLED flag status: "); Serial.println(bitRead(flags, 8));
    if (bitRead(flags, 8)==1)  {
      Serial.println("OLED On");
      UpdateOLED();
    }
    else  {
      Serial.println("OLED Off");
      oled.clearDisplay();
      oled.display();
    }
    #endif
    #ifdef TFTDISPLAY
    Serial.print("TFT flag status: "); Serial.println(bitRead(flags, 9));
    if (bitRead(flags, 9)==1)  {
      Serial.println("TFT On");
      UpdateTFT();
      #endif
    }
    else  {
      Serial.println("TFT Off");
      tft.fillScreen(CBLACK);
      ledcWrite(channelPWM, 0);
    }
  }
    
  if (millis() >= (windTime + 600000)) {                // 10 minutes
    #ifdef LOOPDEBUG
    Serial.println("10 minute reading");
    #endif
    windTime = millis();
    timestamp();
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
    timestamp();
    if ((isnan(weather.temperature)) || (isnan(weather.humidity))) {
      readBME();
    }
    measWindDir();
    measLux();
    
    if (timeMin == 0) {                                 // Every Hour
      #ifdef LOOPDEBUG 
      Serial.println("Hour change detected");
      #endif
      timestamp();
      readDS18B20();
      readBME();
      measUV();
      for (int count = 23; count > 0; count--) {
        weather.rainHr[count - 1] = weather.rainHr[count];
      }
      int index = int(timeHr);
      weather.rainHr[index] = 0;
      if ((weather.rainHr[timeHr] <= oldRainHr - 0.5) || (oldRainHr + 0.5 <= weather.rainHr[timeHr]))  {
        bitSet(flags, 1);  //Rain flag
      }
    }
  }
  #ifdef STAWIFI
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
    Serial.print("Ending flag Value: ");Serial.println(flags-128, BIN);
    #endif
    Serial.println(" ");
  }
  #endif
}
