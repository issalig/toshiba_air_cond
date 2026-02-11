#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

// Structure to hold environmental sensor readings
struct env_snapshot {
  float temp;
  float humidity;
  float pressure;
  bool temp_valid;
  bool humidity_valid;
  bool pressure_valid;
  String temp_source;
  String humidity_source;
  String pressure_source;
};

// Function declarations
env_snapshot readEnvironmentalSensors(const char* context = "");
void initAHT20Sensor();
void initBMP280Sensor();
void initBME280Sensor();

#endif // SENSORS_H
