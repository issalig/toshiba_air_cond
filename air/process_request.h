#ifndef PROCESS_REQUEST_H
#define PROCESS_REQUEST_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "ac_protocol.h"
#include "my_timer.h"
#ifdef USE_ASYNC
#include <ESPAsyncWebServer.h>
#else
#include <WebSocketsServer.h>
#endif
#include "config.h"             // For air_status_t and other configurations
#include <ArduinoJson.h>         // For JsonDocument
#include <Arduino.h>             // For String, Serial, etc.
#include <ESP8266WiFi.h>         // For WiFiUDP
#include <WiFiUdp.h>             // For WiFiUDP
#include <NTPClient.h>           // For NTPClient

// Function declarations
void processRequest(uint8_t *payload);
String air_to_json(air_status_t *air);
String string_to_json(String t);
String timeseries_to_json(String id, String val, void *data, int data_type, int temp_idx);
String txrx_data_to_json(air_status_t *air);

// Helper functions for array serialization
void serialize_array_float(float *ptr, JsonArray &arr, int idx);
void serialize_array_ul_int(unsigned long *ptr, JsonArray &arr, int idx);
void serialize_array_int(int *ptr, JsonArray &arr, int idx);

void notifyWebSocketClients();
void notifyTXRXData(); 

#endif // PROCESS_REQUEST_H