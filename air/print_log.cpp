#include "print_log.h"

// External variables (defined in main file)
extern WebSocketsServer webSocket;

void print_log(const String& msg) {
    //print serial
    Serial.println(msg);

    //print over websockets
    JsonDocument doc;
    doc["id"] = "debug";
    doc["message"] = msg;
    String out;
    serializeJson(doc, out);
    webSocket.broadcastTXT(out);
}

void print_logf(const char* fmt, ...) {
    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    print_log(String(buf));
}

// For F() macro compatibility
void print_log(const __FlashStringHelper* msg) {
    print_log(String(msg));
}