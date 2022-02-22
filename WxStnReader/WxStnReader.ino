#include "WiFi.h"
#include "PubSubClient.h"

//Set up connection constants
const String hostname = "WxStation";            // Wifi hostname
const char* WifiSSID = "IT Hz When IP";         // Wifi SSID
const char* WifiPassword = "antibioticshelp";   // Wifi Password
// const char* WifiSSID = "AccioInternet";         // Wifi SSID
// const char* WifiPassword = "RepelloMuggles";    // Wifi Password  
const char* mqttBroker = "10.0.0.2";            // MQTT broker address
const int mqttPort = 1883;                      // MQTT broker port
const char* mqttCtrlTopic = "/WxStn/Cmd";       // MQTT control topic
const char* mqttDataTopic = "WxStn/Tele";       // MQTT data topic
WiFiClient WxWiFiClient;
PubSubClient client(WxWiFiClient);

// Set up data variables 
int reportNum = 0;   // Report count number(in place of a timestamp?)
float windSpeed = 0;          // Windspeed (km/h)
char windDir = 'N';           // Wind Direction (combined quadrants: 16 possibilities)
float rainMin = 0;            // Rainfall (mm in last minute)
float rainHr = 0;             // Rainfall (mm in last hour)
int temperature = 20;         // Temperature (deg C)
float pressure = 101.25;      // Barometric Pressure (kPa)
int humidity = 50;           // Humidity (%)

// Function globals
unsigned long bootTime = millis();
unsigned long lastUpMillis = bootTime;
byte MinuteTimer = 0;
byte HourTimer = 0;

void initWiFi() {                                 // Set up Wifi Connection
  WiFi.mode(WIFI_STA);  
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(hostname.c_str()); //define hostname
  WiFi.begin(WifiSSID, WifiPassword);
    Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("...");
  }
  Serial.println(" Connected.");
  Serial.print("IP Address is ");
  Serial.println(WiFi.localIP());
  //Serial.println(WiFi.macAddress());
}

void initMQTT() {                     // Set up MQTT 
  client.setServer(mqttBroker, mqttPort);
  Serial.print("Connecting to MQTT broker...");
  while (!client.connected()) {
    Serial.print("...");
    if (client.connect("WxStnClient")) {
      Serial.println("Connected");
    }
    else {
      Serial.print("Failed - Error state ");
      Serial.println(client.state());
      delay(1000);
    }
  }
}

char pubTelemetry(){
//  char telemetryKey[20] = "0";
//  char telemetryValue[6] = "0";
  char* telemetryMessage[175] = {0};
  Serial.println("Telemetry Published.");

/* Desired output string format
  'ReportNumber':12345,   'WindSpeed':13245, 'WindDirection':12345, 'RainMinute':12345, 'RainHour':12345, 'Temperature':12345, 'Pressure':12345, 'Humidity':12345
*/ 
  strcpy(telemetryMessage, "'ReportNumber':");
  strcat(telemetryMessage, reportNum);
  strcat(telemetryMessage, ", 'WindSpeed':");
  strcat(telemetryMessage, windSpeed);
  strcat(telemetryMessage, ", 'WindDirection':");
  strcat(telemetryMessage, windDir);
  strcat(telemetryMessage, ", 'RainMinute':");
  strcat(telemetryMessage, rainMin);
  strcat(telemetryMessage, ", 'RainHour':");
  strcat(telemetryMessage, rainHr);
  strcat(telemetryMessage, ", 'Temperature':");
  strcat(telemetryMessage, temperature);
  strcat(telemetryMessage, ", 'Pressure':");
  strcat(telemetryMessage, pressure);
  strcat(telemetryMessage, ", 'Humidity':");
  strcat(telemetryMessage, humidity);
  Serial.println(telemetryMessage);
//  client.publish("WeatherStation/Tele", telemetryMessage);
  client.publish("WeatherStation/Tele", "TestMessage");
  Serial.println("Telemetry Published.");

}

void measWind() {
  float speedIncrement = 2.4/60; // 60 switch closures per second translates to 2.4km/h windspeed
  int startTime = millis();
  int endTime = startTime+1000;
  while (millis() <= endTime) {
    // Method 1: watch for and count switch closures for one second
    // Method 2: measure time between the next 2 switch closures
  }
}
  
void detectRainfall(){
  float bucketIncrement = 0.2794;
  rainMin=rainMin+bucketIncrement;
  rainHr=rainHr+bucketIncrement;
}

void readBME(){
  temperature= 20;    // Temperature (deg C)
  pressure= 101.25;   // Barometric Pressure (kPa)
  humidity= 50;       // Humidity (%)
}

void getData(){
  reportNum=reportNum+1;  // Report count number(in place of a timestamp?)
  measWind();             // Measure wind speed (km/h) and direction (combined quadrants: 16 possibilities)
  readBME();              // Read conditions from BME chip
} 

void setup() {
  // put your setup code here, to run once:

  /*
  // Define and set up IO pins and variables: 
  byte pinAnemo = 3;  // Anemometer (Digital)
  byte pinVane = 4;   // Vane (Analog)
  byte pinBucket = 5; // Bucket (Digital)
  byte pinBME1 = 36;  // BME module (I2C SCL)
  byte pinBME2 = 33;  // BME module (I2C SDA)
  byte pinOK = 9;     // Status LED (Green/OK)
  byte pinERR = 10;   // Status LED (Red/Error)
  */

 // Set up serial monitor
  Serial.begin(115200);   // Start serial output for monitor

  initWiFi();             // Start WiFi connection
  
  initMQTT();             // Start MQTT communication

}

void loop() {
  // put your main code here, to run repeatedly:
  pubTelemetry();
  delay(1000);

/*  bootTime = millis();
  float secondsUp = bootTime/1000;
  byte minutesUp = secondsUp/60;
  byte hoursUp = minutesUp/60;
  byte daysUp = hoursUp/24;
  if (bootTime >= lastUpMillis+1000) {
    if (daysUp > 0);
      hoursUp = hoursUp-(daysUp*24);
    if (hoursUp > 0);
      minutesUp = minutesUp-(hoursUp*60)-(daysUp*24*60);
    if (minutesUp > 0);
      secondsUp = secondsUp-(minutesUp*60)-(hoursUp*3600)-(daysUp*24*3600);

    if (daysUp >= 7) {
      Serial.println("Preparing to restart.");
      //Reset ESP32 controller
    }
    else {
      Serial.print("Time since last reset: ");
      Serial.print(daysUp);
      if (hoursUp < 10) {
        Serial.print(" days, 0");
      }
      else {
        Serial.print(" days, ");
      } 
      Serial.print(hoursUp);
      if (minutesUp < 10) {
        Serial.print(":0");
      }
      else {
        Serial.print(":");
      } 
      Serial.print(minutesUp);
      if (secondsUp < 10) {
        Serial.print(":0");
      }
      else {
      Serial.print(":");
      } 
      Serial.println(secondsUp);
      lastUpMillis = bootTime;
    }
  }
*/  
}
