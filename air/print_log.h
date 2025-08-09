#ifndef PRINTLOG_H
#define PRINTLOG_H

#include <Arduino.h>
#include <ArduinoJson.h>

#ifdef USE_ASYNC
#include <ESPAsyncWebServer.h>
#else
#include <WebSocketsServer.h>
#endif

// External function declaration (defined in air.ino)
extern void webSocketBroadcast(String message);

// Logging function declarations
void print_log(const String& msg);
void print_logf(const char* fmt, ...);
void print_log(const __FlashStringHelper* msg);

#endif // PRINTLOG_H