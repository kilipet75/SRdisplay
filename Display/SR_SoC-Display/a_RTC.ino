
#include <time.h>
#include <ESP32Time.h>

#define PCF85063_ADDRESS 0x51

ESP32Time rtc(0);  // offset in seconds GMT+1

String Uhrzeit;

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;   // MEZ: +1h = 3600s
const int daylightOffset_sec = 0;  // Sommerzeit: +1h

static char *tag = "pcf85063";

void initTime() {
  setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/3", 1);  // MEZ + MESZ automatisch
  tzset();

  initNTP();
  initRTC();
  delay(100);
  setTimeToRTC();

  xTaskCreatePinnedToCore(
    timeTask,
    "Time Task",
    4096,
    NULL,
    1,
    NULL,
    1);
}

void timeTask(void *pvParameters) {
  const TickType_t xDelay = pdMS_TO_TICKS(60000);
  for (;;) {
    setTimeToRTC();
    Uhrzeit = getTime();
    vTaskDelay(xDelay);
  }
}

void initNTP() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

String getTime() {
  CTime tm;
  PCF85063_getTime(&tm);

  // RTC-Zeit in struct tm überführen (UTC)
  struct tm t;
  t.tm_year = tm.nYear - 1900;
  t.tm_mon = tm.nMonth - 1;
  t.tm_mday = tm.nDay;
  t.tm_hour = tm.nHour;
  t.tm_min = tm.nMin;
  t.tm_sec = tm.nSec;
  t.tm_isdst = -1;  // Automatische Sommerzeit-Erkennung

  time_t utc_time = makeTimeFromUTC(&t);
  struct tm *local = localtime(&utc_time);

  // Debug-Ausgabe
  Serial.printf("RTC-Zeit (UTC):   %04d-%02d-%02d %02d:%02d:%02d\n",
                tm.nYear, tm.nMonth, tm.nDay, tm.nHour, tm.nMin, tm.nSec);
  Serial.printf("Lokale Zeit:      %04d-%02d-%02d %02d:%02d:%02d\n",
                local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
                local->tm_hour, local->tm_min, local->tm_sec);

  return formatTime(local->tm_hour, local->tm_min);
}

time_t makeTimeFromUTC(struct tm *t) {
  // Temporär TZ auf UTC setzen
  setenv("TZ", "UTC", 1);
  tzset();

  time_t utc = mktime(t);

  // Jetzt wieder lokale Zeitzone setzen
  setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/3", 1);
  tzset();

  return utc;
}

String formatTime(int h, int m) {
  String Zeit = "";

  if (h < 10) Zeit += "0";
  Zeit += String(h);
  Zeit += ":";
  if (m < 10) Zeit += "0";
  Zeit += String(m);
  return Zeit;
}

void setTimeToRTC() {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    time_t now;
    time(&now);                        // UTC Zeit in Sekunden seit 1970
    struct tm *utc_tm = gmtime(&now);  // wandelt time_t in UTC struct tm

    CTime timeSet;
    timeSet.nYear = utc_tm->tm_year + 1900;
    timeSet.nMonth = utc_tm->tm_mon + 1;
    timeSet.nDay = utc_tm->tm_mday;
    timeSet.nHour = utc_tm->tm_hour;
    timeSet.nMin = utc_tm->tm_min;
    timeSet.nSec = utc_tm->tm_sec;

    Serial.println("RTC wird mit UTC-Zeit synchronisiert...");
    PCF85063_setTime(timeSet, 1);
  }

  //CTime timeSet;
  //timeSet.nYear = rtc.getYear();
  //timeSet.nMonth = rtc.getMonth();
  //timeSet.nDay = rtc.getDay();
  //timeSet.nHour = rtc.getHour();
  //timeSet.nMin = rtc.getMinute();
  //timeSet.nSec = rtc.getSecond();
  //Serial.println("NTP-Zeit wird synchronisiert...");
  //PCF85063_setTime(timeSet, 1);
}

void initRTC() {
  uint8_t addr = 0x04;  //85063 fixed is this one
  uint8_t buf[20];
  if (PCF85063_register_read(addr, buf, 7) != ESP_OK) {
    Serial.println("PCF85063_read failed");
  }
}


/********************************************/
esp_err_t PCF85063_register_read(uint8_t reg, uint8_t *buf, uint8_t len) {
  return I2C_read(0x51, reg, buf, len);  //Read information
}

