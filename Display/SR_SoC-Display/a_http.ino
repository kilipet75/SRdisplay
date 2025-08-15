//---------------------------------------------------------------------------
// Webinterface HTML
//---------------------------------------------------------------------------
void setupHTTP() {

  server.on("/", handleRoot);
  server.on("/diag", handleDiag);
  server.on("/config", HTTP_POST, handleSave);
  server.on("/resetColor", handleResetColor);
  server.on("/setkm", handleSetKM);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.on("/reboot", handleReboot);
  server.on("/test_whatsapp", handleTestWhatsapp);
  server.on("/generate_204", handleRedirect);         // Android
  server.on("/fwlink", handleRedirect);               // Windows
  server.on("/hotspot-detect.html", handleRedirect);  // Apple

  server.begin();
}


void handleRedirect() {
  server.sendHeader("Location", "http://192.168.4.1/", true);
  server.send(302, "text/plain", "");
}

String getDiagHtml() {
  String html;
  html.reserve(2000);
  html = F("<html><head><title>Diagnose</title>");
  html += F("<meta http-equiv='refresh' content='5'>");  // Automatischer Reload alle 5 Sekunden
  html += F("</head><body>");
  html += F("<a href='/'>Zur&uuml;ck</a> | <a href='' onclick='location.reload(); return false;'>aktualisieren</a><br><br>");
  html += F("<table border='1' cellspacing='0' cellpadding='5'>");
  html += F("<tr><th>Parameter</th><th>Wert</th></tr>");

  html += F("<tr><td>VCU HW Version</td><td>");
  html += data.vcu_hw_version;
  html += F("</td></tr><tr><td>VCU SW Version</td><td>");
  html += data.vcu_sw_version;
  html += F("</td></tr><tr><td>Display Version</td><td>");
  html += String(DISPVERSION);
  html += F("</td></tr><tr><td>Vehicle Type</td><td>");
  html += data.vehicle_type;
  html += F("</td></tr><tr><td>Battery Voltage</td><td>");
  html += String(data.battery_voltage);
  html += F(" V</td></tr><tr><td>Battery Capacity</td><td>");
  html += String(data.battery_capacity);
  html += F(" Ah</td></tr><tr><td>Battery Cell Type</td><td>");
  html += data.battery_cell_type;
  html += F("</td></tr><tr><td>BMS Current</td><td>");
  html += String(data.bms_current);
  html += F(" A</td></tr><tr><td>BMS SOC</td><td>");
  html += String(data.bms_soc);
  html += F(" %</td></tr><tr><td>BMS Max Temp</td><td>");
  html += String(data.bms_max_temp);
  html += F(" &deg;C</td></tr><tr><td>BMS Min Temp</td><td>");
  html += String(data.bms_min_temp);
  html += F(" &deg;C</td></tr><tr><td>BMS Voltage</td><td>");
  html += String(data.bms_voltage);
  html += F(" V</td></tr><tr><td>BMS Charge Status</td><td>");
  html += String(data.bms_charge_status);
  html += F("</td></tr><tr><td>BMS Max Cell Diff</td><td>");
  html += String(data.bms_max_cell_diff);
  html += F(" V</td></tr><tr><td>Controller Current</td><td>");
  html += String(data.ctrl_current);
  html += F(" A</td></tr><tr><td>Controller Voltage</td><td>");
  html += String(data.ctrl_voltage);
  html += F(" V</td></tr><tr><td>Controller Throttle</td><td>");
  html += String(data.ctrl_throttle);
  html += F("</td></tr><tr><td>Controller Temp</td><td>");
  html += String(data.ctrl_temp);
  html += F(" &deg;C</td></tr><tr><td>Controller Motor Temp</td><td>");
  html += String(data.ctrl_motor_temp);
  html += F(" &deg;C</td></tr><tr><td>OCV State</td><td>");
  html += String(data.ocv_state);
  html += F("</td></tr><tr><td>VCU SOC</td><td>");
  html += String(data.vcu_soc);
  html += F(" %</td></tr><tr><td>VCU Voltage</td><td>");
  html += String(data.vcu_voltage);
  html += F(" V</td></tr><tr><td>VCU SOC Voltage</td><td>");
  html += String(data.vcu_soc_voltage);
  html += F(" V</td></tr><tr><td>VCU Throttle Voltage</td><td>");
  html += String(data.vcu_throttle_voltage);
  html += F(" V</td></tr><tr><td>Engine State</td><td>");
  html += data.engine_state;
  html += F("</td></tr><tr><td>Speed</td><td>");
  html += String(data.speed);
  html += F("</td></tr><tr><td>consumption</td><td>");
  html += String(data.consumption);
  html += F("</td></tr><tr><td>remaining Distance</td><td>");
  html += String(data.remainingDistance);
  html += F("</td></tr><tr><td>Smartplug Spannung</td><td>");
  html += String(plugVoltage);
  html += F("</td></tr><tr><td>Smartplug Strom</td><td>");
  html += String(plugCurrent);
  html += F("</td></tr><tr><td>Smartplug Frequenz</td><td>");
  html += String(plugFreq);
  html += F("</td></tr><tr><td>Smartplug Leistung</td><td>");
  html += String(plugPower);
  html += F("</td></tr><tr><td>Smartplug kWh</td><td>");
  html += String(plugEnergy);
  html += F("</td></tr><tr><td>Smartplug kWh total</td><td>");
  html += String(plugEnergyTotal);

  html += F("</td></tr></table></body></html>");
  return html;
}



