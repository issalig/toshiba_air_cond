#pragma once
#include <Arduino.h>
#include "config.h" // check it for settings
#include "helper.hpp"
#include "LittleFS.h"

#include <ESP8266WiFi.h>
#include "DHT.h"

#define SPIFFS LittleFS //dirty hack not to change names in the migration of SPIFFS to LittleFS
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
#include "..\doc\examples\air_test\toshiba_serial.hpp"
#include "MySimpleTimer.hpp"
#include "process_request.hpp"


//#include <GDBStub.h>

const char compile_date[] = __DATE__ " " __TIME__;

//for LED status
#include <Ticker.h>
Ticker ticker;

#ifndef LED_BUILTIN
  #define LED_BUILTIN 13 // ESP32 DOES NOT DEFINE LED_BUILTIN
#endif

int LED = LED_BUILTIN;

const int DHTPin = D3;
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321