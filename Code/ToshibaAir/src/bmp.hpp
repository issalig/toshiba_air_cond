#pragma once
#include "config.h"
#include <Adafruit_BMP085.h>
#include <Arduino.h>
#include <Wire.h>


struct bmp_data {
  float temperature;
  float pressure;
};

typedef struct bmp_data bmp_data_t;

namespace Sensors {
namespace BMPSensor {

Adafruit_BMP085 bmp;

void initialize() { bmp.begin(); }

bmp_data_t read() {
  bmp_data_t result;
  result.temperature = bmp.readTemperature();
  result.pressure = bmp.readPressure() / 100; // in mb
  return result;
}

} // namespace BMPSensor
} // namespace Sensors