String getHTML() {
  String html;
  html.reserve(2000);
  html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'>\n";
  html += F("<style>body{text-align:center;font-family:Arial;}.box{padding:10px;border:1px solid #ccc;border-radius:10px;}</style>\n");
  html += F("</head><body><h1>SR Display</h1>\n");
  html += F("<div class='box'><h2>Akku Ladestand: ");
  html += String(data.bms_soc);
  html += F("%<br>Akku Temperatur\n");
  html += String(int(data.bms_max_temp));
  html += F("&deg;C</h2></div>\n");
  html += F("<form method='POST' action='/config'><br>\n");

  html += F("<h2>WLAN Verbindung 1</h2>\n");
  html += F("<label>SSID: </label><input name='ssid' value='");
  html += String(config.ssid);
  html += F("'><br>\n");
  html += F("<label>Passwort: </label><input name='password' type='password' value='");
  html += String(config.password);
  html += F("'><br>\n");
  html += F("<h2>WLAN Verbindung 2</h2>\n");
  html += F("<label>SSID: </label><input name='ssid2' value='");
  html += String(config.ssid2);
  html += F("'><br>\n");
  html += F("<label>Passwort: </label><input name='password2' type='password' value='");
  html += String(config.password2);
  html += F("'><br>\n");

  if (USE_PLUG) {
    html += F("<h2>Smartplug</h2>");
    html += F("<label>IP im WLAN 1: </label><input name='plug_ip' value='");
    html += String(config.plug_ip);
    html += F("'>");
    html += F("<label><input type='radio' name='plug_type' value='0' ");
    if (config.plug_type == 0) html += F("checked");
    html += F("> Shelly</label>");
    html += F("<label><input type='radio' name='plug_type' value='1' ");
    if (config.plug_type > 0) html += F("checked");
    html += F("> Tasmota</label><br>\n");

    html += F("<label>IP im WLAN 2: </label><input name='plug_ip2' value='");
    html += String(config.plug_ip2);
    html += F("'>");
    html += F("<label><input type='radio' name='plug_type2' value='0' ");
    if (config.plug_type2 == 0) html += F("checked");
    html += F("> Shelly</label>");
    html += F("<label><input type='radio' name='plug_type2' value='1' ");
    if (config.plug_type2 > 0) html += F("checked");
    html += F("> Tasmota</label><br><br>\n");

    if (plugOnline) {
      if (plugON) html += "Smartplug verbunden: EIN, " + String(plugPower) + " Watt";
      else html += "Smartplug verbunden: AUS, " + String(plugPower) + " Watt";
      html += "<br>\n";
    } else {
      html += F("Smartplug-Verbindung nicht m&ouml;glich!<br>\n");
    }
    html += F("<label>Abschaltung bei SoC  >%: </label><input name='soc_threshold' type='number' value='");
    html += String(config.soc_threshold);
    html += F("'><br>\n");
    if (plugOnline) {
      html += F("<a href='/on'>Einschalten</a>   <a href='/off'>Ausschalten</a>\n");
    }
  }

  if (USE_MQTT) {
    html += F("<h2>MQTT</h2>");
    html += F("<label>MQTT Server: </label><input name='mqtt_server' value='");
    html += String(config.mqtt_server);
    html += F("'><br>\n");
    html += F("<label>MQTT Port: </label><input name='mqtt_port' value='");
    html += String(config.mqtt_port);
    html += F("'><br>\n");
    html += F("<label>MQTT Node: </label><input name='mqtt_node' value='");
    html += String(config.mqtt_node);
    html += F("'><br>\n");
    if (MQTTconnected) html += F("Verbindung zu MQTT-Server hergestellt<br>\n");
    else html += F("MQTT-Server nicht verbunden<br>\n");
  }

  html += F("<h1>Displayeinstellungen</h1>\n");
  html += F("<label><input type='radio' name='tacho' value='1' ");
  if (config.tacho) html += F("checked");
  html += F("> Tachoanzeige</label><br>\n");

  html += F("<label><input type='radio' name='tacho' value='0' ");
  if (!config.tacho) html += F("checked");
  html += F("> Batterieanzeige</label><br><br>\n");

  html += F("<label><input type='radio' name='clock' value='1' ");
  if (config.clock) html += F("checked");
  html += F("> Uhr anzeigen (Akku erforderlich)</label><br>\n");

  html += F("<label><input type='radio' name='clock' value='0' ");
  if (!config.clock) html += F("checked");
  html += F("> Uhr ausblenden</label><br><br>\n");

  html += F("<h2>Farbthema 1</h2>\n");
  html += F("<label>Hintergrundfarbe : </label><input type='color' name='back_light' value='#");
  html += String(config.backColorLight | 0x1000000, HEX).substring(1);
  html += F("'><br>\n");

  html += F("<label>Vordergrundfarbe : </label><input type='color' name='fore_light' value='#");
  html += String(config.foreColorLight | 0x1000000, HEX).substring(1);
  html += F("'><br>\n");

  html += F("<label>Skalafarbe Hintergrund: </label><input type='color' name='back_scale_light' value='#");
  html += String(config.backColorScaleLight | 0x1000000, HEX).substring(1);
  html += F("'><br>\n");

  html += F("<label>Skalafarbe Vordergrund: </label><input type='color' name='fore_scale_light' value='#");
  html += String(config.foreColorScaleLight | 0x1000000, HEX).substring(1);
  html += F("'><br>\n");

  html += F("<label>Panelfarbe: </label><input type='color' name='panel_light' value='#");
  html += String(config.PanelColorLight | 0x1000000, HEX).substring(1);
  html += F("'><br>\n");

  html += F("<h2>Farbthema 2</h2>\n");
  html += F("<label>Hintergrundfarbe: </label><input type='color' name='back_dark' value='#");
  html += String(config.backColorDark | 0x1000000, HEX).substring(1);
  html += F("'><br>\n");

  html += F("<label>Vordergrundfarbe: </label><input type='color' name='fore_dark' value='#");
  html += String(config.foreColorDark | 0x1000000, HEX).substring(1);
  html += F("'><br>\n");

  html += F("<label>Skalafarbe Hintergrund: </label><input type='color' name='back_scale_dark' value='#");
  html += String(config.backColorScaleDark | 0x1000000, HEX).substring(1);
  html += F("'><br>\n");

  html += F("<label>Skalafarbe Vordergrund: </label><input type='color' name='fore_scale_dark' value='#");
  html += String(config.foreColorScaleDark | 0x1000000, HEX).substring(1);
  html += F("'><br>\n");

  html += F("<label>Panelfarbe: </label><input type='color' name='panel_dark' value='#");
  html += String(config.PanelColorDark | 0x1000000, HEX).substring(1);
  html += F("'><br>\n");


  html += F("<p><a href='/resetColor'>Farben auf Standard</a></p>\n");


  html += F("<h2>Whatsapp Nachricht bei Abschaltung der Ladung</h2>\n");
  html += F("<label>Telefonnummer (Format '+4917#######'): </label><input name='telefonnummer' value='");
  html += String(config.TelNummer);
  html += F("'><br>\n");
  html += F("<label>Api-Key Whatsapp-CallMe-Bot: </label><input name='apikey' value='");
  html += String(config.apiKey);
  html += F("'><br>\n");
  if (config.TelNummer[0] != '\0' && config.apiKey[0] != '\0') html += F("<a href='/test_whatsapp'><button type='button'>Testnachricht senden</button></a>\n");

  html += F("<br><hr><input type='submit' value='Einstellungen speichern'>\n");
  html += F("</form>\n");

  html += F("<p><a href='/reboot'>ESP neustarten</a></p>\n");
  html += F("<a href='/diag'>Diagnosedaten</a><br><br>");
  html += F("<a href='http://");
  html += String(WiFi.localIP().toString());
  html += F(":8080/webota'>Update</a><br><br>");
  html += F("</body></html>\n");
  //Serial.println(html);

  return html;
}

