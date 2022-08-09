# ESP32-WxStn
Software suite to read data from a weather station with an ESP32 and send it via Wifi and MQTT to a python display. (Display not fyet functional.)

Upcoming features/changes:
  Version 1.1 - Addition of a functional settings page and SPIFFS/EEPROM storage to remove the need for full compile and upload on changes.

Changelog:
Version 1.0 - The first fully functional version of the weather station.
  Supported sensors and peripherals -
    SparkFun weather meter kit (Windspeed, Wind direction, Rainfall)
    DS18B20 temperature sensor
    BME280 temperature/humidity/pressure sensor
    ML8511 UV sensor
    Photoresistor
    Adafruit DS1307 RTC module
    SSD1306 128x64 OLED screen
    Battery backup detection when used with a supporting ESP32 board
    
  Features -
    Trasmits reading data via MQTT when connected to a WiFi network
    Creates a WiFi hotspot when no WiFi wetwork is found
    Hosts a webpage to display reading data
