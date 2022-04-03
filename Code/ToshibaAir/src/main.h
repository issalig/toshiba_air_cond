#pragma once
#include <Arduino.h>
#include "LittleFS.h"
#define SPIFFS LittleFS //dirty hack not to change names in the migration of SPIFFS to LittleFS

#include <ESP8266WiFi.h>

#ifdef USE_ASYNC //for ESP32
  #include <ESPAsync_WiFiManager.h_>
  #include <ESPAsyncTCP.h_>
  #include <ESPAsyncWebServer.h_>
#else //use links2004 websocketserver
  #include <WiFiManager.h>
  #include <ESP8266WiFiMulti.h>
  #include <ESP8266WebServer.h>
  #include <WebSocketsServer.h>
#endif

#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <NTPClient.h> 
#include <WiFiUdp.h>
#include "toshiba_serial.hpp"
#include "MySimpleTimer.hpp"
#include "process_request.hpp"

#include "config.h" // check it for settings

//#include <GDBStub.h>

const char compile_date[] = __DATE__ " " __TIME__;

//for LED status
#include <Ticker.h>
Ticker ticker;

#ifndef LED_BUILTIN
  #define LED_BUILTIN 13 // ESP32 DOES NOT DEFINE LED_BUILTIN
#endif

int LED = LED_BUILTIN;