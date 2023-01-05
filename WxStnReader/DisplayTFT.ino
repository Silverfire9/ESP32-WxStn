#ifdef TFTDISPLAY
void TFTInit()  {
  int xpos;

  digitalWrite(pinBLK, 0);
  Serial.print("Initializing ST7735 display at "); Serial.print(SCREEN_WIDTH_C); Serial.print("x"); Serial.println(SCREEN_HEIGHT_C);
  tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab
  tft.setRotation(2);
  tft.fillScreen(CSKYBLUE);
  tft.fillRect(0,0,128,10,CBLACK);
  tft.setTextColor(CYELLOW);
  tft.setTextSize(1);
  tft.setCursor(17, 0);
  tft.println("Redfield Weather");        //16 characters (96px)
  tft.setCursor(0, 16);
  tft.setTextColor(CBLACK);
  #ifdef APWIFI
  tft.println("WiFi (No password):");   //19 characters (114px)
  tft.println(APSSID);
  tft.println("");
  tft.println("See web display at:");   //19 characters (114px)
  tft.println(WiFi.softAPIP());          //16 characters max. (96px)
  #endif
  #ifdef STAWIFI
  tft.println("See web display at:");   //19 characters (114px)
  tft.println(WiFi.localIP());          //16 characters max. (96px)
  #endif
  for (int i=0; i<=user_settings.screen_bright; i++)  {
    #ifdef TFTDEBUG
    Serial.print("Screen brightness: "); Serial.println(i);
    #endif
    ledcWrite(channelPWM, i);
    delay(25);
  }
}

void drawCheck(int xpos, int ypos)  {
  
  #ifdef TFTDEBUG
  Serial.print("Draw checkmark at ("); Serial.print(xpos); Serial.print(", "); Serial.print(ypos); Serial.println(").");
  #endif
  tft.drawLine(xpos+2, ypos+5, xpos+4, ypos+1, CDKGREEN);
  tft.drawLine(xpos+1, ypos+6, xpos+4, ypos+2, CDKGREEN);
  tft.drawPixel(xpos, ypos+4, CDKGREEN);
  tft.drawPixel(xpos, ypos+6, CDKGREEN);
}

void drawRose()  {
  const int baseX = 63;
  const int baseY = 80;

  
  #ifdef TFTDEBUG
  Serial.print("Draw compass rose centered at ("); Serial.print(baseX); Serial.print(", "); Serial.print(baseY); Serial.println(").");
  #endif
  tft.fillCircle(baseX, baseY, 6, CRED);
  if ((strcmp(weather.windDir, "N") == 0))  {
    tft.drawFastVLine(baseX-1, baseY, -19, CRED);
    tft.drawFastVLine(baseX, baseY, -44, CRED);
    tft.drawFastVLine(baseX+1, baseY, -19, CRED);
  }
  else if ((strcmp(weather.windDir, "NE") == 0))  {
    tft.drawLine(baseX, baseY-1, baseX+11, baseY-12, CRED);
    tft.drawLine(baseX, baseY, baseX+25, baseY-25, CRED);
    tft.drawLine(baseX+1, baseY, baseX+12, baseY-11, CRED);
    }
  else if ((strcmp(weather.windDir, "E") == 0))  {
    tft.drawFastHLine(baseX, baseY-1, 19, CRED);
    tft.drawFastHLine(baseX, baseY, 44, CRED);
    tft.drawFastHLine(baseX, baseY+1, 19, CRED);
  }
  else if ((strcmp(weather.windDir, "SE") == 0))  {
    tft.drawLine(baseX+1, baseY, baseX+12, baseY+11, CRED);
    tft.drawLine(baseX, baseY, baseX+25, baseY+25, CRED);
    tft.drawLine(baseX, baseY+1, baseX+11, baseY+12, CRED);
    }
  else if ((strcmp(weather.windDir, "S") == 0))  {
    tft.drawFastVLine(baseX-1, baseY, 19, CRED);
    tft.drawFastVLine(baseX, baseY, 44, CRED);
    tft.drawFastVLine(baseX+1, baseY, 19, CRED);
  }
  else if ((strcmp(weather.windDir, "SW") == 0))  {
    tft.drawLine(baseX-1, baseY, baseX-12, baseY+11, CRED);
    tft.drawLine(baseX, baseY, baseX-25, baseY+25, CRED);
    tft.drawLine(baseX, baseY+1, baseX-11, baseY+12, CRED);
    }
  else if ((strcmp(weather.windDir, "W") == 0))  {
    tft.drawFastHLine(baseX, baseY-1, -19, CRED);
    tft.drawFastHLine(baseX, baseY, -44, CRED);
    tft.drawFastHLine(baseX, baseY+1, -19, CRED);
    }
  else if ((strcmp(weather.windDir, "NW") == 0))  {
    tft.drawLine(baseX, baseY-1, baseX-11, baseY-12, CRED);
    tft.drawLine(baseX, baseY, baseX-25, baseY-25, CRED);
    tft.drawLine(baseX-1, baseY, baseX-12, baseY-11, CRED);
    }
  
  tft.drawCircle(baseX, baseY, 6, CBLACK);
  tft.drawCircle(baseX, baseY, 16, CBLACK);
  tft.drawCircle(baseX, baseY, 19, CBLACK);
  tft.drawCircle(baseX, baseY, 20, CBLACK);
  tft.drawCircle(baseX, baseY, 33, CBLACK);
  tft.drawCircle(baseX, baseY, 34, CBLACK);
  tft.drawCircle(baseX, baseY, 38, CBLACK);
  tft.drawCircle(baseX, baseY, 39, CBLACK);
  tft.drawLine(baseX-2, baseY-6, baseX, baseY-56, CBLACK);    // North
  tft.drawLine(baseX+2, baseY-6, baseX, baseY-56, CBLACK);
  tft.drawLine(baseX+3, baseY-5, baseX+33, baseY-33, CBLACK);    // Northeast
  tft.drawLine(baseX+5, baseY-3, baseX+33, baseY-33, CBLACK);
  tft.drawLine(baseX+7, baseY-2, baseX+56, baseY, CBLACK);   // East
  tft.drawLine(baseX+7, baseY+2, baseX+56, baseY, CBLACK);
  tft.drawLine(baseX+5, baseY+3, baseX+33, baseY+33, CBLACK);   // Southeast
  tft.drawLine(baseX+3, baseY+5, baseX+33, baseY+33, CBLACK);
  tft.drawLine(baseX-2, baseY+7, baseX, baseY+56, CBLACK);   // South
  tft.drawLine(baseX+2, baseY+7, baseX, baseY+56, CBLACK);
  tft.drawLine(baseX-3, baseY+5, baseX-33, baseY+33, CBLACK);   // Southwest
  tft.drawLine(baseX-5, baseY+3, baseX-33, baseY+33, CBLACK);
  tft.drawLine(baseX-7, baseY-2, baseX-56, baseY, CBLACK);     // West
  tft.drawLine(baseX-7, baseY+2, baseX-56, baseY, CBLACK);
  tft.drawLine(baseX-5, baseY-3, baseX-33, baseY-33, CBLACK);    // Northwest
  tft.drawLine(baseX-3, baseY-5, baseX-33, baseY-33, CBLACK);
}

