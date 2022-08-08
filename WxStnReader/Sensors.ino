void readDS18B20() {
  float ds18Temp;
  static float oldTemp;

  sensors.requestTemperatures();
  temperature = sensors.getTempCByIndex(0);
  
  temperature = temperature + ds18CalOffset;
  #ifdef DS18EBUG 
  Serial.println("Reading DS18B20...");
  Serial.print("DS18B20 temperature reading: "); Serial.print(temperature); Serial.println("ºC");
  Serial.print("DS18B20 vs BME280 difference: "); Serial.print(abs(temperature - temperature)); Serial.println("ºC");
  #endif

  if (oldTemp != temperature) {
    #ifdef DS18EBUG 
    Serial.println("Setting temperature message flag");
    #endif
    bitSet(flags, 2);  //Temperature flag
    oldTemp = temperature;
  }
}

void calculateDewpoint() {
  float a = 17.625;    // Magnus coefficient a
  float b = 243.04;    // Magnus coefficient b
  float alpha = (log(humidity / 100)) + ((a * temperature) / (b + temperature));
  dewpoint = (b * alpha) / (a - alpha);
  #ifdef DEWDEBUG 
  Serial.println("Setting dewpoint message flag");
  #endif
  bitSet(flags, 5);  //Dewpoint flag
}

void calculateSealevel() {
  pressureSeaLvl = ((pressureRaw * 10) * (pow((1 - ((0.0065 * altitude) / (temperature + (0.0065 * altitude) + 273.15))), -5.257))) / 10;
  #ifdef BMEDEBUG 
  Serial.println("Setting pressure message flag");
  #endif
  bitSet(flags, 3);  //Pressure flag
}

void readBME() {
  static float oldPress = pressureRaw;
  static int oldHumid = humidity;

  #ifdef BMEDEBUG 
  Serial.println("Start BME reading...");
  #endif

  humidity = bme.readHumidity();                // Throwaway read for humidity
  #ifdef BMEDEBUG 
  Serial.print("Humidity read - Result: "); Serial.print(humidity); Serial.println("%");
  #endif
  /*temperature = bme.readTemperature();       // Temperature (deg C)
  #ifdef BMEDEBUG 
  Serial.println("Temperature read");
  #endif*/
  pressureRaw = bme.readPressure() / 1000;     // Barometric Pressure (Pa converted to kPa);
  #ifdef BMEDEBUG 
  Serial.print("Pressure read - Result: "); Serial.print(pressureRaw); Serial.println("kPa");
  #endif

  if (isnan(pressureRaw))  {
    Serial.println("Bad BME read, using last pressure values");
  }
  if ((pressureRaw <= oldPress - 0.25) || (oldPress + 0.25 <= pressureRaw))  {
    if ((!isnan(temperature)) && (!isnan(pressureRaw))) {
      calculateSealevel();
    }
    //    Serial.println("Setting pressure message flag");
    bitSet(flags, 3);  //Pressure flag
  }

  if (isnan(humidity)) {
    Serial.println("Bad BME read, using last humidity value");
  }
  if ((humidity <= oldHumid - 5) || (oldHumid + 5 <= humidity)) {
    if ((!isnan(temperature)) && (!isnan(humidity))) {
      calculateDewpoint();
    }
    #ifdef BMEDEBUG 
    Serial.println("Setting dewpoint message flag");
    #endif
    bitSet(flags, 4);
  }
}

void measWindDir() {
  long valueAvg = 0;
  int valueMin = 4096;
  int valueMax = 0;
  int valueAnalog = 0;
  char oldDir[4];

  for (int count = 0; count <= 49; count++) {
    valueAnalog = analogRead(pinVane);
    #ifdef WINDDIRDEBUG 
    Serial.print("Instant ADC: "); Serial.println(valueAnalog);
    #endif
    valueAvg = valueAvg + valueAnalog;
    if (valueAnalog < valueMin) {
      valueMin = valueAnalog;
    }
    if (valueAnalog > valueMax) {
      valueMax = valueAnalog;
    }
  }
  valueAvg = valueAvg / 50;
  #ifdef WINDDIRDEBUG 
  Serial.print("Average analog reading: "); Serial.println(valueAvg);
  Serial.print("Max reading: "); Serial.println(valueMax);
  Serial.print("Min reading: "); Serial.println(valueMin);
  Serial.print("Wind direction: ");
  #endif

  if ((918 < valueAvg) && (valueAvg <= 1464))  {
    #ifdef WINDDIRDEBUG 
    Serial.println("North");
    #endif
    strcpy(windDir, "N");
  }
  else if ((2132 < valueAvg) && (valueAvg <= 2732))  {
    #ifdef WINDDIRDEBUG 
    Serial.println("Northeast");
    #endif
    strcpy(windDir, "NE");
  }
  else if (3751 < valueAvg)  {
    #ifdef WINDDIRDEBUG 
    Serial.println("East");
    #endif
    strcpy(windDir, "E");
  }
  else if ((3317 < valueAvg) && (valueAvg <= 3751))  {
    #ifdef WINDDIRDEBUG 
    Serial.println("Southeast");
    #endif
    strcpy(windDir, "SE");
  }
  else if ((2732 < valueAvg) && (valueAvg <= 3317))  {
    #ifdef WINDDIRDEBUG 
    Serial.println("South");
    #endif
    strcpy(windDir, "S");
  }
  else if ((1464 < valueAvg) && (valueAvg <= 2133))  {
    #ifdef WINDDIRDEBUG 
    Serial.println("Southwest");
    #endif
    strcpy(windDir, "SW");
  }
  else if (valueAvg <= 556)  {
    #ifdef WINDDIRDEBUG 
    Serial.println("West");
    #endif
    strcpy(windDir, "W");
  }
  else if ((556 < valueAvg) && (valueAvg <= 918))  {
    #ifdef WINDDIRDEBUG 
    Serial.println("Northwest");
    #endif
    strcpy(windDir, "NW");
  }

  if ((windDir <= oldDir) && (oldDir <= windDir)) {
    #ifdef WINDDIRDEBUG 
    Serial.println("Setting wind message flag");
    #endif
    bitSet(flags, 0);  //Wind flag
  }
}