// Handler für Webinterface
void handleRoot() {
  Serial.println("handleRoot");
  server.send(200, F("text/html"), getHTML());
}

void handleDiag() {
  Serial.println("handleDiag");
  server.send(200, F("text/html"), getDiagHtml());
}

void handleResetColor() {
  Serial.println("handleResetColor");
  config.backColorLight = 0xFFFFFF;  //Weiß
  config.foreColorLight = 0x000000;  //Schwarz
  config.backColorDark = 0x000000;
  config.foreColorDark = 0xFFFFFF;
  config.backColorScaleDark = 0xDECE73;
  config.foreColorScaleDark = 0xB48918;
  config.backColorScaleLight = 0xDECE73;
  config.foreColorScaleLight = 0xB48918;
  config.PanelColorLight = 0xABABAB;
  config.PanelColorDark = 0xABABAB;
  saveConfig();
  lv_set_ThemeColors();
  server.send(200, F("text/html"), getHTML());
}


void handleTestWhatsapp() {
  String msg;
  msg = "Testnachricht+SoC=";
  msg += int(data.bms_soc);
  msg += "%";
  sendWhatsapp(msg);
  server.send(200, F("text/html"), F("<h1>Nachricht gesendet...</h1><a href='/'>Zur&uuml;ck</a>"));
}

