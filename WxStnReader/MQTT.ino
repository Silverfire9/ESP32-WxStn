void pubTelemetry(char* sensor) {
  if (WiFi.status() != WL_CONNECTED)  {
    Serial.print("Disconnected");
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.disconnect();
      WiFi.reconnect();
      delay(1000);
      Serial.print("...");
    }
  }

  #ifdef MQTTDEBUG 
  Serial.println("");
  Serial.println("Publishing data via MQTT");
  #endif
  char teleTopic[25];
  char teleMessage[51];
  char strBuf[11];

  if (sensor == "Time") {                           // Timestamp
    strcpy(teleTopic, mqttDataTopic);                   // 12
    strcat(teleTopic, "/Timestamp");                    // 11 = 23
    strcpy(teleMessage, "Timestamp:");                  // 9
    strcat(teleMessage, weather.rtcTime);                     // 21 = 30
  }

  if (sensor == "Wind") {                           // Wind
    strcpy(teleTopic, mqttDataTopic);                   // 12
    strcat(teleTopic, "/Wind");                         // 6 = 18
    strcpy(teleMessage, "Speed:");                      // 7
    dtostrf(weather.windSpeed[timeHr / 10], 4, 2, strBuf);
    strcat(teleMessage, strBuf);                        // 7
    strcat(teleMessage, ",Direction:");                 // 12
    strcat(teleMessage, weather.windDir);                       // 11
    strcat(teleMessage, ",Gust:");                      // 7
    dtostrf(weather.windGust, 4, 2, strBuf);
    strcat(teleMessage, strBuf);                        // 7 = 51
  }

  if (sensor == "Rain") {                           // Rain
    strcpy(teleTopic, mqttDataTopic);                   // 12
    strcat(teleTopic, "/Rain");                         // 6 = 18
    strcpy(teleMessage, "Hourly:");                     // 8
    dtostrf(weather.rainHr[timeHr], 4, 2, strBuf);
    strcat(teleMessage, strBuf);                        // 6
    strcat(teleMessage, ",24Hrs:");                     // 8
    dtostrf(weather.rainDaily, 4, 2, strBuf);
    strcat(teleMessage, strBuf);                        // 7 = 29
  }

  if (sensor == "Temp") {                           // Temperature
    strcpy(teleTopic, mqttDataTopic);                   // 12
    strcat(teleTopic, "/Temperature");                  // 13 = 25
    strcpy(teleMessage, "Raw:");                        // 4
    dtostrf(weather.temperature, 4, 1, strBuf);
    strcat(teleMessage, strBuf);                        // 5
    strcat(teleMessage, ",Feel:");                      // 6
    dtostrf(weather.feelTemp, 4, 1, strBuf);
    strcat(teleMessage, strBuf);                        // 5 = 20
  }

  if (sensor == "Press") {                          // Pressure
    strcpy(teleTopic,mqttDataTopic);                 // 12
    strcat(teleTopic, "/Pressure");                   // 10 = 22
    strcpy(teleMessage, "Raw:");                      // 5
    dtostrf(weather.pressureRaw, 4, 1, strBuf);
    strcat(teleMessage, strBuf);                      // 6
    strcat(teleMessage, ",Sea_Lvl:");                 // 9
    dtostrf(weather.pressureSeaLvl, 4, 1, strBuf);
    strcat(teleMessage, strBuf);                      // 6 = 26
  }

  if (sensor == "Humid") {                          // Humidity
    strcpy(teleTopic, mqttDataTopic);                 // 12
    strcat(teleTopic, "/Humidity");                   // 11 = 23
    strcpy(teleMessage, "Humidity:");                 // 10
    dtostrf(weather.humidity, 4, 1, strBuf);
    strcat(teleMessage, strBuf);                      // 7
  }

  if (sensor == "DewPt") {                          // Dewpoint
    strcpy(teleTopic, mqttDataTopic);                 // 12
    strcat(teleTopic, "/Dewpoint");                   // 10 = 22
    strcpy(teleMessage, "Dewpoint:");                 // 11
    dtostrf(weather.dewpoint, 3, 1, strBuf);
    strcat(teleMessage, strBuf);                      // 6 = 17
  }

  if (sensor == "Light") {                          // UV Index
    strcpy(teleTopic, mqttDataTopic);                 // 12
    strcat(teleTopic, "/Light");                      // 7 = 19
    strcpy(teleMessage, "Overall:");                  // 8
    itoa(weather.rawLux, strBuf, 10);                         // 5
    strcat(teleMessage, strBuf);
    strcat(teleMessage, ",UV_Index:");                // 11
    dtostrf(weather.uvIndex, 1, 0, strBuf);
    strcat(teleMessage, strBuf);                      // 5 = 32
  }
  
  while (!MQTTclient.publish(teleTopic, teleMessage)) {
    #ifdef MQTTDEBUG
    Serial.print("Not connected to broker Reconnecting...");
    #endif
    while (!MQTTclient.connected()) {
      #ifdef MQTTDEBUG
      Serial.print("...");
      #endif
      if (MQTTclient.connect("WxStnClient")) {
        #ifdef MQTTDEBUG
        Serial.println("Connected");
        #endif
      }
    }
    delay(1000);
  }
  #ifdef MQTTDEBUG 
  Serial.println("Publishing telemetry:");
  Serial.print(teleTopic); Serial.print(", "); Serial.println(teleMessage);
  #endif
}

void cmdReceive()  {
  Serial.print("Received command");
}