void AnemoInterrupt() {
  static unsigned long oldAnemoTime;
  unsigned long newTime = millis();
  if (newTime >= oldAnemoTime + reedDebounceTime) {
    anemoCount++;
    oldAnemoTime = newTime;
  }
}

float measWindSpd(int readTime) {  
  int avgInterval = timeMin / 10;
  float windAvg = 0.0;
  static float oldSpeed = windSpeed[0];

  if (readTime == 3)  {
    static int interval = 0;
    float tempSpeed;

    windCount[interval] = anemoCount;
    anemoCount = 0;
    tempSpeed = float(windCount[interval]);
    tempSpeed = (tempSpeed / 3) * 2.4;
    if (tempSpeed > windGust)  {
      windGust = tempSpeed;
    }
    #ifdef WINDDEBUG 
    Serial.print("3-second gust. Sample "); Serial.print(interval+1); Serial.print("/200: "); Serial.print(tempSpeed); Serial.print("km/h"); Serial.print("  Gust speed: "); Serial.print(windGust); Serial.println("km/h");
    #endif
    if (interval < 200)  {
      interval++;
    }
    else  {
      interval = 0;
    }
  }

  if (readTime == 10)  {
    #ifdef WINDSPDDEBUG 
    Serial.print("Rolling 10-minute average. Sample "); Serial.print(avgInterval+1); Serial.println("/10");
    Serial.println("Raw counts:");
    #endif
    for (int count = 0; count <= 199; count++)  {
      windAvg = windAvg + float(windCount[count]);
      #ifdef WINDSPDDEBUG 
      Serial.print("Sample "); Serial.print(count+1); Serial.print(": "); Serial.println(windCount[count]);
      #endif
    }
    #ifdef WINDSPDDEBUG 
    Serial.print("10 minute average count: "); Serial.println(windAvg);
    #endif
    windSpeed[avgInterval] = ((windAvg/600) * 2.4);
    #ifdef WINDSPDDEBUG 
    Serial.println("10-minute rolling average windspeed: ");
    for (int count = 0; count < 5; count++)  {
      Serial.print("Average "); Serial.print(count+1); Serial.print(": "); Serial.println(windSpeed[count]);
    }
    #endif
  }


  if ((windSpeed[0] <= oldSpeed - 1) || (oldSpeed + 1 <= windSpeed[0])) {
    //    Serial.println("Setting wind message flag");
    bitSet(flags, 0);  //Wind flag
  }
  
  oldSpeed = windSpeed[avgInterval];
}

void RainInterrupt() {
  static unsigned long oldRainTime;
  unsigned long newTime = millis();
  if (newTime >= oldRainTime + reedDebounceTime) {
    rainHr[timeHr] = rainHr[timeHr] + 0.2794;
    oldRainTime = newTime;
  }
}

void measUV()  {
  static byte oldUV = 0;
  int uvReading = analogRead(pinUV);
  #ifdef UVDEBUG 
  Serial.print("Raw analog UV reading: "); Serial.println(uvReading);
  #endif
  float uvIndexV = uvReading * (3.3 / 4096);
  #ifdef UVDEBUG 
  Serial.print("Raw analog voltage UV reading: "); Serial.println(uvIndexV);
  #endif
  float uvIndexf = (uvIndexV - 0.99) * (15.0 - 0.0) / (2.8 - 0.99) + 0.0;
  #ifdef UVDEBUG 
  Serial.print("Mapped UV Index: "); Serial.println(uvIndexf);
  #endif

  if (uvIndexf < 1)  {
    uvIndex = 1;
  }
  else if (uvIndexf > 15)  {
    uvIndex = 15;
  }
  else {
    uvIndex = round(uvIndexf);
  }
  #ifdef UVDEBUG 
  Serial.print("Final UV Index: "); Serial.println(uvIndex);
  #endif

  if (uvIndex != oldUV) {
    #ifdef UVDEBUG 
    Serial.println("Setting light message flag for UV");
    #endif
    bitSet(flags, 6);
  }
}

void measLux()  {
  static char oldLvl[7];
  rawLux = analogRead(pinLux);
  #ifdef LUXDEBUG 
  Serial.print("Raw lux ADC reading: "); Serial.print(rawLux);
  #endif
  if (rawLux < luxThreshold)  {
    strcpy(lightLvl, "Dark");
  }
  else if ((luxThreshold < rawLux) && (rawLux < (luxThreshold + luxHysteresis)))  {
    strcpy(lightLvl, "Dim");
  }
  else  {
    strcpy(lightLvl, "Bright");
  }
  #ifdef LUXDEBUG 
  Serial.print(" - "); Serial.println(lightLvl);
  #endif

  if (strcmp(lightLvl, oldLvl) != 0)  {
    #ifdef LUXDEBUG 
    Serial.println("Setting light message flag for intensity");
    #endif
    bitSet(flags, 6);
    strcpy(oldLvl, lightLvl);
  }
}