void drawIcon(int xpos, int ypos, char icon[]="Wind")  {
  if (strcmp("Wind", icon)==0)  {
    #ifdef TFTDEBUG
    Serial.print("Draw wind icon at ("); Serial.print(xpos); Serial.print(", "); Serial.print(ypos); Serial.println(").");
    #endif
    tft.drawFastHLine(xpos, ypos+3, 16, CBLACK);
    tft.drawPixel(xpos+16, ypos+2, CBLACK);
    tft.drawPixel(xpos+16, ypos+1, CBLACK);
    tft.drawPixel(xpos+15, ypos, CBLACK);
    tft.drawPixel(xpos+14, ypos, CBLACK);
    tft.drawPixel(xpos+13, ypos+1, CBLACK);
    tft.drawPixel(xpos+12, ypos+1, CBLACK);
    tft.drawFastHLine(xpos, ypos+6, 21, CBLACK);
    tft.drawPixel(xpos+21, ypos+5, CBLACK);
    tft.drawPixel(xpos+21, ypos+4, CBLACK);
    tft.drawPixel(xpos+20, ypos+3, CBLACK);
    tft.drawPixel(xpos+19, ypos+3, CBLACK);
    tft.drawPixel(xpos+18, ypos+4, CBLACK);
    tft.drawPixel(xpos+17, ypos+4, CBLACK);
    tft.drawFastHLine(xpos, ypos+8, 19, CBLACK);
    tft.drawPixel(xpos+19, ypos+9, CBLACK);
    tft.drawPixel(xpos+19, ypos+10, CBLACK);
    tft.drawPixel(xpos+18, ypos+11, CBLACK);
    tft.drawPixel(xpos+17, ypos+11, CBLACK);
    tft.drawPixel(xpos+16, ypos+10, CBLACK);
    tft.drawPixel(xpos+15, ypos+10, CBLACK);
    tft.drawFastHLine(xpos, ypos+11, 14, CBLACK);
    tft.drawPixel(xpos+14, ypos+12, CBLACK);
    tft.drawPixel(xpos+14, ypos+13, CBLACK);
    tft.drawPixel(xpos+13, ypos+14, CBLACK);
    tft.drawPixel(xpos+12, ypos+14, CBLACK);
    tft.drawPixel(xpos+11, ypos+13, CBLACK);
    tft.drawPixel(xpos+10, ypos+13, CBLACK);
  }
  else  {
    Serial.println("Unkown icon requested");
  }
}

