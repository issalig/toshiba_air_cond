#pragma once
#include <Arduino.h>
#include "config.h" // check it for settings
#include "helper.hpp"
#include "LittleFS.h"
#include "ntp_timer.hpp"
#include "wifi.hpp"
#include "ota.hpp"



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


#include <ESP8266mDNS.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <NTPClient.h> 
#include <WiFiUdp.h>
#include "toshiba_serial.hpp"
#include "MySimpleTimer.hpp"
#include "process_request.hpp"


//#include <GDBStub.h>

const char compile_date[] = __DATE__ " " __TIME__;