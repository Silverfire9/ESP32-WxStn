void readDS18B20() {
  #define TWOPTCAL
  //#define OFFSETCAL
  float ds18Temp;
  static float oldTemp;
  #ifdef TWOPTCAL
  const float ds18RawRange = calibrations.ds18RawHi - calibrations.ds18RawLo;
  const float ds18RefRange = calibrations.ds18RefHi - calibrations.ds18RefLo;
  #endif

  sensors.requestTemperatures();
  ds18Temp = sensors.getTempCByIndex(0);

  if (ds18Temp < -100)  {
    Serial.println("No DS18B20 sensor connected. Using fake value");
  }
  else  {
    #ifdef OFFSETCAL
    weather.temperature = ds18Temp + calibrations.ds18CalOffset;
    #elif defined TWOPTCAL
    weather.temperature = (((ds18Temp - calibrations.ds18RawLo) * ds18RefRange) / ds18RawRange) + calibrations.ds18RefLo;
    #else
    weather.temperature = ds18Temp;
    #endif
    #ifdef DS18EBUG 
    Serial.println("Reading DS18B20...");
    Serial.print("DS18B20 temperature reading: "); Serial.print(weather.temperature); Serial.println("ºC");
    Serial.print("DS18B20 vs BME280 difference: "); Serial.print(abs(weather.temperature - weather.temperature)); Serial.println("ºC");
    #endif
  
    if (oldTemp != weather.temperature) {
      #ifdef DS18EBUG 
      Serial.println("Setting temperature message flag");
      #endif
      bitSet(flags, 2);  //Temperature flag
      oldTemp = weather.temperature;
    }
    calculateFeelTemp();
  }
}

void calculateFeelTemp() {
  #ifdef FEELDEBUG
  Serial.print("Raw: ");
  Serial.println(weather.temperature);
  #endif
  if ((weather.temperature > 15) && (weather.dewpoint > 10))  {                                                  // Humidex
    weather.feelTemp = weather.temperature+(0.5555*(6.11*(pow(2.71828, 5417.7530*((1/273.16-(1/(273.15+weather.dewpoint))))))-10));
    #ifdef FEELDEBUG
    Serial.print("Humidex: ");
    #endif
  }
  else if ((weather.temperature <= 10) && (weather.windSpeed[timeMin/10] > 4.8)) {          // Windchill factor
    float v = pow(weather.windSpeed[timeMin/10], 0.16);
    weather.feelTemp = 13.12+(0.6125*weather.temperature)-(11.37*v)+(0.3965*weather.temperature*v);
    #ifdef FEELDEBUG
    Serial.print("Windchill: ");
    #endif
  }
  else {                                                                    // No correction
    weather.feelTemp = weather.temperature;
    #ifdef FEELDEBUG
    Serial.print("Uncorrected: ");
    #endif
  }
  #ifdef FEELDEBUG
  Serial.println(weather.feelTemp);
  #endif
}

void calculateDewpoint() {
  float a = 17.625;    // Magnus coefficient a
  float b = 243.04;    // Magnus coefficient b
  float alpha = (log(weather.humidity / 100)) + ((a * weather.temperature) / (b + weather.temperature));
  weather.dewpoint = (b * alpha) / (a - alpha);
  #ifdef DEWDEBUG 
  Serial.println("Setting dewpoint message flag");
  #endif
  bitSet(flags, 5);  //Dewpoint flag
}

void calculateSealevel() {
  weather.pressureSeaLvl = ((weather.pressureRaw * 10) * (pow((1 - ((0.0065 * weather.altitude) / (weather.temperature + (0.0065 * weather.altitude) + 273.15))), -5.257))) / 10;
  #ifdef BMEDEBUG 
  Serial.println("Setting pressure message flag");
  #endif
  bitSet(flags, 3);  //Pressure flag
}

