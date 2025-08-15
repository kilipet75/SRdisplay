double speedActual;
#define SPEEDINCR 1.0

void initDisp() {
  Touch_Init();
  lcd_lvgl_Init();
  ui_init();

  switchViewMode();

  lv_set_DispStd();

  speedActual = data.speed;

  xTaskCreatePinnedToCore(
    dispTask,
    "Display Task",
    4096,
    NULL,
    1,
    NULL,
    1);
}

void dispTask(void* pvParameters) {
  const TickType_t xDelay = pdMS_TO_TICKS(50);
  int Counter = 0;
  for (;;) {
    lv_aktualisieren();
    //lv_start_Animation();
    if (Counter >= 20) {
      Counter = 0;
      check_ui_Settings();
    }
    Counter++;
    vTaskDelay(xDelay);
  }
}

void switchViewMode() {
  if (!config.dark) _ui_switch_theme(UI_THEME_DEFAULT);
  else _ui_switch_theme(UI_THEME_DARKMODE);

  if (config.tacho) lv_disp_load_scr(ui_Tacho);
}

void lv_SetTextLabel(String text, lv_obj_t* label) {
  char buf[50];
  snprintf(buf, 50, "%s", text);
  lv_label_set_text(label, buf);
}

void lv_SetFlagHide(bool hide, lv_obj_t* obj) {
  if (hide) lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
  else lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
}

