#ifndef PRINTLOG_H
#define PRINTLOG_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WebSocketsServer.h>

// Logging function declarations
void print_log(const String& msg);
void print_logf(const char* fmt, ...);
void print_log(const __FlashStringHelper* msg);

#endif // PRINTLOG_H