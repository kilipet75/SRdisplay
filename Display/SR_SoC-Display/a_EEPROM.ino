
#define EEPROM_SIZE 512  // ggf. erhöhen, falls nötig
#define CONFIG_ADDR 0
#define DISTANCE_ADDR sizeof(config)  // z. B. 200 oder was config eben belegt


// Funktion zum Laden der Konfiguration aus EEPROM
void loadConfig() {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(CONFIG_ADDR, config);
  EEPROM.end();
  if (config.init)initValues();
  checkValues();
}

void initValues() {
  String blank = "";
  config.init = false;
  blank.toCharArray(config.ssid, 32);
  blank.toCharArray(config.password, 32);
  blank.toCharArray(config.ssid2, 32);
  blank.toCharArray(config.password2, 32);
  blank.toCharArray(config.plug_ip, 16);
  config.plug_type = 0;
  blank.toCharArray(config.plug_ip2, 16);
  config.plug_type2 = 0;
  blank.toCharArray(config.mqtt_server, 32);
  config.mqtt_port = 1883;
  blank.toCharArray(config.mqtt_node, 32);
  config.soc_threshold = 80;
  blank.toCharArray(config.TelNummer, 32);
  blank.toCharArray(config.apiKey, 10);
  config.dark = false;
  config.tacho = false;
  config.clock = false;
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
  Serial.println("EEPROM-Werte initialisiert");
}

void checkValues() {
  bool change = false;
  String blank = "";
  //Geladene Werte überprüfen und ggfs. Standardwerte setzen
  if (!isAscii(config.ssid[0])) {
    blank.toCharArray(config.ssid, 32);
    change = true;
  }
  if (!isAscii(config.password[0])) {
    blank.toCharArray(config.password, 32);
    change = true;
  }
  if (!isAscii(config.ssid2[0])) {
    blank.toCharArray(config.ssid2, 32);
    change = true;
  }
  if (!isAscii(config.password2[0])) {
    blank.toCharArray(config.password2, 32);
    change = true;
  }
  if (!isAscii(config.plug_ip[0])) {
    blank.toCharArray(config.plug_ip, 16);
    change = true;
  }
  if (!isAscii(config.plug_ip2[0])) {
    blank.toCharArray(config.plug_ip2, 16);
    change = true;
  }
  if (!isAscii(config.mqtt_server[0])) {
    blank.toCharArray(config.mqtt_server, 32);
    change = true;
  }
  if (config.mqtt_port < 0) {
    config.mqtt_port = 1883;
    change = true;
  }
  if (!isAscii(config.mqtt_node[0])) {
    blank.toCharArray(config.mqtt_node, 32);
    change = true;
  }
  if (config.soc_threshold < 0 || config.soc_threshold > 100) {
    config.soc_threshold = 80;
    change = true;
  }
  if (!isAscii(config.TelNummer[0])) {
    blank.toCharArray(config.TelNummer, 32);
    change = true;
  }
  if (!isAscii(config.apiKey[0])) {
    blank.toCharArray(config.apiKey, 10);
    change = true;
  }

  if (!USE_RTC) config.clock = false;

  if (change) {
    saveConfig();
    Serial.println("Ungültige Werte korrigiert");
  }
}

// Funktion zum Speichern der Konfiguration im EEPROM
void saveConfig() {
  Serial.println("Werte in EEPROM speichern");
  Serial.print("config.PanelColorDark=");
  Serial.println(config.PanelColorDark);
  Serial.print("config.PanelColorLight=");
  Serial.println(config.PanelColorLight);

  EEPROM.begin(EEPROM_SIZE);
  EEPROM.put(CONFIG_ADDR, config);
  EEPROM.commit();
  EEPROM.end();
}

void saveDistance(double dist) {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.put(DISTANCE_ADDR, dist);
  EEPROM.commit();
  EEPROM.end();
}

double loadDistance() {
  double dist = 0;
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(DISTANCE_ADDR, dist);
  EEPROM.end();

  if (isnan(dist) || dist < 0 || dist > 1000000) dist = 0;
  return dist;
}
