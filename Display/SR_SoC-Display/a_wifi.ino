//---------------------------------------------------------------------------
// WLAN verbinden oder AP starten
//---------------------------------------------------------------------------
void initWiFi() {
  xTaskCreatePinnedToCore(
    wifiMonitorTask,  // Funktion
    "WiFi Monitor",   // Name des Tasks
    4096,             // Stack-Größe in Bytes
    NULL,             // Parameter
    1,                // Priorität
    NULL,             // Task Handle
    1                 // Core 1 (0 ist meist für WiFi reserviert)
  );
}


void wifiMonitorTask(void *parameter) {
  const TickType_t xDelay = pdMS_TO_TICKS(10000);
  int Counter = 0;
  setupWiFi(0);

  while (true) {
    //im AP-Modus alle 30s prüfen ob bekannte Netzwerke erreichbar sind
    if (Counter >= 3) {
      if (APmode) checkWifiSta();
      Counter = 0;
    } else CheckWifi();

    Counter++;
    vTaskDelay(xDelay);  // alle 10 Sekunden
  }
}

void setupWiFi(int WLANopt) {
  int WIFIBUFF = 0;
  String ssid_AP;
  String IP;
  char buf[30];

  if (WLANopt == 0 && strcmp(config.ssid, "") != 0) {
    //Verbindung mit primären AP versuchen
    Serial.println("verbindungsversuch mit Primär WLAN! (" + String(config.ssid) + "/" + String(config.password) + ")");
    WiFi.mode(WIFI_STA);
    WiFi.begin(config.ssid, config.password);
    while (WiFi.status() != WL_CONNECTED)  // Start WiFi and connect
    {
      WIFIBUFF++;
      delay(500);
      if (WIFIBUFF > 20) {
        break;
      }
    }
    if (WIFIBUFF < 20) {
      Serial.print("Primär WLAN verbunden: ");
      Serial.println(WiFi.localIP());
      WiFi.setHostname("SRdisplay");
      dnsServer.stop();  // DNS-Server deaktivieren
      IP = WiFi.localIP().toString();
      IP.toCharArray(buf, 30);
      lv_label_set_text(ui_Label_WiFi1, "WLAN 1 Verbunden");
      lv_label_set_text(ui_Label_WiFi2, config.ssid);
      lv_label_set_text(ui_Label_WiFi3, buf);
      lv_obj_clear_flag(ui_Image_WLAN, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(ui_Image_WLAN_2, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_Image_HotSpot, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_Image_HotSpot_2, LV_OBJ_FLAG_HIDDEN);
      primaryWLAN = true;
      secondaryWLAN = false;
      APmode = false;
      getPlugInfo();
      WiFi.setAutoReconnect(1);
      return;
    }
  }
  //Verbindung zu skundär versuchen wenn primär nicht geklappt hat
  WIFIBUFF = 0;
  if ((!primaryWLAN && strcmp(config.ssid2, "") != 0) && WLANopt <= 1) {
    Serial.println("verbindungsversuch mit shelly WLAN! " + String(config.ssid2) + "/" + String(config.password2));
    WiFi.mode(WIFI_STA);
    WiFi.begin(config.ssid2, config.password2);

    while (WiFi.status() != WL_CONNECTED)  // Start WiFi and connect
    {
      WIFIBUFF++;
      delay(500);
      if (WIFIBUFF > 20) {
        break;
      }
    }
    if (WIFIBUFF < 20) {
      Serial.print("shelly WLAN verbunden: ");
      Serial.println(WiFi.localIP());
      WiFi.setHostname("SRdisplay");
      dnsServer.stop();  // DNS-Server deaktivieren
      IP = WiFi.localIP().toString();
      IP.toCharArray(buf, 30);
      lv_label_set_text(ui_Label_WiFi1, "WLAN 2 Verbunden");
      lv_label_set_text(ui_Label_WiFi2, config.ssid2);
      lv_label_set_text(ui_Label_WiFi3, buf);
      lv_obj_add_flag(ui_Image_WLAN, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(ui_Image_WLAN, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(ui_Image_WLAN_2, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_Image_HotSpot, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_Image_HotSpot_2, LV_OBJ_FLAG_HIDDEN);
      primaryWLAN = false;
      secondaryWLAN = true;
      APmode = false;
      WiFi.setAutoReconnect(1);
      getPlugInfo();
      return;
    }
  }


  Serial.println("kein Netzwerk gefunden, Erstelle lokalen AP");
  ssid_AP = "SR HotSpot 192.168.4.1";
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid_AP);
  dnsServer.start(53, "*", WiFi.softAPIP());
  primaryWLAN = false;
  secondaryWLAN = false;
  APmode = true;
  ssid_AP.toCharArray(buf, 30);
  lv_label_set_text(ui_Label_WiFi1, "Keine WLAN-Verbindung");
  lv_label_set_text(ui_Label_WiFi2, buf);
  lv_label_set_text(ui_Label_WiFi3, "--> http://192.168.4.1");
  lv_obj_add_flag(ui_Image_WLAN, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_Image_WLAN_2, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(ui_Image_HotSpot, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(ui_Image_HotSpot_2, LV_OBJ_FLAG_HIDDEN);
}

// Check if connected to access point. Returns true if connected, false if not
bool Wifi_isConned(void) {

  if (!WiFi.isConnected()) return false;
  if (WiFi.status() != WL_CONNECTED) return false;
  //float respTime = Ping.ping(WiFi.gatewayIP(), 1);
  //Serial.println("Ping Gateway:" + String(respTime));
  //if (respTime > 1000.0) return false;

  return true;
}

void CheckWifi() {
  if (secondaryWLAN || primaryWLAN) {
    if (!Wifi_isConned()) {
      checkWifiSta();
      return;
    }

    if (Ping.ping(WiFi.gatewayIP(), 1) > 0) {
      Serial.printf("Ping Antwortzeit auf Gateway: %.2fms\n", Ping.averageTime());
    } else {
      Serial.println("Gateway antwortet nicht");
      checkWifiSta();
      return;
    }
  }
}

//Prüfen ob bekannte Netzwerke in Reichweite sind
void checkWifiSta() {
  int n = WiFi.scanNetworks();
  char buf[32];
  Serial.println("Suche nach Netzwerken...");
  if (n == 0) {
    Serial.println("keine gefunden");
  } else {
    char ssids[n][32];
    for (int i = 0; i < n; ++i) {
      //SSIDs kopieren
      WiFi.SSID(i).toCharArray(buf, 32);
      strcpy(ssids[i], buf);
    }

    //Nach Primär-WLAN suchen
    if (!primaryWLAN) {
      for (int i = 0; i < n; ++i) {
        if (strcmp(ssids[i], config.ssid) == 0) {
          Serial.println("Primär-WLAN gefunden, Verbindung wird hergestellt");
          setupWiFi(0);
          if (Wifi_isConned) return;
        }
      }
    }


    //Nach Sekundär-WLAN suchen
    if (!secondaryWLAN) {
      for (int i = 0; i < n; ++i) {
        if (strcmp(ssids[i], config.ssid2) == 0) {
          Serial.println("Shelly-WLAN gefunden, Verbindung wird hergestellt");
          setupWiFi(1);
          if (Wifi_isConned) return;
        }
      }
    }
  }
  if (!APmode) setupWiFi(2);
}
