/*
GNU GENERAL PUBLIC LICENSE

Version 2, June 1991

Copyright (C) 1989, 1991 Free Software Foundation, Inc.
51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

Everyone is permitted to copy and distribute verbatim copies
of this license document, but changing it is not allowed.
*/

#include "sensors.h"
#include "config.h"
#include "print_log.h"

#ifdef USE_AHT20
#include <Adafruit_AHTX0.h>
extern Adafruit_AHTX0 aht;
extern float aht_t_current;
extern bool humidity_status;
#endif

#ifdef USE_BMP280
#include <Adafruit_BMP280.h>
extern Adafruit_BMP280 bmp280;
extern float bmp280_t_current;
extern bool pressure_status;
#endif

#ifdef USE_BME280
#include <Adafruit_BME280.h>
extern Adafruit_BME280 bme280;
extern float bme280_t_current;
extern bool bme280_pressure_status;
extern bool bme280_humidity_status;
#endif

extern float sensor_humidity_current;
extern float sensor_pressure_current;

// Common sensor reading function - reads all available environmental sensors
// and returns the best available readings with source information
env_snapshot readEnvironmentalSensors(const char* context) {
  env_snapshot snapshot = {
    .temp = -999.0,
    .humidity = -999.0,
    .pressure = -999.0,
    .temp_valid = false,
    .humidity_valid = false,
    .pressure_valid = false,
    .temp_source = "None",
    .humidity_source = "None",
    .pressure_source = "None"
  };

#ifdef USE_AHT20
  if (humidity_status) {
    sensors_event_t humidity, temp;
    if (aht.getEvent(&humidity, &temp)) {
      aht_t_current = temp.temperature;
      sensor_humidity_current = humidity.relative_humidity;
      
      // AHT20 is preferred for both temperature and humidity
      snapshot.temp = aht_t_current;
      snapshot.humidity = sensor_humidity_current;
      snapshot.temp_valid = true;
      snapshot.humidity_valid = true;
      snapshot.temp_source = "AHT20";
      snapshot.humidity_source = "AHT20";
      
      if (strlen(context) > 0) {
        print_logf("[AHT20][%s] temp %.1f hum %.1f", context, aht_t_current, sensor_humidity_current);
      }
    } else {
      print_logf("[AHT20][%s] Failed to read sensor", context);
      aht_t_current = -999.0;
      sensor_humidity_current = -999.0;
    }
  }
  yield();
#endif

#ifdef USE_BMP280
  if (pressure_status) {
    print_logf("[BMP280][%s] Reading sensor", context);
    bmp280_t_current = bmp280.readTemperature();
    sensor_pressure_current = bmp280.readPressure() / 100.0F;
    
    // Use BMP280 temperature only if AHT20 failed or not available
    if (!snapshot.temp_valid && bmp280_t_current > -999.0) {
      snapshot.temp = bmp280_t_current;
      snapshot.temp_valid = true;
      snapshot.temp_source = "BMP280";
    }
    
    // BMP280 is primary pressure source
    if (sensor_pressure_current > -999.0) {
      snapshot.pressure = sensor_pressure_current;
      snapshot.pressure_valid = true;
      snapshot.pressure_source = "BMP280";
    }
    
    if (strlen(context) > 0) {
      print_logf("[BMP280][%s] temp %.1f press %.1f hPa", context, bmp280_t_current, sensor_pressure_current);
    }
  }
  yield();
#endif

#ifdef USE_BME280
  if (bme280_pressure_status) {
    print_logf("[BME280][%s] Reading sensor", context);
    bme280_t_current = bme280.readTemperature();
    float bme280_humidity = bme280.readHumidity();
    sensor_pressure_current = bme280.readPressure() / 100.0F;
    
    // Use BME280 temperature only if AHT20 failed or not available
    if (!snapshot.temp_valid && bme280_t_current > -999.0) {
      snapshot.temp = bme280_t_current;
      snapshot.temp_valid = true;
      snapshot.temp_source = "BME280";
    }
    
    // Use BME280 humidity only if AHT20 failed or not available
    if (!snapshot.humidity_valid && bme280_humidity > -999.0) {
      snapshot.humidity = bme280_humidity;
      snapshot.humidity_valid = true;
      snapshot.humidity_source = "BME280";
    }
    
    // BME280 is also a pressure source
    if (sensor_pressure_current > -999.0) {
      snapshot.pressure = sensor_pressure_current;
      snapshot.pressure_valid = true;
      snapshot.pressure_source = "BME280";
    }
    
    if (strlen(context) > 0) {
      print_logf("[BME280][%s] temp %.1f hum %.1f press %.1f hPa", context, bme280_t_current, bme280_humidity, sensor_pressure_current);
    }
  }
  yield();
#endif

  return snapshot;
}

// Initialize AHT20 temperature and humidity sensor
void initAHT20Sensor() {
#ifdef USE_AHT20
  if (!aht.begin()) {
    print_log("[AHT20] Could not find a valid sensor, check wiring!");
    humidity_status = false;
  } else {
    print_log("[AHT20] Sensor initialized successfully");
    humidity_status = true;
  }
#endif
}

// Initialize BMP280 pressure and temperature sensor
void initBMP280Sensor() {
#ifdef USE_BMP280
  if (!bmp280.begin()) {
    print_log("[BMP280] Could not find a valid sensor, check wiring!");
    pressure_status = false;
  } else {
    print_log("[BMP280] Sensor initialized successfully");
    pressure_status = true;
    bmp280.setSampling(Adafruit_BMP280::MODE_NORMAL,
                      Adafruit_BMP280::SAMPLING_X2,
                      Adafruit_BMP280::SAMPLING_X16,
                      Adafruit_BMP280::FILTER_X16,
                      Adafruit_BMP280::STANDBY_MS_500);
  }
#endif
}

// Initialize BME280 pressure, temperature and humidity sensor
void initBME280Sensor() {
#ifdef USE_BME280
  if (!bme280.begin(0x76)) {  // Try 0x76 first (common for BME280)
    if (!bme280.begin(0x77)) {  // Try 0x77 as backup
      print_log("[BME280] Could not find a valid sensor, check wiring!");
      bme280_pressure_status = false;
      bme280_humidity_status = false;
    } else {
      print_log("[BME280] Found at address 0x77");
      bme280_pressure_status = true;
      bme280_humidity_status = true;
    }
  } else {
    print_log("[BME280] Found at address 0x76");
    bme280_pressure_status = true;
    bme280_humidity_status = true;
  }
  
  if (bme280_pressure_status) {
    bme280.setSampling(Adafruit_BME280::MODE_NORMAL,
                      Adafruit_BME280::SAMPLING_X2,   // Temperature
                      Adafruit_BME280::SAMPLING_X16,  // Pressure  
                      Adafruit_BME280::SAMPLING_X1,   // Humidity
                      Adafruit_BME280::FILTER_X16,
                      Adafruit_BME280::STANDBY_MS_500);
  }
#endif
}
