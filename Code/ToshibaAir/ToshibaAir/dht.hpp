#pragma once
#include "DHT.h"
#include "config.h"
#include <Arduino.h>
#include "helper.hpp"


struct dht_data {
  float temperature;
  float humidity;
};

typedef struct dht_data dht_data_t;

namespace Sensors {
namespace DHTSensor {

DHT dht(DHTPin, DHTTYPE);

void initialize() { dht.begin(); }

dht_data_t read() {
  dht_data_t result;
  result.temperature = round_f(dht.readTemperature());
  result.humidity = dht.readHumidity();
  return result;
}

} // namespace DHTSensor
} // namespace Sensors