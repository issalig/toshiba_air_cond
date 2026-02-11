/*
  GNU GENERAL PUBLIC LICENSE

  Version 2, June 1991

  Copyright (C) 1989, 1991 Free Software Foundation, Inc.
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

  Everyone is permitted to copy and distribute verbatim copies
  of this license document, but changing it is not allowed.

*/

#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include "ac_protocol.h"

// Settings structure for persistent configuration
typedef struct {
  // Serial configuration
  uint8_t txpin;
  uint8_t rxpin;
  unsigned int rxsize;
  
  // I2C configuration
  uint8_t sda_pin;
  uint8_t scl_pin;
  
  // Address configuration
  uint8_t master;
  uint8_t remote;
  
  // Working mode preferences
  uint8_t default_mode;
  uint8_t default_fan;
  uint8_t default_temp;
  bool default_save_mode;
  
  // MQTT settings (if enabled)
  #ifdef USE_MQTT
  char mqtt_server[64];
  uint16_t mqtt_port;
  char mqtt_username[32];
  char mqtt_password[32];
  char mqtt_device_name[32];
  bool mqtt_enabled;
  #endif
  
  // Other settings
  int temperature_interval;  // Sampling interval
  bool autonomous_mode;
  
  // Version and validation
  uint32_t magic_number;  // For validation (e.g., 0xAC123456)
  uint8_t version;
} settings_t;

#define SETTINGS_MAGIC 0xAC123456
#define SETTINGS_VERSION 1
#define SETTINGS_FILENAME "/settings.json"

// Function declarations
bool saveSettings(air_status_t *air, const settings_t *settings);
bool loadSettings(air_status_t *air, settings_t *settings);
void applySettings(air_status_t *air, const settings_t *settings);
void getSettings(air_status_t *air, settings_t *settings);

#endif // SETTINGS_H