// Akkuanzeige für Second-Ride Elektroantriebe
// Kilian Peters 2025
// Arduino IDE 2.3.6

// Version der Bibliotheken:
//        ESP32: 3.1.3! wg. Kompatibilität mit SPI-Treibern
//        lvgl: 8.3.11! wg. Kompatibilität mit Squareline
//        ArduinoJSON: 7.3.1 (by Benoit Blanchon)
//        ESP32-OTA: 0.1.6 (by Scott Baker)
//        PubSubClient: 2.8 (by Nick O'Leary)


// Board: 'Waveshare ESP32-S3-Touch-AMOLED-1.43'
//        USB CDC OnBoot: Enabled
//        Partition Scheme: Custom --> https://thelastoutpostworkshop.github.io/microcontroller_devkit/esp32partitionbuilder/
//                          5MB APP, 6.4MB FATFS
//        PSRAM: Enabled
//
// WICHTIG: Es sind zwei verschiedene Boards erhältlich (V1 und V2).
// Sollte das Display nichts anzeigen, in LCD.C "#define AMOLED_143_V2" anpassen.


#include <ESPping.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <NetworkUdp.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WebOTA.h>
#include <Update.h>
#include "lcd.h"
#include "FT3168.h"
#include "ui.h"
#include "FS.h"
#include "SPI.h"
#include <PubSubClient.h>
#include <Wire.h>

#define DISPVERSION "0.6.0"
#define USE_MQTT 1
#define USE_RTC 1
#define USE_PLUG 1

// Webserver etc.
DNSServer dnsServer;
WebServer server(80);
WiFiClient espClient;
WiFiClient espClientMQTT;
PubSubClient client;
HTTPClient http;

DynamicJsonDocument doc(2048);

TaskHandle_t mqttTaskHandle = NULL;

// Standardwerte für Konfiguration
struct Config {
  bool init;  //ist bei leerem EEPROM auf true --> Es müssen standard werte gesetzt werden
  char ssid[32];
  char password[32];
  char ssid2[32];
  char password2[32];
  char mqtt_server[32];
  int mqtt_port = 1883;
  char mqtt_node[32];
  char plug_ip[16];
  uint8_t plug_type;  //Steckertype 0=shelly, 1=Tasmota
  char plug_ip2[16];
  uint8_t plug_type2;  //Steckertype 0=shelly, 1=Tasmota
  int soc_threshold;   // Grenzwert für Abschaltung
  char TelNummer[32];  //Telefonnummer für Whatsapp-Nachrichten
  char apiKey[10];     //API-Key vom Whatsapp-Callmebot
  bool dark;           //Darkmodus ein
  bool tacho;          //Beim start auf Tachoanzeige wechseln
  bool clock;          //Uhrzeit anzeigen
  uint32_t backColorLight;
  uint32_t backColorDark;
  uint32_t foreColorLight;
  uint32_t foreColorDark;
  uint32_t backColorScaleDark;
  uint32_t backColorScaleLight;
  uint32_t foreColorScaleDark;
  uint32_t foreColorScaleLight;
  uint32_t PanelColorDark;
  uint32_t PanelColorLight;
  bool maxTempReached = false;  //Akkutemperatur hat 45°C überschritten (bis Temperatur <40°C)
} config;

struct Data {
  String vcu_hw_version;
  String vcu_sw_version;
  String vehicle_type;
  float battery_voltage;
  double battery_capacity;
  String battery_cell_type;
  float bms_current;
  float bms_soc;
  float bms_max_temp;
  float bms_min_temp;
  float bms_voltage;
  float bms_charge_status;
  float bms_max_cell_diff;
  float ctrl_current;
  float ctrl_voltage;
  float ctrl_throttle;
  float ctrl_temp;
  float ctrl_motor_temp;
  int ocv_state;
  float vcu_soc;
  float vcu_voltage;
  float vcu_soc_voltage;
  float vcu_throttle_voltage;
  String engine_state;
  int out_dcdc_en;
  int out_discharge;
  int out_mcu_limp;
  int out_ctrl_en;
  int out_ctrl_reverse;
  int out_brake;
  int out_mosfet_charge_port;
  int out_boot0;
  int out_throttle;
  float power_limit_soc;
  float power_limit_temp;
  float max_discharge_current;
  float max_recharge_current;
  int voltage_good;
  int voltage_standby;
  int voltage_low;
  double speed;
  double consumption;
  double remainingDistance;
} data;

typedef struct
{
  int nYear;
  int nMonth;
  int nWeek;
  int nDay;
  int nHour;
  int nMin;
  int nSec;
} CTime;


bool shouldSaveConfig = false;
bool primaryWLAN = false;
bool secondaryWLAN = false;
bool APmode = false;

uint16_t cycleCount = 0;

uint8_t MQTTcount = 0;
bool MQTTconnected = false;

bool plugOnline = false;
bool plugON = false;
float plugPower = 0.0;
float plugVoltage = 0.0;
float plugCurrent = 0.0;
float plugFreq = 0.0;
float plugEnergy = 0.0;
float plugEnergyTotal = 0.0;
float plugEnergyTotalSave = 0.0;
bool plugSwitchON = false;
bool plugSwitchOFF = false;

bool ladenAktiv = false;
bool maxTempReached = false;
bool msgSent = false;

long timer1s;

int i = 0;

void setup() {
  Serial.begin(115200);
  Serial0.begin(115200);
  Serial.println("SoC-Anzeiger für Second-Ride Elektroantrieb, by K.Peters 06/2025");

  loadConfig();

  initDisp();

  initDistance();

  initWiFi();

  if (USE_MQTT) initMQTT();

  if (USE_PLUG) initPlug();

  if (USE_RTC) initTime();

  if (MDNS.begin("SRdisplay")) {
    Serial.println("MDNS responder started");
  }

  setupHTTP();

  timer1s = millis();

  Serial.flush();
  Serial0.flush();

  Serial.println("Setup beendet");
}

void loop() {
  webota.handle();
  server.handleClient();
  if (!primaryWLAN && !secondaryWLAN) dnsServer.processNextRequest();
  receiveSerial();

  //TIMER 1 Sekunde
  if (millis() > timer1s + 1000UL) {
    timer1s = millis();
    Serial.print("Free HEAP: ");
    Serial.println(ESP.getFreeHeap());
  }
}
