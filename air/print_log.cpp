#include "print_log.h"

// External variables (defined in main file)
#ifdef USE_ASYNC
//extern AsyncWebServer server;
extern AsyncWebSocket webSocket;
#else
extern WebSocketsServer webSocket;
#endif

void _print_log(const String& msg) {
    
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


#define LOG_QUEUE_SIZE 50
String log_queue[LOG_QUEUE_SIZE];
int log_head = 0;
int log_tail = 0;
int log_count = 0;


void process_log_queue() {
    // Process one message per call to avoid blocking
    if (log_count > 0) {
        String msg = log_queue[log_tail];
        log_tail = (log_tail + 1) % LOG_QUEUE_SIZE;
        log_count--;

        //print over websockets
        JsonDocument doc;
        doc["id"] = "debug";
        doc["message"] = msg;
        String out;
        serializeJson(doc, out);
        webSocket.broadcastTXT(out);
    }
}

bool g_log_async = false;

void set_log_async_mode(bool mode) {
    g_log_async = mode;
}

bool get_log_async_mode() {
    return g_log_async;
}

void print_log(const String& msg) {
    //print serial immediately
    //Serial.println(msg); //disabled to avoid flooding serial output and slowing down processing

    if (g_log_async) {
        //enqueue for websockets
        if (log_count < LOG_QUEUE_SIZE) {
            log_queue[log_head] = msg;
            log_head = (log_head + 1) % LOG_QUEUE_SIZE;
            log_count++;
        } else {
            Serial.println("[WARN] Log queue full!");
            // also send warning over websockets immediately
            JsonDocument doc;
            doc["id"] = "debug";
            doc["message"] = "[WARN] Log queue full!";
            String out;
            serializeJson(doc, out);
            webSocket.broadcastTXT(out);
        }
    } else {
        //print over websockets immediately
        JsonDocument doc;
        doc["id"] = "debug";
        doc["message"] = msg;
        String out;
        serializeJson(doc, out);
        webSocket.broadcastTXT(out);
    }
}