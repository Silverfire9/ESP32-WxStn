#ifdef OLEDDISPLAY
void OLEDInit()  { 
  int IPlen;
  Serial.print("Initializing SSD1306 display at "); Serial.print(SCREEN_WIDTH); Serial.print("x"); Serial.println(SCREEN_HEIGHT);
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  oled.setTextColor(WHITE, BLACK);
  oled.clearDisplay();
  oled.display();

  oled.invertDisplay(true);
  delay(500);
  oled.invertDisplay(false);

  oled.setTextSize(1);
  oled.setCursor(17, 0);
  oled.println("Redfield Weather");        //16 characters (96px)
  oled.setCursor(0, 16);
  #ifdef APWIFI
  oled.println("WiFi (No password):");   //19 characters (114px)
  oled.println(APSSID);
  oled.println("");
  oled.println("See web display at:");   //19 characters (114px)
  oled.println(WiFi.softAPIP());          //16 characters max. (96px)
  oled.display();
  #endif
  #ifdef STAWIFI
  oled.println("See web display at:");   //19 characters (114px)
  //oled.setCursor(0, 32);
  oled.println(WiFi.localIP());          //16 characters max. (96px)
  oled.display();
  #endif
}

void UpdateOLED()  {
  static int parameter = 1;
  char dataName[22];
  char dataValue[16];
  char strBuf[10];

  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setCursor(0,0);
  
  if (parameter == 1)  {
    parameter++;
    oled.setCursor(31 ,0);
    strcpy(dataName, "Temperature");      //11 characters (66px)
    oled.println(dataName);
    oled.setCursor(0,16);
    oled.println("Actual Temperature:");
    oled.setTextSize(2);
    oled.setCursor(0,24);
    dtostrf(weather.temperature, 4, 1, dataValue);
    oled.print(dataValue);
    oled.print(char(247));
    oled.println("C");
    oled.setCursor(0,40);
    oled.setTextSize(1);
    oled.println("Feels like:");
    oled.setTextSize(2);
    oled.setCursor(0,48);
    dtostrf(weather.temperature, 4, 1, dataValue);
    oled.print(dataValue);
    oled.print(char(247));
    oled.println("C");
  }
  else if (parameter == 2)  {
    parameter++;
    oled.setCursor(40,0);
    oled.println("Humidity");        //17 characters (102px)
    oled.setCursor(0,16);
    oled.println("Relative Humidity:");
    oled.setTextSize(2);
    oled.setCursor(0,24);
    oled.print(weather.humidity);
    oled.println("%");
    oled.setCursor(0,40);
    oled.setTextSize(1);
    oled.println("Dewpoint:");
    oled.setTextSize(2);
    oled.setCursor(0,48);
    oled.print(weather.dewpoint);
    oled.print(char(247));
    oled.println("C");
  }
  else if (parameter == 3)  {
    parameter++;
    oled.setTextSize(1);
    oled.setCursor(56,0);
    oled.print("Wind");         // 4 characters (16px)
    oled.setCursor(37,8);
    oled.println("Direction");         // 9 characters (54px)
    oled.setTextSize(2);
    oled.setCursor(0,32);
    if ((strcmp(weather.windDir, "N") == 0))  {oled.print("North");}
    else if ((strcmp(weather.windDir, "NE") == 0))  {oled.print("Northeast");}
    else if ((strcmp(weather.windDir, "E") == 0))  {oled.print("East");}
    else if ((strcmp(weather.windDir, "SE") == 0))  {oled.print("Southeast");}
    else if ((strcmp(weather.windDir, "S") == 0))  {oled.print("South");}
    else if ((strcmp(weather.windDir, "SW") == 0))  {oled.print("Southwest");}
    else if ((strcmp(weather.windDir, "W") == 0))  {oled.print("West");}
    else if ((strcmp(weather.windDir, "NW") == 0))  {oled.print("Northwest");}
    else {oled.print("Unknown");}
  }
  else if (parameter == 4)  {
    parameter++;
    oled.setTextSize(1);
    oled.setCursor(52,0);
    oled.println("Wind");         // 4 characters (24px)
    oled.setCursor(49,8);
    oled.println("Speed");          // 5 characters (30px)
    oled.setCursor(0,16);
    oled.println("Average windspeed:");
    oled.setTextSize(2);
    oled.setCursor(0,24);
    oled.print(int(weather.windSpeed[timeHr/10]));
    oled.println("km/h");
    oled.setTextSize(1);
    oled.setCursor(0,40);
    oled.print("Gust: ");
    oled.setTextSize(2);
    oled.setCursor(0,48);
    oled.print(int(weather.windGust));
    oled.print("km/h");
  }
  else if (parameter == 5)  {
    parameter++;
    //oled.setTextSize(1);
    //oled.setCursor(40, 0);
    oled.setTextSize(2);
    oled.setCursor(16, 0);
    oled.println("Rainfall");          // 8 characters (48/96px)
    oled.setTextSize(1);
    oled.setCursor(0, 16);
    oled.println("Last Hour:");
    oled.setCursor(0, 24);
    oled.setTextSize(2);
    oled.println(weather.rainHr[timeHr]);
    oled.setCursor(0,40);
    oled.setTextSize(1);
    oled.println("Last 24 hours:");
    oled.setTextSize(2);
    oled.setCursor(0,48);
    oled.println(weather.rainDaily);
  }
  else if (parameter == 6)  {
    parameter++;
    //oled.setTextSize(1);
    //oled.setCursor(40, 0);
    oled.setTextSize(2);
    oled.setCursor(16, 0);
    oled.println("Pressure");          // 8 characters (48/96px)
    oled.setTextSize(1);
    oled.setCursor(0, 16);
    oled.println("Raw Pressure:");
    oled.setTextSize(2);
    oled.setCursor(0,24);
    oled.println(weather.pressureRaw);
    oled.setTextSize(1);
    oled.setCursor(0, 40);
    oled.println("Sea Level Pressure:");
    oled.setTextSize(2);
    oled.setCursor(0,48);
    oled.println(weather.pressureSeaLvl);
  }
  else if (parameter == 7)  {
    parameter++;
    oled.setTextSize(2);
    oled.setCursor(34, 0);
    oled.println("Light");         // 5 characters (30/60px)
    oled.setTextSize(1);
    oled.setCursor(0, 16);
    oled.println("Brightness:");
    oled.setTextSize(2);
    oled.setCursor(0,24);
    oled.println(weather.lightLvl);
  }
  else if (parameter == 8)  {
    parameter = 1;
    oled.setTextSize(2);
    oled.setCursor(34, 0);
    oled.println("Light");         // 5 characters (30/60px)
    oled.setTextSize(1);
    oled.setCursor(0, 16);
    oled.println("Raw Reading:");
    oled.setCursor(0,24);
    oled.setTextSize(2);
    itoa(weather.uvReading, strBuf, 10);
    strcpy(dataValue, strBuf);
    strcat(dataValue, "/");
    dtostrf(uvIndexV, 1, 2, strBuf);
    strcat(dataValue, strBuf);
    oled.println(dataValue);
    oled.setTextSize(1);
    oled.setCursor(0, 40);
    oled.println("UV Index:");
    oled.setTextSize(2);
    oled.setCursor(0,48);
    oled.println(weather.uvIndex);
    bitClear(flags, 8);
  }
  oled.display();
}
#endif