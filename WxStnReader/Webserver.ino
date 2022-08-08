// WebsServer pages

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

//  Serial.print("Populating %"); Serial.print(var); Serial.print("% with ");
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
  else if (var == "TEMPERATURE")  {
    dtostrf(temperature, 4, 1, filler);
  }
  else if (var == "HUMIDITY")  {
    itoa(humidity, filler, 10);
  }
  else if (var == "DEWPOINT")  {
    dtostrf(dewpoint, 4, 1, filler);
  }
  else if (var == "WINDDIR")  {
    if ((strcmp(windDir, "N") == 0))  {strcpy(filler, "North");}
    else if ((strcmp(windDir, "NE") == 0))  {strcpy(filler, "Northeast");}
    else if ((strcmp(windDir, "E") == 0))  {strcpy(filler, "East");}
    else if ((strcmp(windDir, "SE") == 0))  {strcpy(filler, "Southeast");}
    else if ((strcmp(windDir, "S") == 0))  {strcpy(filler, "South");}
    else if ((strcmp(windDir, "SW") == 0))  {strcpy(filler, "Southwest");}
    else if ((strcmp(windDir, "W") == 0))  {strcpy(filler, "West");}
    else if ((strcmp(windDir, "NW") == 0))  {strcpy(filler, "Northwest");}
    else {strcpy(filler, "Unknown");}
  }
  else if (var == "WINDSPEED")  {
    dtostrf(windSpeed[timeHr / 10], 4, 2, filler);
  }
  else if (var == "WINDGUST")  {
    itoa(windGust, filler, 10);
  }
  else if (var == "HOURLYRAIN")  {
    dtostrf(rainHr[timeHr], 4, 2, filler);
  }
  else if (var == "DAILYRAIN")  {
    dtostrf(rainDaily, 4, 2, filler);
  }
  else if (var == "RAWPRESSURE")  {
    if (pressureRaw >= 100.0)  {dtostrf(pressureRaw, 4, 1, filler);}
    else  {dtostrf(pressureRaw, 4, 2, filler);}
  }
  else if (var == "SEALEVEL")  {
    if (pressureSeaLvl >= 100.0)  {dtostrf(pressureSeaLvl, 4, 1, filler);}
    else  {dtostrf(pressureSeaLvl, 4, 2, filler);}
  }
  else if (var == "BRIGHTNESS")  {
    strcpy(filler, lightLvl);
  }
  else if (var == "UVINDEX")  {
    itoa(uvIndex, filler, 10);
  }
  else  {
//    Serial.println("UNKNOwN VARIABLE!");
    strcpy(filler, "ERROR!");
  }
  #ifdef WEBDEBUG 
  Serial.print("Filling variable '"); Serial.print(var); Serial.print("' with '");Serial.print(filler); Serial.println("'");
  #endif
  return filler;
}

String settingsFill(const String& var) {
  Serial.print("Displaying Settings page");
}
