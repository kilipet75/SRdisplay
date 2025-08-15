
int aliveMQTT = 0;


void sendMQTT() {
  aliveMQTT++;
  char msg[64];
  char node[64];

  snprintf(node, sizeof(node), "%s/Alive", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%i", aliveMQTT);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/vcu/hw_version", config.mqtt_node);
  client.publish(node, data.vcu_hw_version.c_str());

  snprintf(node, sizeof(node), "%s/vcu/sw_version", config.mqtt_node);
  client.publish(node, data.vcu_sw_version.c_str());

  snprintf(node, sizeof(node), "%s/vehicle/type", config.mqtt_node);
  client.publish(node, data.vehicle_type.c_str());

  snprintf(node, sizeof(node), "%s/battery/voltage", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%.2f", data.battery_voltage);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/battery/capacity", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%.2f", data.battery_capacity);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/battery/cell_type", config.mqtt_node);
  client.publish(node, data.battery_cell_type.c_str());

  snprintf(node, sizeof(node), "%s/bms/current", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%.2f", data.bms_current);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/bms/soc", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%.2f", data.bms_soc);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/bms/max_temp", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%.2f", data.bms_max_temp);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/bms/min_temp", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%.2f", data.bms_min_temp);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/bms/voltage", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%.2f", data.bms_voltage);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/bms/charge_status", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%.2f", data.bms_charge_status);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/bms/max_cell_diff", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%.2f", data.bms_max_cell_diff);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/ctrl/current", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%.2f", data.ctrl_current);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/ctrl/voltage", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%.2f", data.ctrl_voltage);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/ctrl/throttle", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%.2f", data.ctrl_throttle);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/ctrl/temp", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%.2f", data.ctrl_temp);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/ctrl/motor_temp", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%.2f", data.ctrl_motor_temp);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/ocv_state", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%d", data.ocv_state);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/vcu/soc", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%.2f", data.vcu_soc);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/vcu/voltage", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%.2f", data.vcu_voltage);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/vcu/soc_voltage", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%.2f", data.vcu_soc_voltage);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/vcu/throttle_voltage", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%.2f", data.vcu_throttle_voltage);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/engine_state", config.mqtt_node);
  client.publish(node, data.engine_state.c_str());

  // Digitale Ausgänge
  snprintf(node, sizeof(node), "%s/out/dcdc_en", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%d", data.out_dcdc_en);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/out/discharge", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%d", data.out_discharge);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/out/mcu_limp", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%d", data.out_mcu_limp);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/out/ctrl_en", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%d", data.out_ctrl_en);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/out/ctrl_reverse", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%d", data.out_ctrl_reverse);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/out/brake", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%d", data.out_brake);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/out/mosfet_charge_port", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%d", data.out_mosfet_charge_port);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/out/boot0", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%d", data.out_boot0);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/out/throttle", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%d", data.out_throttle);
  client.publish(node, msg);

  // Leistungsgrenzen
  snprintf(node, sizeof(node), "%s/power_limit/soc", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%.2f", data.power_limit_soc);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/power_limit/temp", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%.2f", data.power_limit_temp);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/power_limit/max_discharge_current", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%.2f", data.max_discharge_current);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/power_limit/max_recharge_current", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%.2f", data.max_recharge_current);
  client.publish(node, msg);

  // Spannungsstatus
  snprintf(node, sizeof(node), "%s/voltage/good", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%d", data.voltage_good);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/voltage/standby", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%d", data.voltage_standby);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/voltage/low", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%d", data.voltage_low);
  client.publish(node, msg);

  // Fahrdaten
  snprintf(node, sizeof(node), "%s/vehicle/speed", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%.2f", data.speed);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/vehicle/consumption", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%.2f", data.consumption);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/vehicle/remaining_distance", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%.2f", data.remainingDistance);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/vehicle/trip_distance", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%.2f", trip_distance_km);
  client.publish(node, msg);

  snprintf(node, sizeof(node), "%s/vehicle/total_distance", config.mqtt_node);
  snprintf(msg, sizeof(msg), "%.2f", total_distance_km);
  client.publish(node, msg);
}


void initMQTT() {
  client.setClient(espClientMQTT);
  client.setServer(config.mqtt_server, config.mqtt_port);

  xTaskCreatePinnedToCore(
    mqttTask,         // Funktion
    "MQTTTask",       // Name
    8192,             // Stack-Größe
    NULL,             // Parameter
    1,                // Priorität
    &mqttTaskHandle,  // Task Handle
    1                 // Core (0 oder 1)
  );
}

void mqttTask(void* parameter) {
  char buf[50];

  for (;;) {
    if (config.mqtt_node[0] != '\0') {
      if ((primaryWLAN || secondaryWLAN)) {
        if (!client.connected()) {
          lv_label_set_text(ui_Label_MQTT1, "MQTT-Server nicht verbunden");
          snprintf(buf, 50, "IP: %s:%s", config.mqtt_server, String(config.mqtt_port));
          lv_label_set_text(ui_Label_MQTT2, buf);
          reconnect();  // MQTT-Verbindung aufbauen
          MQTTconnected = false;
        } else {
          client.loop();  // MQTT-Verarbeitung
          sendMQTT();
          lv_label_set_text(ui_Label_MQTT1, "MQTT-Server verbunden");
          snprintf(buf, 50, "IP: %s:%s", config.mqtt_server, String(config.mqtt_port));
          lv_label_set_text(ui_Label_MQTT2, buf);
          MQTTconnected = true;
        }
      } else {
        lv_label_set_text(ui_Label_MQTT1, "MQTT-Server nicht möglich");        
        lv_label_set_text(ui_Label_MQTT2, "keine WLAN-Verbindung");
        MQTTconnected = false;
      }
    } else {
      lv_label_set_text(ui_Label_MQTT1, "MQTT-Server nicht konfiguriert");
      lv_label_set_text(ui_Label_MQTT2, "");
      MQTTconnected = false;
    }


    vTaskDelay(5000 / portTICK_PERIOD_MS);  // 5s warten
  }
}


void reconnect() {
  while (!client.connected()) {
    Serial.print("MQTT verbindet... ");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("MQTT verbunden");
    } else {
      Serial.print("MQTT Fehlgeschlagen, rc=");
      Serial.println(client.state());
      vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
  }
}
