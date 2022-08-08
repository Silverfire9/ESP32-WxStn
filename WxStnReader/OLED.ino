#ifdef OLEDDISPLAY
void OLEDInit()  { 
  int IPlen;
  Serial.println("Initializing SSD1306 display");
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  display.setTextColor(WHITE, BLACK);
  display.clearDisplay();
  display.display();

  display.invertDisplay(true);
  delay(1000);
  display.invertDisplay(false);

  display.setTextSize(1);
  display.setCursor(17, 0);
  display.println("Redfield Weather");        //16 characters (96px)
  display.setCursor(0, 16);
  if (digitalRead(pinNetworkSelect) == 0)  {
    display.println("WiFi (No password):");   //19 characters (114px)
    display.println(APSSID);
    display.println("");
    display.println("See web display at:");   //19 characters (114px)
    display.println(WiFi.softAPIP());          //16 characters max. (96px)
    display.display();
  }
  else  {
    display.println("See web display at:");   //19 characters (114px)
    //display.setCursor(0, 32);
    display.println(WiFi.localIP());          //16 characters max. (96px)
    display.display();
  }
}

void UpdateOLED()  {
  static int parameter = 1;
  char dataName[22];
  char dataValue[16];
  char strBuf[10];

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  
  if (parameter == 1)  {
    parameter++;
    display.setCursor(31 ,0);
    strcpy(dataName, "Temperature");      //11 characters (66px)
    display.println(dataName);
    display.setTextSize(2);
    display.setCursor(0,28);
    dtostrf(temperature, 4, 1, dataValue);
    display.print(dataValue);
    display.print(char(247));
    display.println("C");
    //display.println(dataValue);
  }
  else if (parameter == 2)  {
    parameter++;
    display.setCursor(40,0);
    display.println("Humidity");        //17 characters (102px)
    display.setCursor(0,16);
    display.println("Relative Humidity:");
    display.setTextSize(2);
    display.setCursor(0,24);
    display.print(humidity);
    display.println("%");
    display.setCursor(0,40);
    display.setTextSize(1);
    display.println("Dewpoint:");
    display.setTextSize(2);
    display.setCursor(0,48);
    display.print(dewpoint);
    display.print(char(247));
    display.println("C");
  }
  else if (parameter == 3)  {
    parameter++;
    display.setTextSize(1);
    display.setCursor(56,0);
    display.print("Wind");         // 4 characters (16px)
    display.setCursor(37,8);
    display.println("Direction");         // 9 characters (54px)
    display.setTextSize(2);
    display.setCursor(0,32);
    if ((strcmp(windDir, "N") == 0))  {display.print("North");}
    else if ((strcmp(windDir, "NE") == 0))  {display.print("Northeast");}
    else if ((strcmp(windDir, "E") == 0))  {display.print("East");}
    else if ((strcmp(windDir, "SE") == 0))  {display.print("Southeast");}
    else if ((strcmp(windDir, "S") == 0))  {display.print("South");}
    else if ((strcmp(windDir, "SW") == 0))  {display.print("Southwest");}
    else if ((strcmp(windDir, "W") == 0))  {display.print("West");}
    else if ((strcmp(windDir, "NW") == 0))  {display.print("Northwest");}
    else {display.print("Unknown");}
    
    //display.print(windDir);
  }
  else if (parameter == 4)  {
    parameter++;
    display.setTextSize(1);
    display.setCursor(52,0);
    display.println("Wind");         // 4 characters (24px)
    display.setCursor(49,8);
    display.println("Speed");          // 5 characters (30px)
    display.setCursor(0,16);
    display.println("Average windspeed:");
    display.setTextSize(2);
    display.setCursor(0,24);
    display.print(int(windSpeed[timeHr/10]));
    display.println("km/h");
    display.setTextSize(1);
    display.setCursor(0,40);
    display.print("Gust: ");
    display.setTextSize(2);
    display.setCursor(0,48);
    display.print(int(windGust));
    display.print("km/h");
  }
  else if (parameter == 5)  {
    parameter++;
    //display.setTextSize(1);
    //display.setCursor(40, 0);
    display.setTextSize(2);
    display.setCursor(16, 0);
    display.println("Rainfall");          // 8 characters (48/96px)
    display.setTextSize(1);
    display.setCursor(0, 16);
    display.println("Last Hour:");
    display.setCursor(0, 24);
    display.setTextSize(2);
    display.println(rainHr[timeHr]);
    display.setCursor(0,40);
    display.setTextSize(1);
    display.println("Last 24 hours:");
    display.setTextSize(2);
    display.setCursor(0,48);
    display.println(rainDaily);
  }
  else if (parameter == 6)  {
    parameter++;
    //display.setTextSize(1);
    //display.setCursor(40, 0);
    display.setTextSize(2);
    display.setCursor(16, 0);
    display.println("Pressure");          // 8 characters (48/96px)
    display.setTextSize(1);
    display.setCursor(0, 16);
    display.println("Raw Pressure:");
    display.setTextSize(2);
    display.setCursor(0,24);
    display.println(pressureRaw);
    display.setTextSize(1);
    display.setCursor(0, 40);
    display.println("Sea Level Pressure:");
    display.setTextSize(2);
    display.setCursor(0,48);
    display.println(pressureSeaLvl);
  }
  else if (parameter == 7)  {
    parameter = 1;
    //parameter = 1;
    //display.setTextSize(1);
    //display.setCursor(49, 0);
    display.setTextSize(2);
    display.setCursor(34, 0);
    display.println("Light");         // 5 characters (30/60px)
    display.setTextSize(1);
    display.setCursor(0, 16);
    display.println("Brightness:");
    display.setTextSize(2);
    display.setCursor(0,24);
    display.println(lightLvl);
    display.setTextSize(1);
    display.setCursor(0, 40);
    display.println("UV Index:");
    display.setTextSize(2);
    display.setCursor(0,48);
    display.println(uvIndex);
  }
  display.display();
}
#endif