// Speichern der Konfigurationswerte aus dem Webinterface
void handleSave() {
  Serial.println(F("Speichern der Konfigurationswerte aus dem Webinterface"));
  bool newWiFi = false;
  bool newColor = false;

  if (server.hasArg("ssid")) {
    if (strcmp(config.ssid, server.arg("ssid").c_str()) != 0) {
      strncpy(config.ssid, server.arg("ssid").c_str(), sizeof(config.ssid));
      newWiFi = true;
    }
  }

  if (server.hasArg("password")) {
    if (strcmp(config.password, server.arg("password").c_str()) != 0) {
      strncpy(config.password, server.arg("password").c_str(), sizeof(config.password));
      newWiFi = true;
    }
  }

  if (server.hasArg("ssid2")) {
    if (strcmp(config.ssid2, server.arg("ssid2").c_str()) != 0) {
      strncpy(config.ssid2, server.arg("ssid2").c_str(), sizeof(config.ssid2));
      newWiFi = true;
    }
  }

  if (server.hasArg("password2")) {
    if (strcmp(config.password2, server.arg("password2").c_str()) != 0) {
      strncpy(config.password2, server.arg("password2").c_str(), sizeof(config.password2));
      newWiFi = true;
    }
  }

  if (server.hasArg("plug_ip")) strncpy(config.plug_ip, server.arg("plug_ip").c_str(), sizeof(config.plug_ip));
  if (server.hasArg("plug_type")) config.plug_type = server.arg("plug_type").toInt();

  if (server.hasArg("plug_ip2")) strncpy(config.plug_ip2, server.arg("plug_ip2").c_str(), sizeof(config.plug_ip2));
  if (server.hasArg("plug_type2")) config.plug_type2 = server.arg("plug_type2").toInt();

  if (server.hasArg("telefonnummer")) strncpy(config.TelNummer, server.arg("telefonnummer").c_str(), sizeof(config.TelNummer));
  if (server.hasArg("apikey")) strncpy(config.apiKey, server.arg("apikey").c_str(), sizeof(config.apiKey));

  if (server.hasArg("soc_threshold")) {
    config.soc_threshold = int(server.arg("soc_threshold").toFloat() / 5.0) * 5;
    lv_roller_set_selected(ui_SoCsetpoint, int(config.soc_threshold / 5.0) - 1, LV_ANIM_OFF);
    msgSent = false;
  }

  if (server.hasArg("mqtt_server")) strncpy(config.mqtt_server, server.arg("mqtt_server").c_str(), sizeof(config.mqtt_server));

  if (server.hasArg("mqtt_port")) config.mqtt_port = server.arg("mqtt_port").toInt();

  if (server.hasArg("mqtt_node")) strncpy(config.mqtt_node, server.arg("mqtt_node").c_str(), sizeof(config.mqtt_node));

  if (server.hasArg("tacho")) {
    config.tacho = server.arg("tacho") == "1";
  }

  if (server.hasArg("clock")) {
    config.clock = server.arg("clock") == "1";
    lv_SetFlagHide(!config.clock, ui_Label_Time);
    lv_SetFlagHide(!config.clock, ui_Label_Time_2);
  }

  if (server.hasArg("back_light")) {
    config.backColorLight = strtoul(server.arg("back_light").c_str() + 1, NULL, 16);  // überspringe das #
    newColor = true;
  }
  if (server.hasArg("back_dark")) {
    config.backColorDark = strtoul(server.arg("back_dark").c_str() + 1, NULL, 16);  // überspringe das #
    newColor = true;
  }
  if (server.hasArg("fore_light")) {
    config.foreColorLight = strtoul(server.arg("fore_light").c_str() + 1, NULL, 16);  // überspringe das #
    newColor = true;
  }
  if (server.hasArg("fore_dark")) {
    config.foreColorDark = strtoul(server.arg("fore_dark").c_str() + 1, NULL, 16);  // überspringe das #
    newColor = true;
  }
  if (server.hasArg("back_scale_dark")) {
    config.backColorScaleDark = strtoul(server.arg("back_scale_dark").c_str() + 1, NULL, 16);  // überspringe das #
    newColor = true;
  }
  if (server.hasArg("fore_scale_dark")) {
    config.foreColorScaleDark = strtoul(server.arg("fore_scale_dark").c_str() + 1, NULL, 16);  // überspringe das #
    newColor = true;
  }
  if (server.hasArg("back_scale_light")) {
    config.backColorScaleLight = strtoul(server.arg("back_scale_light").c_str() + 1, NULL, 16);  // überspringe das #
    newColor = true;
  }
  if (server.hasArg("fore_scale_light")) {
    config.foreColorScaleLight = strtoul(server.arg("fore_scale_light").c_str() + 1, NULL, 16);  // überspringe das #
    newColor = true;
  }
  if (server.hasArg("panel_light")) {
    config.PanelColorLight = strtoul(server.arg("panel_light").c_str() + 1, NULL, 16);  // überspringe das #
    newColor = true;
  }
  if (server.hasArg("panel_dark")) {
    config.PanelColorDark = strtoul(server.arg("panel_dark").c_str() + 1, NULL, 16);  // überspringe das #
    newColor = true;
  }

  saveConfig();

  if (newColor) lv_set_ThemeColors();

  if (newWiFi) {
    server.send(200, F("text/html"), F("<h1>Gespeichert! Wifi-Verbindung wird hergestellt...</h1><a href='/'>Zur&uuml;ck</a>"));
    checkWifiSta();
  } else server.send(200, F("text/html"), F("<h1>Gespeichert!</h1><a href='/'>Zur&uuml;ck</a>"));
}

