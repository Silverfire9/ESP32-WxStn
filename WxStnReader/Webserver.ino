void webserverInit()  {          // Initialize Webserver
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest * request) {       // Route to load combined stylesheet
    request->send(SPIFFS, "/style.css", "text/css");
  });
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest * request) {       // Route to load mobile stylesheet
    request->send(SPIFFS, "/mobile.css", "text/css");
  });
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest * request) {       // Route to load desktop stylesheet
    request->send(SPIFFS, "/desktop.css", "text/css");
  });
  server.on("/wxstn.ico", HTTP_GET, [](AsyncWebServerRequest *request){         // Route for favicon
  request->send(SPIFFS, "/wxstn.ico", "image/png");
  });
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {                // Route 1 for main page
    request->send(SPIFFS, "/main.html", String(), false, mainFill);
  });
  server.on("/main.html", HTTP_GET, [](AsyncWebServerRequest * request) {       // Route 2 for main page
    request->send(SPIFFS, "/main.html", String(), false, mainFill);
  });
  server.on("/settings.html", HTTP_GET, [](AsyncWebServerRequest * request) {   // Route for settings page
    request->send(SPIFFS, "/settings.html", String(), false, settingsFill);
  });
  server.on("/wait.html", HTTP_GET, [](AsyncWebServerRequest * request) {       // Route for waiting page
    request->send(SPIFFS, "/wait.html");
  });
  server.on("/solar.png", HTTP_GET, [](AsyncWebServerRequest *request){         // Route for solar icon
  request->send(SPIFFS, "/solar.png", "image/png");
  });
  server.on("/batt.png", HTTP_GET, [](AsyncWebServerRequest *request){          // Route for battery icon
  request->send(SPIFFS, "/batt.png", "image/png");
  });
  server.on("/temperature.png", HTTP_GET, [](AsyncWebServerRequest * request) { // Route for temperature icon
    request->send(SPIFFS, "/temperature.png", "image/png");
  });
  server.on("/humidity.png", HTTP_GET, [](AsyncWebServerRequest *request){      // Route for humidity icon
  request->send(SPIFFS, "/humidity.png", "image/png");
  });
  server.on("/wind.png", HTTP_GET, [](AsyncWebServerRequest *request){          // Route for wind icon
  request->send(SPIFFS, "/wind.png", "image/png");
  });
  server.on("/rain.png", HTTP_GET, [](AsyncWebServerRequest *request){          // Route for rainfall icon
  request->send(SPIFFS, "/rain.png", "image/png");
  });
  server.on("/pressure.png", HTTP_GET, [](AsyncWebServerRequest *request){      // Route for pressure icon
  request->send(SPIFFS, "/pressure.png", "image/png");
  });
  server.on("/light.png", HTTP_GET, [](AsyncWebServerRequest *request){         // Route for light icon
  request->send(SPIFFS, "/light.png", "image/png");
  });
  server.on("/clearsky.jpg", HTTP_GET, [](AsyncWebServerRequest *request){      // Route for background
  request->send(SPIFFS, "/clearsky.jpg", "image/png");
  });
}

