#pragma once
#include "config.h"
#include <Arduino.h>
#include "dht.hpp"
#include "bmp.hpp"

namespace Sensors {

bool dht_status = false;
bool bmp_status = false;

void initialize() {
#ifdef USE_DHT
  dht_status = true;
  DHTSensor::initialize();
#endif
#ifdef USE_BMP
  bmp_status = true;
  BMPSensor::initialize();
#endif
}

dht_data_t read_dht() { return DHTSensor::read(); }

bmp_data_t read_bmp() { return BMPSensor::read(); }
} // namespace Sensors
