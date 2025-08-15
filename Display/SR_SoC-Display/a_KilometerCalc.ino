double total_distance_km = 0;
double trip_distance_km = 0;
double last_saved_distance = 0;
bool was_moving = false;

void initDistance() {
  total_distance_km = loadDistance();
  last_saved_distance = total_distance_km;
  trip_distance_km = 0;

  xTaskCreatePinnedToCore(
    distanceTask,
    "Distance Task",
    4096,
    NULL,
    1,
    NULL,
    1);
}

void distanceTask(void *pvParameters) {
  const TickType_t xDelay = pdMS_TO_TICKS(500);

  for (;;) {
    float speed_kmh = data.speed; 
    double speed_ms = speed_kmh / 3.6;
    double delta_distance = speed_ms * 0.5;  // 0.5 Sekunden Intervall

    if (speed_kmh > 1.0) {
      total_distance_km += delta_distance / 1000.0;
      trip_distance_km += delta_distance / 1000.0;
      was_moving = true;
    } else if (was_moving) {
      was_moving = false;

      if (abs(total_distance_km - last_saved_distance) > 0.5) {  // mind. 500 m
        saveDistance(total_distance_km);
        last_saved_distance = total_distance_km;
        Serial.println("Distance saved");
      }
    }

    vTaskDelay(xDelay);
  }
}