String mainFill(const String& var) {
  char filler[10];
  strcpy(filler, "Placehldr");

  //Serial.print("Populating %"); Serial.print(var); Serial.print("% with ");
  if (var == "POWER")  {
    if (digitalRead(pinPower) == 1)  {
      strcpy(filler, "solar.png");
    }
    else {
      strcpy(filler, "batt.png");
    }
  }
  else if (var == "HOUR")  {
    if (digitalRead(pinPower) == 1)  {
      char strBuf[3];
      strcpy(filler, ("at "));
      itoa(timeHr, strBuf, 10);
      strcat(filler, strBuf);
    }
    else  {strcpy(filler, "N/A");}
  }
  else if (var == "WIDE")  {
    if (digitalRead(pinPower) == 1)  {
      strcpy(filler, "25px");
    }
    else  {
      strcpy(filler, "60px");
    }
  }
  else if (var == "MINUTE")  {
    if (digitalRead(pinPower) == 1)  {
      if (timeMin < 10)  {
        char strBuf[3];
        strcpy(filler, "0");
        itoa(timeMin, strBuf, 10);
        strcat(filler, strBuf);
      }
      else  {
        itoa(timeMin, filler, 10);
      }
    }
    else  {strcpy(filler, "N/A");}
  }
  else if (var == "RAWTEMP")  {
    dtostrf(weather.temperature, 4, 1, filler);
  }
  else if (var == "FEELTEMP")  {
    dtostrf(weather.feelTemp, 4, 1, filler);
  }
  else if (var == "HUMIDITY")  {
    itoa(weather.humidity, filler, 10);
  }
  else if (var == "DEWPOINT")  {
    dtostrf(weather.dewpoint, 4, 1, filler);
  }
  else if (var == "WINDDIR")  {
    if ((strcmp(weather.windDir, "N") == 0))  {strcpy(filler, "North");}
    else if ((strcmp(weather.windDir, "NE") == 0))  {strcpy(filler, "Northeast");}
    else if ((strcmp(weather.windDir, "E") == 0))  {strcpy(filler, "East");}
    else if ((strcmp(weather.windDir, "SE") == 0))  {strcpy(filler, "Southeast");}
    else if ((strcmp(weather.windDir, "S") == 0))  {strcpy(filler, "South");}
    else if ((strcmp(weather.windDir, "SW") == 0))  {strcpy(filler, "Southwest");}
    else if ((strcmp(weather.windDir, "W") == 0))  {strcpy(filler, "West");}
    else if ((strcmp(weather.windDir, "NW") == 0))  {strcpy(filler, "Northwest");}
    else {strcpy(filler, "Unknown");}
  }
  else if (var == "WINDSPEED")  {
    dtostrf(weather.windSpeed[timeHr / 10], 4, 2, filler);
  }
  else if (var == "WINDGUST")  {
    itoa(weather.windGust, filler, 10);
  }
  else if (var == "HOURLYRAIN")  {
    dtostrf(weather.rainHr[timeHr], 4, 2, filler);
  }
  else if (var == "DAILYRAIN")  {
    dtostrf(weather.rainDaily, 4, 2, filler);
  }
  else if (var == "RAWPRESSURE")  {
    if (weather.pressureRaw >= 100.0)  {dtostrf(weather.pressureRaw, 4, 1, filler);}
    else  {dtostrf(weather.pressureRaw, 4, 2, filler);}
  }
  else if (var == "SEALEVEL")  {
    if (weather.pressureSeaLvl >= 100.0)  {dtostrf(weather.pressureSeaLvl, 4, 1, filler);}
    else  {dtostrf(weather.pressureSeaLvl, 4, 2, filler);}
  }
  else if (var == "BRIGHTNESS")  {
    strcpy(filler, weather.lightLvl);
  }
  else if (var == "UVINDEX")  {
    itoa(weather.uvIndex, filler, 10);
  }
  else  {
    Serial.print("UNKNOwN VARIABLE! - ");
    Serial.println(var);
    strcpy(filler, "ERROR!");
  }
  #ifdef WEBDEBUG 
  Serial.print("Filling variable '"); Serial.print(var); Serial.print("' with '");Serial.print(filler); Serial.println("'");
  #endif
  return filler;
}

