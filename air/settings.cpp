/*
  GNU GENERAL PUBLIC LICENSE

  Version 2, June 1991

  Copyright (C) 1989, 1991 Free Software Foundation, Inc.
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

  Everyone is permitted to copy and distribute verbatim copies
  of this license document, but changing it is not allowed.

*/

#include <ESP8266WiFi.h> 
#include <Wire.h>
#include "config.h"
#include "settings.h"
#include "print_log.h"
#include <ArduinoJson.h>
#include <LittleFS.h>

extern int temperature_interval;
extern bool autonomous_mode;

#ifdef USE_MQTT
#include "mqtt.h"
#endif

bool saveSettings(air_status_t *air, const settings_t *settings) {
  JsonDocument doc;
  
  // Add all settings to JSON
  doc["magic"] = settings->magic_number;
  doc["version"] = settings->version;
  
  // Serial configuration
  JsonObject serial = doc["serial"].to<JsonObject>();
  serial["tx_pin"] = settings->txpin;
  serial["rx_pin"] = settings->rxpin;
  serial["rx_buffer_size"] = settings->rxsize;
  
  // I2C configuration
  JsonObject i2c = doc["i2c"].to<JsonObject>();
  i2c["sda_pin"] = settings->sda_pin;
  i2c["scl_pin"] = settings->scl_pin;
  
  // Address configuration
  JsonObject addresses = doc["addresses"].to<JsonObject>();
  addresses["master"] = settings->master;
  addresses["remote"] = settings->remote;
  
  // Working mode preferences
  JsonObject mode = doc["mode"].to<JsonObject>();
  mode["default_mode"] = settings->default_mode;
  mode["default_fan"] = settings->default_fan;
  mode["default_temp"] = settings->default_temp;
  mode["default_save"] = settings->default_save_mode;
  
  // Other settings
  doc["temperature_interval"] = settings->temperature_interval;
  doc["autonomous_mode"] = settings->autonomous_mode;
  
  #ifdef USE_MQTT
  JsonObject mqtt = doc["mqtt"].to<JsonObject>();
  mqtt["server"] = settings->mqtt_server;
  mqtt["port"] = settings->mqtt_port;
  mqtt["username"] = settings->mqtt_username;
  mqtt["password"] = settings->mqtt_password;
  mqtt["device_name"] = settings->mqtt_device_name;
  mqtt["enabled"] = settings->mqtt_enabled;
  #endif
  
  // Save to file
  File file = LittleFS.open(SETTINGS_FILENAME, "w");
  if (!file) {
    Serial.println("[SETTINGS] Failed to open file for writing");
    return false;
  }
  
  if (serializeJson(doc, file) == 0) {
    Serial.println("[SETTINGS] Failed to write to file");
    file.close();
    return false;
  }
  
  file.close();
  Serial.println("[SETTINGS] Settings saved successfully");
  return true;
}

bool loadSettings(air_status_t *air, settings_t *settings) {
  if (!LittleFS.exists(SETTINGS_FILENAME)) {
    Serial.println("[SETTINGS] Settings file not found");
    return false;
  }
  
  File file = LittleFS.open(SETTINGS_FILENAME, "r");
  if (!file) {
    Serial.println("[SETTINGS] Failed to open file for reading");
    return false;
  }
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    Serial.printf("[SETTINGS] Failed to parse JSON: %s\n", error.c_str());
    return false;
  }
  
  // Validate settings
  if (doc["magic"] != SETTINGS_MAGIC) {
    Serial.println("[SETTINGS] Invalid settings file (magic number mismatch)");
    return false;
  }
  
  // Load settings
  settings->magic_number = doc["magic"];
  settings->version = doc["version"];
  
  // Serial configuration
  settings->txpin = doc["serial"]["tx_pin"] | air->txpin;
  settings->rxpin = doc["serial"]["rx_pin"] | air->rxpin;
  settings->rxsize = doc["serial"]["rx_buffer_size"] | air->rxsize;
  
  // I2C configuration
  settings->sda_pin = doc["i2c"]["sda_pin"] | air->sda_pin;
  settings->scl_pin = doc["i2c"]["scl_pin"] | air->scl_pin;
  
  // Address configuration
  settings->master = doc["addresses"]["master"] | air->master;
  settings->remote = doc["addresses"]["remote"] | air->remote;
  
  // Working mode preferences
  settings->default_mode = doc["mode"]["default_mode"] | MODE_AUTO;
  settings->default_fan = doc["mode"]["default_fan"] | FAN_AUTO;
  settings->default_temp = doc["mode"]["default_temp"] | 24;
  settings->default_save_mode = doc["mode"]["default_save"] | false;
  
  // Other settings
  settings->temperature_interval = doc["temperature_interval"] | 60;
  settings->autonomous_mode = doc["autonomous_mode"] | false;
  
  #ifdef USE_MQTT
  strlcpy(settings->mqtt_server, doc["mqtt"]["server"] | "", sizeof(settings->mqtt_server));
  settings->mqtt_port = doc["mqtt"]["port"] | 1883;
  strlcpy(settings->mqtt_username, doc["mqtt"]["username"] | "", sizeof(settings->mqtt_username));
  strlcpy(settings->mqtt_password, doc["mqtt"]["password"] | "", sizeof(settings->mqtt_password));
  strlcpy(settings->mqtt_device_name, doc["mqtt"]["device_name"] | "toshiba_ac", sizeof(settings->mqtt_device_name));
  settings->mqtt_enabled = doc["mqtt"]["enabled"] | false;
  #endif
  
  Serial.println("[SETTINGS] Settings loaded successfully");
  return true;
}