void handleOn() {
  if (togglePlug(true)) {
    server.send(200, F("text/html"), F("<h1>Smartplug eingeschaltet!</h1><a href='/'>Zur&uuml;ck</a>"));

  } else {
    server.send(200, F("text/html"), F("<h1>Smartplug nicht erreichbar!</h1><a href='/'>Zur&uuml;ck</a>"));
  }
}

void handleOff() {
  if (togglePlug(false)) {
    server.send(200, F("text/html"), F("<h1>Smartplug ausgeschaltet!</h1><a href='/'>Zur&uuml;ck</a>"));

  } else {
    server.send(200, F("text/html"), F("<h1>Smartplug nicht erreichbar!</h1><a href='/'>Zur&uuml;ck</a>"));
  }
}

void handleReboot() {
  Serial.println(F("Neustart von Webinterface"));
  server.send(200, F("text/html"), F("<h1>ESP wird neu gestartet!</h1><a href='/'>Zur&uuml;ck</a>"));
  delay(2000);
  ESP.restart();
}

void handleSetKM() {
  if (!server.hasArg("secret") || server.arg("secret") != "not4all") {
    server.send(403, F("text/html"), F("<h1>Zugriff verweigert</h1><a href='/'>Zur&uuml;ck</a>"));
    return;
  }

  if (!server.hasArg("value")) {
    server.send(400, F("text/html"), F("<h1>Fehlender Parameter: value</h1><a href='/'>Zur&uuml;ck</a>"));

    return;
  }

  double newKM = server.arg("value").toDouble();
  if (isnan(newKM) || newKM < 0 || newKM > 1000000) {
    server.send(400, F("text/html"), F("<h1>Ungueltiger Wert</h1><a href='/'>Zur&uuml;ck</a>"));

    return;
  }

  total_distance_km = newKM;
  last_saved_distance = newKM;
  saveDistance(newKM);

  server.send(200, "text/html", "<h1>Kilometerstand gesetzt auf: " + String(newKM) + "</h1><a href='/'>Zur&uuml;ck</a>");
}