String settingsFill(const String& var) {
  //Serial.println("Displaying Settings page");
  char filler[20];
  strcpy(filler, "Placehldr");

  if (var== "HOSTNAME")  {
    hostname.toCharArray(filler, 20);
  }
  else if (var == "STAMode")  {
    #ifdef STAWIFI
    strcpy(filler, "checked");
    #endif
    #ifndef STAWIFI
    strcpy(filler, "");
    #endif
  }
  else if (var == "APMode")  {
    
    #ifdef APWIFI
    strcpy(filler, "checked");
    #endif
    #ifndef APWIFI
    strcpy(filler, "");
    #endif
  }
  else if (var == "WIFISSID")  {
    #ifdef STAWIFI
    strcpy(filler, "\"");
    strcat(filler, WifiSSID);
    strcat(filler, "\"");
    #endif
    #ifdef APWIFI
    strcpy(filler, APSSID);
    #endif
  }
  else if (var == "WIFIPASS")  {
    strcpy(filler, "\"");
    strcat(filler, WifiPassword);
    strcat(filler, "\"");
  }
  else if (var == "StaticIP")  {
    strcpy(filler, "unchecked");
  }
  else if (var == "GATEWAY0")  {
    IPAddress currentGateway = WiFi.gatewayIP(); 
    itoa(currentGateway[0], filler, 10);
  }
  else if (var == "GATEWAY1")  {
    IPAddress currentGateway = WiFi.gatewayIP();
    itoa(currentGateway[1], filler, 10);
  }
  else if (var == "GATEWAY2")  {
    IPAddress currentGateway = WiFi.gatewayIP();
    itoa(currentGateway[2], filler, 10); 
  }
  else if (var == "GATEWAY3")  {
    IPAddress currentGateway = WiFi.gatewayIP(); 
    itoa(currentGateway[3], filler, 10);
  }
  else if (var == "STATICIP0")  {
    IPAddress currentIP = WiFi.localIP(); 
    itoa(currentIP[0], filler, 10);
  }
  else if (var == "STATICIP1")  {
    IPAddress currentIP = WiFi.localIP();
    itoa(currentIP[1], filler, 10);
  }
  else if (var == "STATICIP2")  {
    IPAddress currentIP = WiFi.localIP();
    itoa(currentIP[2], filler, 10); 
  }
  else if (var == "STATICIP3")  {
    IPAddress currentIP = WiFi.localIP(); 
    itoa(currentIP[3], filler, 10);
  }
  else if (var == "SUBMASK0")  {
    IPAddress currentSubMask = WiFi.subnetMask(); 
    itoa(currentSubMask[0], filler, 10);
  }
  else if (var == "SUBMASK1")  {
    IPAddress currentSubMask = WiFi.subnetMask(); 
    itoa(currentSubMask[1], filler, 10);
  }
  else if (var == "SUBMASK2")  {
    IPAddress currentSubMask = WiFi.subnetMask(); 
    itoa(currentSubMask[2], filler, 10);
  }
  else if (var == "SUBMASK3")  {
    IPAddress currentSubMask = WiFi.subnetMask(); 
    itoa(currentSubMask[3], filler, 10);
  }
  else if (var == "MQTTIP0")  {
    itoa(mqttBroker[0], filler, 10);
  }
  else if (var == "MQTTIP1")  { 
    itoa(mqttBroker[1], filler, 10);
  }
  else if (var == "MQTTIP2")  {
    itoa(mqttBroker[2], filler, 10);
  }
  else if (var == "MQTTIP3")  {
    itoa(mqttBroker[3], filler, 10);
  }
  else if (var == "MQTTPORT")  {
    itoa(mqttPort, filler, 10);
  }
  else if (var == "MQTTTOPIC")  {
   strcpy(filler, mqttTopic);
  }
  else if (var == "NTPSERVER1")  {
   strcpy(filler, NTP_RTC.ntpServer1);
  }
  else if (var == "NTPSERVER2")  {
   strcpy(filler, NTP_RTC.ntpServer2);
  }
  else if (var == "UTCOFFSET")  {
    int Buffer;
    Buffer = NTP_RTC.gmtOffset_sec/3600;
    itoa(Buffer, filler, 10);
  }
  else if (var == "USEDST")  {
   if (NTP_RTC.daylightOffset_sec != 0)  {
    strcpy(filler, "checked");
   }
   else  {
    strcpy(filler, "unchecked");
   }
  }
  else if (var == "DSTOFFSET")  {
    int Buffer;
    Buffer = NTP_RTC.daylightOffset_sec/60;
    itoa(Buffer, filler, 10);
  }
  else if (var == "DSTSTARTMO")  {
   strcpy(filler, "1");
  }
  else if (var == "DSTSTARTWK")  {
   strcpy(filler, "1");
  }
  else if (var == "DSTSTARTTIME")  {
   strcpy(filler, "02:00:00");
  }
  else if (var == "DSTENDMO")  {
   strcpy(filler, "1");
  }
  else if (var == "DSTENDWK")  {
   strcpy(filler, "1");
  }
  else if (var == "DSTENDTIME")  {
   strcpy(filler, "02:00:00");
  }
  #ifdef OLEDDISPLAY || TFTDISPLAY
  else if (var == "SCREENALWAYSON")  {
   strcpy(filler, "checked");
  }
  #else
  else if (var == "SCREENALWAYSON")  {
  strcpy(filler, "unchecked");
  }
  #endif
  else  {
    Serial.print("UNKNOwN VARIABLE! - ");
    Serial.println(var);
    strcpy(filler, "ERROR!");
  }  
  return filler;
}