void UpdateTFT()  {
  static int parameter = 1;
  char dataName[22];
  char dataValue[16];
  char strBuf[10];

  tft.fillScreen(CSKYBLUE);
  tft.setTextColor(CBLACK, CSKYBLUE);
  ledcWrite(channelPWM, user_settings.screen_bright);
  tft.setTextSize(1);
  tft.setCursor(0,0);
  
  if (parameter == 1)  {          // Temperature
    parameter++;
    #ifdef TFTDEBUG
    Serial.println("Display temperature info");
    #endif
    tft.setCursor(31 ,0);
    strcpy(dataName, "Temperature");      //11 characters (66px)
    tft.println(dataName);
    tft.setCursor(0,16);
    tft.println("Actual Temperature:");
    tft.setTextSize(2);
    tft.setCursor(0,24);
    dtostrf(weather.temperature, 4, 1, dataValue);
    tft.print(dataValue);
    tft.print(char(247));
    tft.println("C");
    tft.setCursor(0,40);
    tft.setTextSize(1);
    tft.println("Feels like:");
    tft.setTextSize(2);
    tft.setCursor(0,48);
    dtostrf(weather.temperature, 4, 1, dataValue);
    tft.print(dataValue);
    tft.print(char(247));
    tft.println("C");
  }
  else if (parameter == 2)  {          // Humidity
    parameter++;
    #ifdef TFTDEBUG
    Serial.println("Display humidity info");
    #endif
    tft.setCursor(40,0);
    tft.println("Humidity");        //17 characters (102px)
    tft.setCursor(0,16);
    tft.println("Relative Humidity:");
    tft.setTextSize(2);
    tft.setCursor(0,24);
    tft.print(weather.humidity);
    tft.println("%");
    tft.setCursor(0,40);
    tft.setTextSize(1);
    tft.println("Dewpoint:");
    tft.setTextSize(2);
    tft.setCursor(0,48);
    tft.print(weather.dewpoint);
    tft.print(char(247));
    tft.println("C");
  }
  else if (parameter == 3)  {          // Wind
    parameter++;
    #ifdef TFTDEBUG
    Serial.println("Display wind info");
    #endif
    char bufSpd[4];
    char bufGust[4];
    int bufText;

    tft.setTextSize(2);
    tft.setCursor(40, 0);
    tft.print("Wind");         // 4 characters (16px)
    drawRose();
    tft.setTextSize(1);
    itoa(int(weather.windSpeed[timeHr/10]), bufSpd, 10);
    Serial.print("Speed buffer: "); Serial.println(bufSpd);
    itoa(int(weather.windGust), bufGust, 10);
    Serial.print("gust buffer: "); Serial.println(bufGust);
    tft.setCursor((25-((strlen(bufSpd))*6)), 144);
    tft.print(bufSpd);
    tft.print("km/h");
    drawIcon(53, 140);
    tft.setCursor(80, 144);
    tft.print(bufGust);
    tft.print("km/h");
  }
  else if (parameter == 4)  {          // Rain
    parameter++;
    #ifdef TFTDEBUG
    Serial.println("Display rain info");
    #endif
    tft.setTextSize(2);
    tft.setCursor(16, 0);
    tft.println("Rainfall");          // 8 characters (48/96px)
    tft.setTextSize(1);
    tft.setCursor(0, 16);
    tft.println("Last Hour:");
    tft.setCursor(0, 24);
    tft.setTextSize(2);
    tft.println(weather.rainHr[timeHr]);
    tft.setCursor(0,40);
    tft.setTextSize(1);
    tft.println("Last 24 hours:");
    tft.setTextSize(2);
    tft.setCursor(0,48);
    tft.println(weather.rainDaily);
  }
  else if (parameter == 5)  {          // Pressure
    parameter++;
    #ifdef TFTDEBUG
    Serial.println("Display pressure info");
    #endif
    tft.setTextSize(2);
    tft.setCursor(16, 0);
    tft.println("Pressure");          // 8 characters (48/96px)
    tft.setTextSize(1);
    tft.setCursor(0, 16);
    tft.println("Raw Pressure:");
    tft.setTextSize(2);
    tft.setCursor(0,24);
    tft.println(weather.pressureRaw);
    tft.setTextSize(1);
    tft.setCursor(0, 40);
    tft.println("Sea Level Pressure:");
    tft.setTextSize(2);
    tft.setCursor(0,48);
    tft.println(weather.pressureSeaLvl);
  }
  else if (parameter == 6)  {          // Light
    parameter = 1;
    #ifdef TFTDEBUG
    Serial.println("Display light info");
    #endif
    tft.setTextSize(2);
    tft.setCursor(34, 0);
    tft.println("Light");         // 5 characters (30/60px)
    tft.setTextSize(1);
    tft.println("Brightness:");
    tft.setTextSize(2);
    tft.println(weather.lightLvl);
    tft.setTextSize(1);
    tft.println("Raw UV Reading:");
    tft.setTextSize(2);
    itoa(weather.uvReading, strBuf, 10);
    strcpy(dataValue, strBuf);
    strcat(dataValue, "/");
    dtostrf(uvIndexV, 1, 2, strBuf);
    strcat(dataValue, strBuf);
    tft.println(dataValue);
    tft.setTextSize(1);
    tft.println("UV Index:");
    tft.setTextSize(2);
    tft.println(weather.uvIndex);
    bitClear(flags, 9);
  }
}
#endif