void lv_aktualisieren() {
  char buf[80];

  if (speedActual < data.speed) {
    if (speedActual + SPEEDINCR > data.speed) speedActual = data.speed;
    else speedActual += SPEEDINCR;
  }
  if (speedActual > data.speed) {
    if (speedActual - SPEEDINCR < data.speed) speedActual = data.speed;
    else speedActual -= SPEEDINCR;
  }

  //Bild mit der Ladeanzeige
  if (lv_scr_act() == ui_Ladeanzeige) {
    lv_SetFlagHide(data.out_brake == 0, ui_Image_Brake);                             //Symbol Bremse
    lv_SetFlagHide(data.engine_state.indexOf("engine_state_run") < 0, ui_Image_GO);  //Symbol OK
    lv_SetFlagHide(!SerialTimeout, ui_Image_Error);                                  //Status Dateneingang
    lv_slider_set_value(ui_AkkuLogo, int(data.bms_soc), LV_ANIM_OFF);                //Akkuanzeige
    snprintf(buf, 4, "%i", int(data.bms_soc));
    lv_label_set_text(ui_Label_SoC_Value, buf);
    snprintf(buf, 4, "%.0f", data.remainingDistance);
    lv_label_set_text(ui_Label_km_Value, buf);
    snprintf(buf, 6, "%s", Uhrzeit);
    lv_label_set_text(ui_Label_Time, buf);
    lv_SetFlagHide(!plugOnline, ui_Image_Shelly);
    if (ladenAktiv) {
      if (config.maxTempReached) {
        lv_obj_clear_flag(ui_Label_LoadTempHigh, LV_OBJ_FLAG_HIDDEN);
      } else {
        lv_obj_add_flag(ui_Label_LoadTempHigh, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_Label_LoadPower, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_Label_Threshold, LV_OBJ_FLAG_HIDDEN);
        //Ladeleistung anzeigen
        snprintf(buf, 80, "Wird geladen mit %.0f W", data.bms_current * data.bms_voltage);
        lv_label_set_text(ui_Label_LoadPower, buf);
        snprintf(buf, 80, "bis %i%%", config.soc_threshold);
        lv_label_set_text(ui_Label_Threshold, buf);
      }
    } else {
      //Animation unsichtbar schalten
      lv_obj_add_flag(ui_Label_LoadTempHigh, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_Label_LoadPower, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_Label_Threshold, LV_OBJ_FLAG_HIDDEN);
    }
    if (data.power_limit_soc < 100.0 || data.power_limit_temp < 100.0) {
      lv_obj_clear_flag(ui_Image_Achtung, LV_OBJ_FLAG_HIDDEN);
    } else lv_obj_add_flag(ui_Image_Achtung, LV_OBJ_FLAG_HIDDEN);

    snprintf(buf, 80, "%i°C", int(data.bms_max_temp));
    lv_label_set_text(ui_Label_BattTemp_Value, buf);
    if (data.bms_max_temp < 40.0) {
      //unter 40° alles ok
      lv_obj_set_style_bg_opa(ui_Label_BattTemp_Value, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    } else if (data.bms_max_temp >= 45.0) {
      //über 45°->Rot
      lv_obj_set_style_bg_color(ui_Label_BattTemp_Value, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_bg_opa(ui_Label_BattTemp_Value, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    } else {
      //zwischen 40 und 45° Orange
      lv_obj_set_style_bg_color(ui_Label_BattTemp_Value, lv_color_hex(0xEE8D00), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_bg_opa(ui_Label_BattTemp_Value, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    //Bild mit Tacho
  } else if (lv_scr_act() == ui_Tacho) {
    lv_SetFlagHide(data.out_brake == 0, ui_Image_Brake_2);                             //Symbol Bremse
    lv_SetFlagHide(data.engine_state.indexOf("engine_state_run") < 0, ui_Image_GO_2);  //Symbol OK
    lv_SetFlagHide(!SerialTimeout, ui_Image_Error_2);                                  //Status Dateneingang
    lv_slider_set_value(ui_AkkuLogo_2, int(data.bms_soc), LV_ANIM_OFF);                //Akkuanzeige
    lv_obj_invalidate(ui_Arc_Speed_2);
    lv_arc_set_value(ui_Arc_Speed_2, int(speedActual));  //Tacho Bogen
    lv_obj_invalidate(ui_Zeiger);
    lv_img_set_angle(ui_Zeiger, int(((speedActual / 80.0) * (2 * 1380.0)) - 1380.0));  //Tacho Zeiger --> 0kmh=-1380°; 80kmh=+1380°
    lv_obj_invalidate(ui_Arc_Reku);
    lv_arc_set_value(ui_Arc_Reku, int((data.bms_current * data.bms_voltage * 1.0) / 40.0));  //Rekuperationsanzeige -100% bis 100%

    snprintf(buf, 4, "%i", int(data.speed));
    lv_obj_invalidate(ui_Label_Speed_Value_2);
    lv_label_set_text(ui_Label_Speed_Value_2, buf);
    snprintf(buf, 4, "%i", int(data.bms_soc));
    lv_label_set_text(ui_Label_SoC_Value_2, buf);
    snprintf(buf, 4, "%.i", int(data.remainingDistance));
    lv_label_set_text(ui_Label_km_Value_2, buf);
    snprintf(buf, 6, "%s", Uhrzeit);
    lv_label_set_text(ui_Label_Time_2, buf);
    lv_SetFlagHide(!plugOnline, ui_Image_Shelly_2);

    if (ladenAktiv) {
      if (config.maxTempReached) {
        lv_obj_clear_flag(ui_Label_LoadTempHigh2, LV_OBJ_FLAG_HIDDEN);
      } else {
        lv_obj_add_flag(ui_Label_LoadTempHigh2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_Label_LoadPower_2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_Label_Threshold_2, LV_OBJ_FLAG_HIDDEN);
        //Ladeleistung anzeigen
        snprintf(buf, 80, "Wird geladen mit %.0f W", data.bms_current * data.bms_voltage);
        lv_label_set_text(ui_Label_LoadPower_2, buf);
        snprintf(buf, 80, "bis %i%%", config.soc_threshold);
        lv_label_set_text(ui_Label_Threshold_2, buf);
      }
    } else {
      //Animation unsichtbar schalten
      lv_obj_add_flag(ui_Label_LoadTempHigh2, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_Label_LoadPower_2, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_Label_Threshold_2, LV_OBJ_FLAG_HIDDEN);
    }
    if ((data.power_limit_soc < 100.0 || data.power_limit_temp < 100.0) && data.engine_state.indexOf("engine_state_run") >= 0) {
      lv_obj_clear_flag(ui_Image_Achtung2, LV_OBJ_FLAG_HIDDEN);
    } else lv_obj_add_flag(ui_Image_Achtung2, LV_OBJ_FLAG_HIDDEN);

    snprintf(buf, 80, "%i°C", int(data.bms_max_temp));
    lv_label_set_text(ui_Label_BattTemp_Value2, buf);
    if (data.bms_max_temp < 40.0) {
      //unter 40° alles ok
      lv_obj_set_style_bg_opa(ui_Label_BattTemp_Value2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    } else if (data.bms_max_temp >= 45.0) {
      //über 45°->Rot
      lv_obj_set_style_bg_color(ui_Label_BattTemp_Value2, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_bg_opa(ui_Label_BattTemp_Value2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    } else {
      //zwischen 40 und 45° Orange
      lv_obj_set_style_bg_color(ui_Label_BattTemp_Value2, lv_color_hex(0xEE8D00), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_bg_opa(ui_Label_BattTemp_Value2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    }


  } else if (lv_scr_act() == ui_Settings) {

  } else if (lv_scr_act() == ui_WiFiInfo) {

  } else if (lv_scr_act() == ui_SRInfo) {
    snprintf(buf, 80, "%.0f°C", data.bms_max_temp);
    lv_label_set_text(ui_Label_BattMaxTemp, buf);
    snprintf(buf, 80, "%.0f°C", data.bms_min_temp);
    lv_label_set_text(ui_Label_BattMinTemp, buf);
    snprintf(buf, 80, "%.0fV", data.bms_voltage);
    lv_label_set_text(ui_Label_BattVoltage, buf);
    snprintf(buf, 80, "%.1fA", data.bms_current);
    lv_label_set_text(ui_Label_BattCurrent, buf);
    snprintf(buf, 80, "%.1f%%", data.bms_soc);
    lv_label_set_text(ui_Label_BattSoC, buf);
    lv_slider_set_value(ui_Slider_Batterie, int(data.bms_soc), LV_ANIM_OFF);
    snprintf(buf, 80, "%.0f°C", data.ctrl_temp);
    lv_label_set_text(ui_Label_CtrlTemp, buf);
    snprintf(buf, 80, "%.0f°C", data.ctrl_motor_temp);
    lv_label_set_text(ui_Label_MotorTemp, buf);
    snprintf(buf, 80, "%.0f%%", data.power_limit_soc);
    lv_label_set_text(ui_Label_limitSoC, buf);
    snprintf(buf, 80, "%.0f%%", data.power_limit_temp);
    lv_label_set_text(ui_Label_limitTemp, buf);
    snprintf(buf, 80, "%.0fW", plugPower);
    lv_label_set_text(ui_Label_ChargePower, buf);
    lv_SetFlagHide(!ladenAktiv, ui_Panel_ChargerON);

    snprintf(buf, 80, "HW:%s", data.vcu_hw_version);
    lv_label_set_text(ui_Label_HWversion, buf);
    snprintf(buf, 80, "SW:%s", data.vcu_sw_version);
    lv_label_set_text(ui_Label_SWversion, buf);
    snprintf(buf, 80, "DISP:%s", DISPVERSION);
    lv_label_set_text(ui_Label_DISPversion, buf);

    snprintf(buf, 80, "%.1f", trip_distance_km);
    lv_label_set_text(ui_Label_tripDistance, buf);
    snprintf(buf, 80, "%.0f", total_distance_km);
    lv_label_set_text(ui_Label_totalDistance, buf);
  }
}

void lv_diagnose() {
  char buf[80];

  snprintf(buf, 80, "%.0f°C", data.bms_max_temp);
  lv_label_set_text(ui_Label_BattMaxTemp, buf);
  snprintf(buf, 80, "%.0f°C", data.bms_min_temp);
  lv_label_set_text(ui_Label_BattMinTemp, buf);
  snprintf(buf, 80, "%.0fV", data.bms_voltage);
  lv_label_set_text(ui_Label_BattVoltage, buf);
  snprintf(buf, 80, "%.1fA", data.bms_current);
  lv_label_set_text(ui_Label_BattCurrent, buf);
  snprintf(buf, 80, "%.1f%%", data.bms_soc);
  lv_label_set_text(ui_Label_BattSoC, buf);
  lv_slider_set_value(ui_Slider_Batterie, int(data.bms_soc), LV_ANIM_OFF);
  snprintf(buf, 80, "%.0f°C", data.ctrl_temp);
  lv_label_set_text(ui_Label_CtrlTemp, buf);
  snprintf(buf, 80, "%.0f°C", data.ctrl_motor_temp);
  lv_label_set_text(ui_Label_MotorTemp, buf);
  snprintf(buf, 80, "%.0f%%", data.power_limit_soc);
  lv_label_set_text(ui_Label_limitSoC, buf);
  snprintf(buf, 80, "%.0f%%", data.power_limit_temp);
  lv_label_set_text(ui_Label_limitTemp, buf);
  snprintf(buf, 80, "%.0fW", plugPower);
  lv_label_set_text(ui_Label_ChargePower, buf);
  lv_SetFlagHide(!ladenAktiv, ui_Panel_ChargerON);

  snprintf(buf, 80, "HW:%s", data.vcu_hw_version);
  lv_label_set_text(ui_Label_HWversion, buf);
  snprintf(buf, 80, "SW:%s", data.vcu_sw_version);
  lv_label_set_text(ui_Label_SWversion, buf);
}


void check_ui_Settings() {
  float set = float((lv_roller_get_selected(ui_SoCsetpoint) + 1) * 5);
  bool save = false;

  if (set != config.soc_threshold) {
    Serial.println("Ladegrenze wurde über das Display geändert" + String(set));
    config.soc_threshold = set;
    save = true;
    msgSent = false;
  }
  if (lv_obj_has_state(ui_SwitchDark, LV_STATE_CHECKED) != config.dark) {
    Serial.println("Anzeigeeinstellung geändert" + String(set));
    config.dark = lv_obj_has_state(ui_SwitchDark, LV_STATE_CHECKED);
    save = true;
  }
  if (save) saveConfig();
}

void lv_set_DispStd() {
  char buf[5];

  //Main
  lv_obj_add_flag(ui_Image_Shelly, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_Image_WLAN, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_Image_HotSpot, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_Label_LoadPower, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_Label_Threshold, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_Label_LoadTempHigh, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_Image_Brake, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_Image_GO, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_Image_Error, LV_OBJ_FLAG_HIDDEN);
  lv_label_set_text(ui_Label_SoC_Value, "0");
  lv_label_set_text(ui_Label_km_Value, "0");
  lv_slider_set_value(ui_AkkuLogo, 0, LV_ANIM_OFF);
  lv_SetFlagHide(!config.clock, ui_Label_Time);

  //Main2
  lv_obj_add_flag(ui_Image_Shelly_2, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_Image_WLAN_2, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_Image_HotSpot_2, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_Label_LoadPower_2, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_Label_Threshold_2, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_Label_LoadTempHigh2, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_Image_Brake_2, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_Image_GO_2, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_Image_Error_2, LV_OBJ_FLAG_HIDDEN);
  lv_label_set_text(ui_Label_SoC_Value_2, "0");
  lv_label_set_text(ui_Label_km_Value_2, "0");
  lv_arc_set_value(ui_Arc_Speed_2, 0);
  lv_SetFlagHide(!config.clock, ui_Label_Time_2);

  //Settings
  lv_roller_set_selected(ui_SoCsetpoint, int(config.soc_threshold / 5.0) - 1, LV_ANIM_OFF);
  lv_obj_set_style_bg_color(ui_Button_StartStop, lv_color_hex(0x9F9F9F), LV_PART_MAIN | LV_STATE_DEFAULT);

  //Wifi-Info
  lv_label_set_text(ui_Label_WiFi1, " ");
  lv_label_set_text(ui_Label_WiFi2, " ");
  lv_label_set_text(ui_Label_WiFi3, " ");

  lv_label_set_text(ui_Label_Shelly1, " ");
  lv_label_set_text(ui_Label_Shelly2, " ");


  if (config.dark) lv_obj_add_state(ui_SwitchDark, LV_STATE_CHECKED);
  else lv_obj_clear_state(ui_SwitchDark, LV_STATE_CHECKED);

  lv_set_ThemeColors();
}

void lv_set_ThemeColors() {
  _ui_theme_color_BackColor[0] = config.backColorLight;
  _ui_theme_color_BackColor[1] = config.backColorDark;
  _ui_theme_color_ForeColor[0] = config.foreColorLight;
  _ui_theme_color_ForeColor[1] = config.foreColorDark;
  _ui_theme_color_BackScale[0] = config.backColorScaleLight;
  _ui_theme_color_BackScale[1] = config.backColorScaleDark;
  _ui_theme_color_FrontScale[0] = config.foreColorScaleLight;
  _ui_theme_color_FrontScale[1] = config.foreColorScaleDark;
  _ui_theme_color_PanelColor[0] = config.PanelColorLight;
  _ui_theme_color_PanelColor[1] = config.PanelColorDark;

  _ui_theme_set_variable_styles(UI_VARIABLE_STYLES_MODE_FOLLOW);
  if (!config.dark) {
    _ui_switch_theme(UI_THEME_DEFAULT);

  } else {
    _ui_switch_theme(UI_THEME_DARKMODE);
  }
}

void lv_start_Animation() {
  char buf[10];

  if (data.engine_state.indexOf("engine_state_charging_") >= 0) {
    if (!ladenAktiv) {
      ladenAktiv = true;

      //Animation sichtbar schalten
      lv_obj_clear_flag(ui_Label_LoadPower, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(ui_Label_Threshold, LV_OBJ_FLAG_HIDDEN);

      lv_obj_clear_flag(ui_Label_LoadPower_2, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(ui_Label_Threshold_2, LV_OBJ_FLAG_HIDDEN);
    }

    //Ladeleistung anzeigen
    snprintf(buf, 10, "%.0f W", data.bms_current * data.bms_voltage);
    lv_label_set_text(ui_Label_LoadPower, buf);
    lv_label_set_text(ui_Label_LoadPower_2, buf);
    snprintf(buf, 10, "bis %i%%", config.soc_threshold);
    lv_label_set_text(ui_Label_Threshold, buf);
    lv_label_set_text(ui_Label_Threshold_2, buf);
  } else {
    if (ladenAktiv) {
      ladenAktiv = false;
      //Animation unsichtbar schalten
      lv_obj_add_flag(ui_Label_LoadPower, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_Label_Threshold, LV_OBJ_FLAG_HIDDEN);

      lv_obj_add_flag(ui_Label_LoadPower_2, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_Label_Threshold_2, LV_OBJ_FLAG_HIDDEN);
    }
  }
}
