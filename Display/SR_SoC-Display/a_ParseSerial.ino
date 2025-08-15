#include <ArduinoJson.h>

String SerialString = "";  // Hier wird der empfangene String gespeichert

long timestampMessage;

bool SerialTimeout = false;

void receiveSerial() {
  // Prüfen, ob Daten auf Serial0 vorhanden sind
  while (Serial0.available()) {
    char c = Serial0.read();

    if (c == '\n') {  // Zeile ist vollständig
      SerialString.trim();
      SerialString.toLowerCase();
      Serial.println(SerialString);
      parseLine(SerialString);
      SerialString = "";  // Zurücksetzen für den nächsten String
    } else {
      SerialString += c;
    }
  }
  // Prüfen, ob Daten auf USB-Serial vorhanden sind
  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n') {  // Zeile ist vollständig
      SerialString.trim();
      SerialString.toLowerCase();
      Serial.println(SerialString);
      parseLine(SerialString);
      SerialString = "";  // Zurücksetzen für den nächsten String
    } else {
      SerialString += c;
    }
  }

  if (millis() - timestampMessage > 5000UL) SerialTimeout = true;
  else SerialTimeout = false;
}

void checkSoC() {
  String msg = "";
  if (int(data.bms_soc) >= config.soc_threshold) {  //Ladegrenze erreicht
    if (plugOnline && plugON) {
      togglePlug(false);  // Smartplug abschalten, wenn SoC über Grenzwert
      plugON = false;
      msg = "Ladevorgang+bei+";
      msg += int(data.bms_soc);
      msg += "%+beendet";
    } else {
      msg = "Eingestellte+Ladegrenze+erreicht.+SoC=";
      msg += int(data.bms_soc);
      msg += "%";
    }
    if (primaryWLAN) {
      if (!msgSent) {
        sendWhatsapp(msg);
        msgSent = true;
      }
    }
  }
}