void applySettings(air_status_t *air, const settings_t *settings) {
  // Apply serial settings (requires restart to take full effect)
  air->txpin = settings->txpin;
  air->rxpin = settings->rxpin;
  air->rxsize = settings->rxsize;
  
  // Apply I2C settings BEFORE sensors are initialized
  air->sda_pin = settings->sda_pin;
  air->scl_pin = settings->scl_pin;
  
  // Reinitialize I2C bus with new pins
  //Wire.end();
  Wire.begin(air->sda_pin, air->scl_pin);
  delay(100); // Allow bus to stabilize
  
  print_logf("[SETTINGS] I2C reinitialized - SDA: GPIO%d, SCL: GPIO%d", 
             settings->sda_pin, settings->scl_pin);
  
  // Apply addresses
  air->master = settings->master;
  air->remote = settings->remote;
  
  // Apply working mode preferences
  // These are stored but not applied immediately
  
  // Apply other settings
  temperature_interval = settings->temperature_interval;
  autonomous_mode = settings->autonomous_mode;
  
  #ifdef USE_MQTT
  if (settings->mqtt_enabled) {
    configureMQTT(settings->mqtt_server, settings->mqtt_port, 
                  settings->mqtt_username, settings->mqtt_password, 
                  settings->mqtt_device_name);
    setMQTTEnabled(true);
  }
  #endif
  
  print_log("[SETTINGS] Settings applied - restart may be required for some changes");
}

void getSettings(air_status_t *air, settings_t *settings) {
  settings->magic_number = SETTINGS_MAGIC;
  settings->version = SETTINGS_VERSION;
  
  // Get current serial configuration
  settings->txpin = air->txpin;
  settings->rxpin = air->rxpin;
  settings->rxsize = air->rxsize;
  
  // Get current I2C configuration
  settings->sda_pin = air->sda_pin;
  settings->scl_pin = air->scl_pin;
  
  // Get current addresses
  settings->master = air->master;
  settings->remote = air->remote;
  
  // Get current working mode as defaults
  settings->default_mode = air->mode;
  settings->default_fan = air->fan;
  settings->default_temp = air->target_temp;
  settings->default_save_mode = air->save;
  
  // Get other settings
  settings->temperature_interval = temperature_interval;
  settings->autonomous_mode = autonomous_mode;
  
  #ifdef USE_MQTT
  strlcpy(settings->mqtt_server, getMQTTServer().c_str(), sizeof(settings->mqtt_server));
  settings->mqtt_port = getMQTTPort();
  strlcpy(settings->mqtt_username, getMQTTUser().c_str(), sizeof(settings->mqtt_username));
  strlcpy(settings->mqtt_password, getMQTTPassword().c_str(), sizeof(settings->mqtt_password));
  strlcpy(settings->mqtt_device_name, getMQTTDeviceName().c_str(), sizeof(settings->mqtt_device_name));
  settings->mqtt_enabled = air->mqtt_enabled;
  #endif
}