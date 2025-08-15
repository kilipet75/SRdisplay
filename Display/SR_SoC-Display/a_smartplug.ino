//---------------------------------------------------------------------------
// Shelly Funktionen
//ShellyAZPlug-E4B3233568_DC
//192.168.33.1
//---------------------------------------------------------------------------

//Tasmota-Commands
//http://<ip>/cm?cmnd=Power%20On
//http://<ip>/cm?cmnd=Power%20off


//Shelly-JSON:
//{"ble":{},"cloud":{"connected":true},"mqtt":{"connected":false},"plugs_ui":{},"switch:0":
//{"id":0, "source":"HTTP_in", "output":false, "apower":0.0, "voltage":236.5, "freq":50.1,
//"current":0.000, "aenergy":{"total":1871.055,"by_minute":[0.000,0.000,0.000],
//"minute_ts":1742209020}, "ret_aenergy":{"total":0.000,"by_minute":[0.000,0.000,0.000],
//"minute_ts":1742209020},"temperature":{"tC":18.8, "tF":65.9}},"sys":
//{"mac":"E4B3233568DC","restart_required":false,"time":"11:57","unixtime":1742209056,
//"uptime":315251,"ram_size":255132,"ram_free":90612,"fs_size":1048576,"fs_free":729088,
//"cfg_rev":16,"kvs_rev":0,"schedule_rev":0,"webhook_rev":0,"available_updates":
//{"beta":{"version":"1.5.1-beta2"},"stable":{"version":"1.4.4"}},"reset_reason":1},
//"wifi":{"sta_ip":"192.168.123.79","status":"got ip","ssid":"PetersWLAN","rssi":-78}}

int httpResponseCode;
//StaticJsonDocument<1024> doc; in Main-ino deklariert

#define TYPE_SHELLY 0
#define TYPE_TASMOTA 1



void initPlug() {
  xTaskCreatePinnedToCore(
    plugTask,
    "Smartplug Task",
    4096,
    NULL,
    1,
    NULL,
    1);
}

void plugTask(void* pvParameters) {
  const TickType_t xDelay = pdMS_TO_TICKS(5000);

  getPlugInfo();
  plugEnergyTotalSave = plugEnergyTotal;

  for (;;) {
    getPlugInfo();
    vTaskDelay(xDelay);
  }
}

void StartStopPlug(lv_event_t* e) {
  if (plugOnline) {
    if (plugON) togglePlug(false);
    else togglePlug(true);
  }

  getPlugInfo();
  if (plugON) lv_obj_set_style_bg_color(ui_Button_StartStop, lv_color_hex(0x20B24A), LV_PART_MAIN | LV_STATE_DEFAULT);
  else lv_obj_set_style_bg_color(ui_Button_StartStop, lv_color_hex(0x9F9F9F), LV_PART_MAIN | LV_STATE_DEFAULT);
}

bool togglePlug(bool turnOn) {
  if (WiFi.status() != WL_CONNECTED) return false;

  String Command = "";
  if (primaryWLAN) {
    if (!checkConnection(String(config.plug_ip))) return false;

    if (config.plug_type == TYPE_SHELLY) {
      if (turnOn) Command = "http://" + String(config.plug_ip) + "/relay/0?turn=on";
      else Command = "http://" + String(config.plug_ip) + "/relay/0?turn=off";
    } else if (config.plug_type == TYPE_TASMOTA) {
      if (turnOn) Command = "http://" + String(config.plug_ip) + "/cm?cmnd=Power%20On";
      else Command = "http://" + String(config.plug_ip) + "/cm?cmnd=Power%20Off";
    } else return false;

  } else if (secondaryWLAN) {
    if (!checkConnection(String(config.plug_ip2))) return false;

    if (config.plug_type2 == TYPE_SHELLY) {
      if (turnOn) Command = "http://" + String(config.plug_ip2) + "/relay/0?turn=on";
      else Command = "http://" + String(config.plug_ip2) + "/relay/0?turn=off";
    } else if (config.plug_type2 == TYPE_TASMOTA) {
      if (turnOn) Command = "http://" + String(config.plug_ip2) + "/cm?cmnd=Power%20On";
      else Command = "http://" + String(config.plug_ip2) + "/cm?cmnd=Power%20Off";
    } else return false;

  } else return false;

  Serial.print("Plug schalten: ");
  Serial.println(Command);

  return http_Post_Request(Command);
}

bool checkConnection(String ip) {
  WiFiClient client;
  IPAddress plugIP;
  int port = 80;
  unsigned long connectTimeout = 500;

  plugIP.fromString(ip);

  unsigned long startAttempt = millis();
  bool reachable = false;
  while (millis() - startAttempt < connectTimeout) {
    if (client.connect(plugIP, port)) {
      reachable = true;
      client.stop();
      break;
    }
    delay(10);  // ganz kurz warten
  }
  return reachable;
}


