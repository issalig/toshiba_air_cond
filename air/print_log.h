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

// Asynchronous logging functions
//void print_log_async(const String& msg);
//void print_logf_async(const char* fmt, ...);
//void process_log_queue();

// Setters and getters for async mode
void set_log_async_mode(bool mode);
bool get_log_async_mode();
void process_log_queue();

#endif // PRINTLOG_H