void readBME() {
  static float oldPress = weather.pressureRaw;
  static int oldHumid = weather.humidity;

  #ifdef BMEDEBUG 
  Serial.println("Start BME reading...");
  #endif

  weather.humidity = bme.readHumidity();                // Throwaway read for humidity
  weather.pressureRaw = bme.readPressure();             // Throwaway read for pressure
  #ifdef BMEDEBUG 
  Serial.print("Humidity read - Result: "); Serial.print(weather.humidity); Serial.println("%");
  #endif
  /*weather.temperature = bme.readTemperature();       // Temperature (deg C)
  #ifdef BMEDEBUG 
  Serial.println("Temperature read");
  #endif*/
  weather.pressureRaw = bme.readPressure() / 1000;     // Barometric Pressure (Pa converted to kPa);
  #ifdef BMEDEBUG 
  Serial.print("Pressure read - Result: "); Serial.print(weather.pressureRaw); Serial.println("kPa");
  #endif

  if (isnan(weather.pressureRaw))  {
    Serial.println("Bad BME read, using last pressure values");
  }
  if ((weather.pressureRaw <= oldPress - 0.25) || (oldPress + 0.25 <= weather.pressureRaw))  {
    if ((!isnan(weather.temperature)) && (!isnan(weather.pressureRaw))) {
      calculateSealevel();
    }
    //    Serial.println("Setting pressure message flag");
    bitSet(flags, 3);  //Pressure flag
  }

  if (isnan(weather.humidity)) {
    Serial.println("Bad BME read, using last humidity value");
  }
  if ((weather.humidity <= oldHumid - 5) || (oldHumid + 5 <= weather.humidity)) {
    if ((!isnan(weather.temperature)) && (!isnan(weather.humidity))) {
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
    strcpy(weather.windDir, "N");
  }
  else if ((2132 < valueAvg) && (valueAvg <= 2732))  {
    #ifdef WINDDIRDEBUG 
    Serial.println("Northeast");
    #endif
    strcpy(weather.windDir, "NE");
  }
  else if (3751 < valueAvg)  {
    #ifdef WINDDIRDEBUG 
    Serial.println("East");
    #endif
    strcpy(weather.windDir, "E");
  }
  else if ((3317 < valueAvg) && (valueAvg <= 3751))  {
    #ifdef WINDDIRDEBUG 
    Serial.println("Southeast");
    #endif
    strcpy(weather.windDir, "SE");
  }
  else if ((2732 < valueAvg) && (valueAvg <= 3317))  {
    #ifdef WINDDIRDEBUG 
    Serial.println("South");
    #endif
    strcpy(weather.windDir, "S");
  }
  else if ((1464 < valueAvg) && (valueAvg <= 2133))  {
    #ifdef WINDDIRDEBUG 
    Serial.println("Southwest");
    #endif
    strcpy(weather.windDir, "SW");
  }
  else if (valueAvg <= 556)  {
    #ifdef WINDDIRDEBUG 
    Serial.println("West");
    #endif
    strcpy(weather.windDir, "W");
  }
  else if ((556 < valueAvg) && (valueAvg <= 918))  {
    #ifdef WINDDIRDEBUG 
    Serial.println("Northwest");
    #endif
    strcpy(weather.windDir, "NW");
  }

  if ((weather.windDir <= oldDir) && (oldDir <= weather.windDir)) {
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

void measWindSpd(int readTime) {  
  int avgInterval = timeMin / 10;;
  //static int interval = 0;
  float tempSpeed;
  float windAvg = 0.0;
  while (avgInterval>10)  {
    avgInterval = avgInterval / 10;
  }

  #ifdef WINDSPDDEBUG
  Serial.print("timeMin: "); Serial.println(timeMin);
  Serial.print("avgInterval: "); Serial.println(avgInterval);
  #endif
  if (readTime == 3)  {
    #ifdef WINDSPDDEBUG
    Serial.println("3-second reading");
    #endif

    weather.windCount[weather.interval] = anemoCount;
    anemoCount = 0;
    tempSpeed = float(weather.windCount[weather.interval]);
    tempSpeed = (tempSpeed / 3) * 2.4;
    if (tempSpeed > weather.windGust)  {
      weather.windGust = tempSpeed;
    }
    #ifdef WINDSPDDEBUG 
    Serial.print("3-second gust. Sample "); Serial.print(weather.interval+1); Serial.print("/200: "); Serial.print(tempSpeed); Serial.print("km/h"); Serial.print("  Gust speed: "); Serial.print(weather.windGust); Serial.println("km/h");
    Serial.print("Interval count:"); Serial.println(weather.interval);
    #endif
    if (weather.interval < 199)  {
      weather.interval++;
    }
    else  {
      weather.interval = 0;
    }
  }
  else if (readTime == 10)  {
    #ifdef WINDSPDDEBUG 
    Serial.print("Rolling 10-minute average. Sample "); Serial.print(avgInterval+1); Serial.println("/10");
    Serial.println("Raw counts:");
    #endif
    for (int count = 0; count <= 199; count++)  {
      windAvg = windAvg + float(weather.windCount[count]);
      #ifdef WINDSPDDEBUG 
      Serial.print("Sample "); Serial.print(count+1); Serial.print(": "); Serial.println(weather.windCount[count]);
      #endif
    }
    #ifdef WINDSPDDEBUG 
    Serial.print("10 minute average count: "); Serial.println(windAvg);
    #endif
    weather.windSpeed[avgInterval] = ((windAvg/600) * 2.4);
    #ifdef WINDSPDDEBUG 
    Serial.println("10-minute rolling average weather.windspeed: ");
    for (int count = 0; count < 5; count++)  {
      Serial.print("Average "); Serial.print(count+1); Serial.print(": "); Serial.println(weather.windSpeed[count]);
    }
    #endif
  }
  #ifdef WINDSPDDEBUG
  Serial.print("Windspeed: "); Serial.println(weather.windSpeed[avgInterval]);
  Serial.print("Old Speed: "); Serial.println(weather.oldSpeed);
  #endif
  if (int(weather.windSpeed[avgInterval]) != int(weather.oldSpeed) - 1) {
    #ifdef MQTTDEBUG
    Serial.println("Setting wind message flag");
    #endif
    bitSet(flags, 0);  //Wind flag
    weather.oldSpeed = weather.windSpeed[avgInterval]; //fails here
  }
  else {
    #ifdef WINDSPDDEBUG
    Serial.print("'Old' weather.oldSpeed:"); Serial.println(weather.oldSpeed);
    #endif
    weather.oldSpeed = weather.oldSpeed+random(-5, 5);
    #ifdef WINDSPDDEBUG
    Serial.print("'New' weather.oldSpeed:"); Serial.println(weather.oldSpeed);
    #endif
  }
}

void RainInterrupt() {
  static unsigned long oldRainTime;
  unsigned long newTime = millis();
  if (newTime >= oldRainTime + reedDebounceTime) {
    weather.rainHr[timeHr] = weather.rainHr[timeHr] + 0.2794;
    oldRainTime = newTime;
  }
}

void measUV()  {
  static byte oldUV = 0;
  weather.uvReading = analogRead(pinUV);
  #ifdef UVDEBUG 
  Serial.print("Raw analog UV reading: "); Serial.println(weather.uvReading);
  #endif
  uvIndexV = weather.uvReading * (3.3 / 4096);
  #ifdef UVDEBUG 
  Serial.print("Raw analog voltage UV reading: "); Serial.println(uvIndexV);
  #endif
  float uvIndexf = (uvIndexV - 0.99) * (15.0 - 0.0) / (2.8 - 0.99) + 0.0;
  #ifdef UVDEBUG 
  Serial.print("Mapped UV Index: "); Serial.println(uvIndexf);
  #endif

  if (uvIndexf < 1)  {
    weather.uvIndex = 1;
  }
  else if (uvIndexf > 15)  {
    weather.uvIndex = 15;
  }
  else {
    weather.uvIndex = round(uvIndexf);
  }
  #ifdef UVDEBUG 
  Serial.print("Final UV Index: "); Serial.println(weather.uvIndex);
  #endif

  if (weather.uvIndex != oldUV) {
    #ifdef UVDEBUG 
    Serial.println("Setting light message flag for UV");
    #endif
    bitSet(flags, 6);
  }
}

void measLux()  {
  static char oldLvl[7];
  weather.rawLux = analogRead(pinLux);
  #ifdef LUXDEBUG 
  Serial.print("Raw lux ADC reading: "); Serial.print(weather.rawLux);
  #endif
  if (weather.rawLux < calibrations.luxThreshold)  {
    strcpy(weather.lightLvl, "Dark");
  }
  else if ((calibrations.luxThreshold < weather.rawLux) && (weather.rawLux < (calibrations.luxThreshold + calibrations.luxHysteresis)))  {
    strcpy(weather.lightLvl, "Dim");
  }
  else  {
    strcpy(weather.lightLvl, "Bright");
  }
  #ifdef LUXDEBUG 
  Serial.print(" - "); Serial.println(weather.lightLvl);
  #endif

  if (strcmp(weather.lightLvl, oldLvl) != 0)  {
    #ifdef LUXDEBUG 
    Serial.println("Setting light message flag for intensity");
    #endif
    bitSet(flags, 6);
    strcpy(oldLvl, weather.lightLvl);
  }
}