/********************************************/
/*Writes to the clock sensor*/
esp_err_t PCF85063_register_write(uint8_t reg_addr, uint8_t *data, int len) {
  uint8_t ret;
  ret = I2C_write(0x51, reg_addr, data, len);
  return ret;
}

/********************************************/
int PCF85063_get_Week(int y, int m, int d) {
  int week = 0;
  if (m == 1 || m == 2) {
    m += 12;
    y--;
  }
  week = (d + 2 * m + 3 * (m + 1) / 5 + y + y / 4 - y / 100 + y / 400) % 7;
  //ret=0   Sunday
  //ret=N
  return (week + 1) % 7;
}

/********************************************/
//Get year month day hour minute second
bool PCF85063_getTime(CTime *tm) {
  uint8_t data[7] = { 0 };
  uint8_t inbuf = 0x04;
  if (PCF85063_register_read(inbuf, data, 7) != ESP_OK) {
    ESP_LOGW(tag, "PCF85063_read failed\n");
  }
  bool flag_19xx = (data[5] >> 7) & 0x01;  // Year:19XX_Flag
  bool flag_vl = (data[0] >> 7) & 0x01;    //(Voltage Low)VL=1:Initial data is unreliable

  tm->nSec = PCD85063TP_bcdToDec(data[0] & 0x7f);
  tm->nMin = PCD85063TP_bcdToDec(data[1] & 0x7f);
  tm->nHour = PCD85063TP_bcdToDec(data[2] & 0x3f);
  tm->nDay = PCD85063TP_bcdToDec(data[3] & 0x3f);
  tm->nWeek = PCD85063TP_bcdToDec(data[4]);
  tm->nMonth = PCD85063TP_bcdToDec(data[5] & 0x1f);
  tm->nYear = PCD85063TP_bcdToDec(data[6]);  //0~99
  if (flag_19xx) {
    tm->nYear += 1900;
  } else {
    tm->nYear += 2000;
  }
  return flag_vl;
}

/********************************************/
//Set the time structure to automatically calculate the day of the week
void PCF85063_setTime(CTime tm, int isAutoCalcWeek) {
  uint8_t data[7] = { 0 };
  if (isAutoCalcWeek) {
    tm.nWeek = PCF85063_get_Week(tm.nYear, tm.nMonth, tm.nDay);
  }
  bool flag_19xx = true;
  uint16_t yr = tm.nYear;
  if (tm.nYear >= 2000) {
    flag_19xx = false;
    yr -= 2000;
  } else {
    yr -= 1900;
  }
  data[0] = PCD85063TP_decToBcd(tm.nSec);
  data[1] = PCD85063TP_decToBcd(tm.nMin);
  data[2] = PCD85063TP_decToBcd(tm.nHour);
  data[3] = PCD85063TP_decToBcd(tm.nDay);
  data[4] = PCD85063TP_decToBcd(tm.nWeek);
  data[5] = PCD85063TP_decToBcd(tm.nMonth);
  data[6] = PCD85063TP_decToBcd(yr);
  if (flag_19xx) {
    data[5] |= 0x80;
  }
  if ((PCF85063_register_write(0x04, data, 7) != ESP_OK)) {
    Serial.println("PCF85063_Sending failed");
  }
}

/********************************************/
//12=>0x12
uint8_t PCD85063TP_decToBcd(uint8_t val) {
  return ((val / 10 * 16) + (val % 10));
}


/********************************************/
//0x12=>12
uint8_t PCD85063TP_bcdToDec(uint8_t val) {
  return ((val / 16 * 10) + (val % 16));
}

/********************************************/
uint8_t I2C_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len) {
  uint8_t ret;
  ret = i2c_master_write_read_device(I2C_NUM_0, addr, &reg, 1, buf, len, 1000);
  return ret;
}

/********************************************/
uint8_t I2C_write(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len) {
  uint8_t ret;
  uint8_t *pbuf = (uint8_t *)malloc(len + 1);
  pbuf[0] = reg;
  for (uint8_t i = 0; i < len; i++) {
    pbuf[i + 1] = buf[i];
  }
  ret = i2c_master_write_to_device(I2C_NUM_0, addr, pbuf, len + 1, 1000);
  free(pbuf);
  pbuf = NULL;
  return ret;
}