void getPlugInfo() {
  char buf[50];
  char url[100];
  bool noConnection = false;
  int type;

  if (primaryWLAN) {
    if (!checkConnection(String(config.plug_ip))) noConnection = true;
    if (config.plug_type == TYPE_SHELLY) snprintf(url, sizeof(url), "http://%s/rpc/Shelly.GetStatus", config.plug_ip);
    else if (config.plug_type == TYPE_TASMOTA) snprintf(url, sizeof(url), "http://%s/cm?cmnd=status%%200", config.plug_ip);
    else noConnection = true;
    type = config.plug_type;

  } else if (secondaryWLAN) {
    if (!checkConnection(String(config.plug_ip2))) noConnection = true;
    if (config.plug_type2 == TYPE_SHELLY) snprintf(url, sizeof(url), "http://%s/rpc/Shelly.GetStatus", config.plug_ip2);
    else if (config.plug_type2 == TYPE_TASMOTA) snprintf(url, sizeof(url), "http://%s/cm?cmnd=status%%200", config.plug_ip2);
    else noConnection = true;
    type = config.plug_type2;

  } else {
    noConnection = true;
  }

  if (noConnection) {
    lv_label_set_text(ui_Label_Shelly1, "Plug nicht verbunden");
    lv_label_set_text(ui_Label_Shelly2, " ");
    plugOnline = false;
    return;
  }

  String response = http_Post_Request(url);
  
  if (httpResponseCode > 0) {
    DeserializationError error = deserializeJson(doc, response);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      http.end();

      return;
    }
    
    if (type == TYPE_SHELLY) {
      plugPower = float(doc["switch:0"]["apower"]);
      plugVoltage = float(doc["switch:0"]["voltage"]);
      plugCurrent = float(doc["switch:0"]["current"]);
      plugFreq = float(doc["switch:0"]["freq"]);
      plugEnergyTotal = float(doc["switch:0"]["aenergy"]["total"]);
      plugON = bool(doc["switch:0"]["output"]);

    } else if (type == TYPE_TASMOTA) {
      if (int(doc["Status"]["Power"]) == 0)  plugON = false;
      else plugON = true;
      plugPower = float(doc["StatusSNS"]["ENERGY"]["Power"]);
      plugVoltage = float(doc["StatusSNS"]["ENERGY"]["Voltage"]);
      plugCurrent = float(doc["StatusSNS"]["ENERGY"]["Current"]);
      plugFreq = 50.0; //nicht vorhanden
      plugEnergyTotal = float(doc["StatusSNS"]["ENERGY"]["Total"]);      
    }

    plugOnline = true;
    if (plugON) snprintf(buf, 50, "EIN P=%.0f Watt", plugPower);
    else snprintf(buf, 50, "AUS");
    lv_label_set_text(ui_Label_Shelly1, "Smartplug verbunden");
    lv_label_set_text(ui_Label_Shelly2, buf);

  } else {
    Serial.println("Fehler bei der Smartplug-Anfrage: " + httpResponseCode);
    plugOnline = false;
    lv_label_set_text(ui_Label_Shelly1, "Smartplug nicht verbunden");
    lv_label_set_text(ui_Label_Shelly2, " ");
    lv_obj_add_flag(ui_Image_Shelly, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_Image_Shelly_2, LV_OBJ_FLAG_HIDDEN);
  }
}

String http_Post_Request(String url) {
  bool response;
  http.setTimeout(5000);
  http.useHTTP10(true);
  http.begin(url);                // Starte die HTTP-Verbindung
  httpResponseCode = http.GET();  // Sende HTTP GET-Anfrage
  String payload = "";            // Leeres Payload für die Antwort

  if (httpResponseCode > 0) {
    payload = http.getString();  // Erhalte Antwort als String
  } else {
    Serial.println("Fehler bei der http-Anfrage. Fehler:" + String(httpResponseCode));
    Serial.println(url);
  }
  http.end();  // Beende die HTTP-Verbindung
  return payload;
}

bool sendWhatsapp(String Message) {
  if (WiFi.status() != WL_CONNECTED) return false;
  if (strcmp(config.TelNummer, "") == 0) return false;
  if (strcmp(config.apiKey, "") == 0) return false;

  //Message += " SoC:" + data.bms_soc + "%";

  String Command = "";
  String response = "";
  if (primaryWLAN) {
    Command = "http://api.callmebot.com/whatsapp.php?phone=" + String(config.TelNummer) + "&text=" + Message + "&apikey=" + String(config.apiKey);
    Serial.print("Sende Whatsapp-Nachricht: ");
    Serial.println(Command);
    response = http_Post_Request(Command);
    if (httpResponseCode > 0) return true;
    else return false;
  }
  return false;
}