void parseLine(String line) {
  String FindString = "";
  timestampMessage = millis();

  FindString = "VCU HW Version: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.vcu_hw_version = line.substring(FindString.length());
    return;
  }
  FindString = "VCU SW Version: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.vcu_sw_version = line.substring(FindString.length());
    return;
  }
  FindString = "Vehicle type: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.vehicle_type = line.substring(FindString.length());
    return;
  }
  FindString = "Battery design voltage: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.battery_voltage = line.substring(FindString.length()).toFloat();
    return;
  }
  FindString = "Battery design capacity: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.battery_capacity = line.substring(FindString.length()).toDouble();
    return;
  }
  FindString = "Battery cell type: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.battery_cell_type = line.substring(FindString.length());
    return;
  }
  FindString = "BMS Current: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.bms_current = line.substring(FindString.length()).toFloat();
    return;
  }
  FindString = "BMS SOC: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.bms_soc = line.substring(FindString.length()).toFloat() / 10.0;
    checkSoC();
    return;
  }
  FindString = "BMS Max Temp: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.bms_max_temp = line.substring(FindString.length()).toFloat();

    //Wenn die Akkutemperatur 45°C übersteigt kann nicht geladen werden bis die Temperatur wieder auf unter 40°C fällt
    if (data.bms_max_temp >= 45.0 && !config.maxTempReached) {
      config.maxTempReached = true;
      saveConfig();
    } else if (data.bms_max_temp < 40.0 && config.maxTempReached) {
      config.maxTempReached = false;
      saveConfig();
    }
    return;
  }
  FindString = "BMS Min Temp: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.bms_min_temp = line.substring(FindString.length()).toFloat();
    return;
  }
  FindString = "BMS Voltage: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.bms_voltage = line.substring(FindString.length()).toFloat();
    return;
  }
  FindString = "BMS charge status: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.bms_charge_status = line.substring(FindString.length()).toFloat();
    return;
  }
  FindString = "BMS max cell diff: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.bms_max_cell_diff = line.substring(FindString.length()).toFloat();
    return;
  }
  FindString = "CTRL current: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.ctrl_current = line.substring(FindString.length()).toFloat();
    return;
  }
  FindString = "CTRL voltage: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.ctrl_voltage = line.substring(FindString.length()).toFloat();
    return;
  }
  FindString = "CTRL throttle: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.ctrl_throttle = line.substring(FindString.length()).toFloat();
    return;
  }
  FindString = "CTRL temp: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.ctrl_temp = line.substring(FindString.length()).toFloat();
    return;
  }
  FindString = "CTRL motor temp: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.ctrl_motor_temp = line.substring(FindString.length()).toFloat();
    return;
  }
  FindString = "OCV_state: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.ocv_state = line.substring(FindString.length()).toInt();
    return;
  }
  FindString = "VCU SOC: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.vcu_soc = line.substring(FindString.length()).toFloat();
    return;
  }
  FindString = "VCU Voltage: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.vcu_voltage = line.substring(FindString.length()).toFloat();
    return;
  }
  FindString = "VCU SOC Voltage: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.vcu_soc_voltage = line.substring(FindString.length()).toFloat();
    return;
  }
  FindString = "VCU Throttle Voltage: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.vcu_throttle_voltage = line.substring(FindString.length()).toFloat();
    return;
  }
  FindString = "Engine State: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.engine_state = line.substring(FindString.length());

    if (data.engine_state.indexOf("engine_state_charging_") >= 0) ladenAktiv = true;
    else ladenAktiv = false;

    return;
  }
  FindString = "outDCDC_EN: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.out_dcdc_en = line.substring(FindString.length()).toInt();
    return;
  }
  FindString = "outDischarge: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.out_discharge = line.substring(FindString.length()).toInt();
    return;
  }
  FindString = "outMCULimp: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.out_mcu_limp = line.substring(FindString.length()).toInt();
    return;
  }
  FindString = "outCTRL_EN: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.out_ctrl_en = line.substring(FindString.length()).toInt();
    return;
  }
  FindString = "outCTRL_Reverse: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.out_ctrl_reverse = line.substring(FindString.length()).toInt();
    return;
  }
  FindString = "outBrake: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.out_brake = line.substring(FindString.length()).toInt();
    return;
  }
  FindString = "outMosfetChargePort: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.out_mosfet_charge_port = line.substring(FindString.length()).toInt();
    return;
  }
  FindString = "outBoot0: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.out_boot0 = line.substring(FindString.length()).toInt();
    return;
  }
  FindString = "outThrottle: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.out_throttle = line.substring(FindString.length()).toInt();
    return;
  }
  FindString = "powerLimitSOC: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.power_limit_soc = line.substring(FindString.length()).toFloat();
    return;
  }
  FindString = "powerLimitTemp: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.power_limit_temp = line.substring(FindString.length()).toFloat();
    return;
  }
  FindString = "maxDischargeCurrent: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.max_discharge_current = line.substring(FindString.length()).toFloat();
    return;
  }
  FindString = "maxRechargeCurrent: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.max_recharge_current = line.substring(FindString.length()).toFloat();
    return;
  }
  FindString = "voltageGood: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.voltage_good = line.substring(FindString.length()).toInt();
    return;
  }
  FindString = "voltageStandby: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.voltage_standby = line.substring(FindString.length()).toInt();
    return;
  }
  FindString = "voltageLow: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.voltage_low = line.substring(FindString.length()).toInt();
    return;
  }
  FindString = "speed: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.speed = line.substring(FindString.length()).toFloat();
    return;
  }
  FindString = "consumption: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.consumption = line.substring(FindString.length()).toFloat();
    return;
  }
  FindString = "remainingDistance: ";
  FindString.toLowerCase();
  if (line.indexOf(FindString) >= 0) {
    data.remainingDistance = line.substring(FindString.length()).toFloat() / 1000.0;
    return;
  }
}
