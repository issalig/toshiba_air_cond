/*
  Name:         air.ino
  Description:  Controls toshiba air conditioner
  Author:       issalig

  History:      01/06/2020 initial version
                24/07/2025 v2.0 superloaded with MQTT, OLED and a lots of goodies

  Instructions:
                HW
                R/W circuit (see readme.md)
                Oled screen connected to D1 (SCL) D2 (SDA) (optional)
                AHT20 + BMP280 connected to D1 (SCL) D2 (SDA) (optional but one temperature sensor is needed if autonomous mode is used)

                Software serial rx on D7, tx on D8
                Wemos mini D1

                SW
                The recommended editor is VSCode with PlatformIO, but you can use Arduino IDE too.
                Upload data directory with ESP8266SketchDataUpload and flash it with USB for the first time. Then, when installed you can use OTA updates.                
                If you want to use MQTT, set the MQTT_HOST, MQTT_PORT, MQTT_USER and MQTT_PASSWORD
                Web server is available at http://air.local and OTA password is set in config.h

  References:   https://github.com/tttapa/ESP8266/
                https://github.com/luisllamasbinaburo/ESP8266-Examples
                https://diyprojects.io/esp8266-web-server-part-5-add-google-charts-gauges-and-charts/#.X0gBsIbtY5k

  Dependencies:
                ESPSoftwareSerial
                Adafruit SSD1306
                Adafruit GFX Library
                WifiManager
                ESP8266WiFi
                links2004/WebSockets
                ArduinoJson
                LittleFS
                arduino-libraries/NTPClient
                adafruit/DHT sensor library
                adafruit/Adafruit BMP085 Library
                knolleary/PubSubClient

  This code in under license GPL v2

  GNU GENERAL PUBLIC LICENSE

  Version 2, June 1991

  Copyright (C) 1989, 1991 Free Software Foundation, Inc.
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

  Everyone is permitted to copy and distribute verbatim copies
  of this license document, but changing it is not allowed.
*/

#include "config.h" // check it for settings
#include "file_manager_html.h" //unified file manager
#include "display.h"

#include "LittleFS.h"
#include <ESP8266WiFi.h>

#ifdef USE_ASYNC // AsyncWebServer
#include <WiFiManager.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#else //use links2004 websocketserver
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h> //https://github.com/Links2004/arduinoWebSockets/
#endif

#include <SPI.h>

#ifdef USE_OTA
#include <ArduinoOTA.h>
#endif

#include <ESP8266mDNS.h>
#include <FS.h>
#include <ArduinoJson.h>  //by Benoit Blanchon
#include <NTPClient.h>    //https://github.com/arduino-libraries/NTPClient
#include <WiFiUdp.h>
#include "ac_protocol.h"
//#include "MySimpleTimer.hpp"
#include "my_timer.h"
#include "process_request.h"
#include "print_log.h"  // print log functions
#ifdef USE_TELEGRAM
#include "telegram_bot.h" // Telegram bot functions
#endif

#ifdef USE_MQTT
#include "mqtt.h"
extern bool getMQTTStatus();
#endif 


//for LED status
#include <Ticker.h>
Ticker ticker;

#ifndef LED_BUILTIN
#define LED_BUILTIN 13 // ESP32 DOES NOT DEFINE LED_BUILTIN
#endif

int LED = LED_BUILTIN;

//not necessary because now we use WifiManager
//change it according to your network
IPAddress ip(192, 168, 2, 200);
IPAddress gateway(192, 168, 2, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);

air_status_t air_status;
my_timer timerOnOff;

#ifdef USE_ASYNC
AsyncWebServer server(80);
AsyncWebSocket webSocket("/ws");
#else
ESP8266WebServer server(80);       // create a web server on port 80
WebSocketsServer webSocket(81);    // create a websocket server on port 81
#endif

File fsUploadFile;                 // a File variable to temporarily store the received file


#ifdef USE_AHT20
#include <Adafruit_AHTX0.h>
Adafruit_AHTX0 aht;
#endif

#ifdef USE_BMP280
#include <Adafruit_BMP280.h>
Adafruit_BMP280 bmp280;
#endif

#ifdef USE_AHT20
float aht_t_current = -999.0;
bool humidity_status = false;
#endif

#ifdef USE_BMP280
float bmp280_t_current = -999.0;
bool pressure_status = false;
#endif


// Arrays for storing sensor data
float ac_sensor_temperature[MAX_LOG_DATA];
int ac_outdoor_te[MAX_LOG_DATA];
float sensor_temperature[MAX_LOG_DATA]; // Centralized temperature from best available sensor
float sensor_humidity[MAX_LOG_DATA];    // Centralized humidity (available regardless of sensor type)
float sensor_pressure[MAX_LOG_DATA];    // Centralized pressure
unsigned long timestamps[MAX_LOG_DATA]; // Timestamps for sensor readings
int temp_idx = 0; // Index for circular temperature arrays

// Current readings (for real-time data)
float sensor_temperature_current = -999.0;  // Current temperature from best sensor
float sensor_humidity_current = -999.0;     // Current humidity from best sensor
float sensor_pressure_current = -999.0;     // Current pressure from best sensor


// sensors ids
byte sensor_ids[] = {
  INDOOR_ROOM,
  INDOOR_FAN_SPEED,
  INDOOR_TA, INDOOR_TCJ, INDOOR_TC,
  INDOOR_FILTER_TIME,
  INDOOR_FAN_RUN_TIME,
  OUTDOOR_TE, OUTDOOR_TO,
  OUTDOOR_TD, OUTDOOR_TS, OUTDOOR_THS,
  OUTDOOR_CURRENT,
  OUTDOOR_HOURS, 
  OUTDOOR_TL, 
  OUTDOOR_COMP_FREQ,
  OUTDOOR_LOWER_FAN_SPEED, 
  OUTDOOR_UPPER_FAN_SPEED
};

// Define NTP Client to get time
WiFiUDP ntpUDP;
const long utcOffsetInSeconds = 3600; //1 hour for Europe/Brussels
int timeOffset = 0;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 0); //do not apply utc here

// Timers
my_timer timerTemperature;
my_timer timerStatus;
my_timer timerReadSerial;
my_timer timerSaveFile;
my_timer timerMQTT; 
my_timer timerAutonomous;
//MySimpleTimer timerTelegram;

bool autonomous_mode = false; // use autonomous mode if there is no wired remote
bool simulation_mode = false; // Set to true to enable simulation mode without AC unit


int temp_interval = 1800; //in secs, 30 mins for temperature readings

//bool mqtt_enabled = true;



/*__________________________________________________________SETUP__________________________________________________________*/

void setup() {

  my_timer_init(&timerOnOff);
  my_timer_init(&timerTemperature);
  my_timer_init(&timerStatus);
  my_timer_init(&timerReadSerial);
  my_timer_init(&timerSaveFile);
  my_timer_init(&timerMQTT);
  my_timer_init(&timerAutonomous);
  #ifdef USE_TELEGRAM
  my_timer_init(&timerTelegram);
  #endif

  Serial.begin(9600);        // Start the Serial communication to send messages to the computer
  delay(10);
  print_log("Air conditioning starts!");

  #ifdef USE_SCREEN
  startDisplay();
  #endif  
  startWifiManager();          // Use wifimanager to connect
  //is_reset_button();
  #ifdef USE_OTA
  startOTA();                  // Start the OTA service
  #endif

  startLittleFS();             // Start the LittleFS and list all contents
  startWebSocket();            // Start a WebSocket server
  startMDNS();                 // Start the mDNS responder
  startServer();               // Start a HTTP server with a file read handler and an upload handler

  init_air_serial(&air_status);// Start air conditioning structure and software serial
  air_status.ip = WiFi.localIP().toString();

  startReadSerial();           // Start timer for serial readings (1s)
  startStatus();               // Start timer for status print (10s)
  startTime();                 // Get time from NTP server
  startTemperature();          // Start timer for temperature readings (120s)

  #ifdef USE_MQTT
  if (WiFi.status() == WL_CONNECTED) {
    loadMQTTConfigFromFile();
    startMQTT();
    startMQTTTimer();
  } else {
    print_log("[MQTT] WiFi not connected");
  }
  #endif //USE_MQTT

  #ifdef USE_TELEGRAM
    startTelegram();
  #endif

  if (simulation_mode) {
    initSimulationMode();
  }


  Serial.println();
  Serial.println("=== ESP8266 Flash Info ===");
  Serial.printf("Flash Chip ID: %08X\n", ESP.getFlashChipId());
  Serial.printf("Flash Size: %u bytes (%.1f MB)\n", ESP.getFlashChipRealSize(), ESP.getFlashChipRealSize() / 1024.0 / 1024.0);
  Serial.printf("Sketch Size: %u bytes\n", ESP.getSketchSize());
  Serial.printf("Free Sketch Space: %u bytes\n", ESP.getFreeSketchSpace());
  Serial.printf("Filesystem Size: %u bytes\n", ESP.getFlashChipSize() - ESP.getSketchSize());

}

// start functions

void startTime() {
  long int boot_time;

  timeClient.begin();                        // Get NTP time
  //timeClient.update();
  for (int i = 0; i < 5; i++) {
    timeClient.update();                     // update time
    boot_time = timeClient.getEpochTime();
    if (boot_time > 3600) break;             // check we are not still in 1970
  }
  air_status.boot_time = boot_time;
}

// Structure to hold environmental sensor readings
struct env_snapshot {
  float temp;
  float humidity;
  float pressure;
  bool temp_valid;
  bool humidity_valid;
  bool pressure_valid;
  String temp_source;
  String humidity_source;
  String pressure_source;
};

// Common sensor reading function - reads all available environmental sensors
// and returns the best available readings with source information
env_snapshot readEnvironmentalSensors(const char* context = "") {
  env_snapshot snapshot = {
    .temp = -999.0,
    .humidity = -999.0,
    .pressure = -999.0,
    .temp_valid = false,
    .humidity_valid = false,
    .pressure_valid = false,
    .temp_source = "None",
    .humidity_source = "None",
    .pressure_source = "None"
  };

#ifdef USE_AHT20
  if (humidity_status) {
    sensors_event_t humidity, temp;
    if (aht.getEvent(&humidity, &temp)) {
      aht_t_current = temp.temperature;
      sensor_humidity_current = humidity.relative_humidity;
      
      // AHT20 is preferred for both temperature and humidity
      snapshot.temp = aht_t_current;
      snapshot.humidity = sensor_humidity_current;
      snapshot.temp_valid = true;
      snapshot.humidity_valid = true;
      snapshot.temp_source = "AHT20";
      snapshot.humidity_source = "AHT20";
      
      if (strlen(context) > 0) {
        print_logf("[AHT20][%s] temp %.1f hum %.1f", context, aht_t_current, sensor_humidity_current);
      }
    } else {
      print_logf("[AHT20][%s] Failed to read sensor", context);
      aht_t_current = -999.0;
      sensor_humidity_current = -999.0;
    }
  }
  yield();
#endif

#ifdef USE_BMP280
  if (pressure_status) {
    bmp280_t_current = bmp280.readTemperature();
    sensor_pressure_current = bmp280.readPressure() / 100.0F;
    
    // Use BMP280 temperature only if AHT20 failed or not available
    if (!snapshot.temp_valid && bmp280_t_current > -999.0) {
      snapshot.temp = bmp280_t_current;
      snapshot.temp_valid = true;
      snapshot.temp_source = "BMP280";
    }
    
    // BMP280 is primary pressure source
    if (sensor_pressure_current > -999.0) {
      snapshot.pressure = sensor_pressure_current;
      snapshot.pressure_valid = true;
      snapshot.pressure_source = "BMP280";
    }
    
    if (strlen(context) > 0) {
      print_logf("[BMP280][%s] temp %.1f press %.1f hPa", context, bmp280_t_current, sensor_pressure_current);
    }
  }
  yield();
#endif

  return snapshot;
}


void startTemperature() {
  my_timer_set_unit(&timerTemperature, 1000); // 1000ms
  my_timer_set_interval(&timerTemperature, temp_interval); //update every XX s
  my_timer_repeat(&timerTemperature);
  my_timer_start(&timerTemperature); 

  // Clear centralized arrays
  memset(ac_sensor_temperature, 0, MAX_LOG_DATA * sizeof(float));
  memset(ac_outdoor_te, 0, MAX_LOG_DATA * sizeof(int));
  memset(sensor_temperature, 0, MAX_LOG_DATA * sizeof(float));
  memset(sensor_humidity, 0, MAX_LOG_DATA * sizeof(float));
  memset(sensor_pressure, 0, MAX_LOG_DATA * sizeof(float));
  memset(timestamps, 0, MAX_LOG_DATA * sizeof(unsigned long));

  // Initialize sensors
#ifdef USE_AHT20
  if (!aht.begin()) {
    print_log("[AHT20] Could not find a valid sensor, check wiring!");
    humidity_status = false;
  } else {
    humidity_status = true;
  }
#endif

#ifdef USE_BMP280
  if (!bmp280.begin()) {
    print_log("[BMP280] Could not find a valid sensor, check wiring!");
    pressure_status = false;
  } else {
    pressure_status = true;
    bmp280.setSampling(Adafruit_BMP280::MODE_NORMAL,
                      Adafruit_BMP280::SAMPLING_X2,
                      Adafruit_BMP280::SAMPLING_X16,
                      Adafruit_BMP280::FILTER_X16,
                      Adafruit_BMP280::STANDBY_MS_500);
  }
#endif

  // Get initial sensor readings
  env_snapshot snapshot = readEnvironmentalSensors("INIT");

  // Set centralized readings with AC fallback for temperature
  sensor_temperature[0] = snapshot.temp_valid ? snapshot.temp : air_status.remote_sensor_temp;
  sensor_humidity[0] = snapshot.humidity_valid ? snapshot.humidity : -999.0;
  sensor_pressure[0] = snapshot.pressure_valid ? snapshot.pressure : -999.0;

  sensor_temperature_current = sensor_temperature[0];
  sensor_humidity_current = sensor_humidity[0];
  sensor_pressure_current = sensor_pressure[0];

  print_logf("[SENSOR] Temperature source: %s (%.1f°C)", 
             snapshot.temp_valid ? snapshot.temp_source.c_str() : "AC fallback", 
             sensor_temperature[0]);
  
  if (snapshot.humidity_valid) {
    print_logf("[SENSOR] Humidity source: %s (%.1f%%)", snapshot.humidity_source.c_str(), sensor_humidity[0]);
  }
  
  if (snapshot.pressure_valid) {
    print_logf("[SENSOR] Pressure source: %s (%.1f hPa)", snapshot.pressure_source.c_str(), sensor_pressure[0]);
  }

  // Query AC and log first sample
  air_send_ping(&air_status);
  air_parse_serial(&air_status);
  ac_sensor_temperature[0] = air_status.remote_sensor_temp;

  air_query_sensor(&air_status, OUTDOOR_TE);
  air_parse_serial(&air_status);
  ac_outdoor_te[0] = air_status.outdoor_te;

  timeClient.update();
  timestamps[0] = timeClient.getEpochTime();

  temp_idx = (temp_idx + 1) % MAX_LOG_DATA;
}

void startStatus() {
  my_timer_set_unit(&timerStatus, 1000); // 1000ms
  my_timer_set_interval(&timerStatus, 120); //update every XX s
  my_timer_repeat(&timerStatus);
  my_timer_start(&timerStatus);

  air_query_sensors(&air_status, sensor_ids, sizeof(sensor_ids)); //query sensors on start
}

void startReadSerial() {
  my_timer_set_unit(&timerReadSerial, 1000); // 1000ms
  my_timer_set_interval(&timerReadSerial, 1); //update every 1 s
  my_timer_repeat(&timerReadSerial);
  my_timer_start(&timerReadSerial);
}

//save status every hour
void startSaveFile() {
  my_timer_set_unit(&timerSaveFile, 1000); // 1000ms
  my_timer_set_interval(&timerSaveFile, 60); //update every 60 s
  my_timer_repeat(&timerSaveFile);
  my_timer_start(&timerSaveFile);  
}

void startAutonomous() {
  my_timer_set_unit(&timerAutonomous, 1000); // 1000ms
  my_timer_set_interval(&timerAutonomous, 8); //update every 8 s
  my_timer_repeat(&timerAutonomous);
  my_timer_start(&timerAutonomous);  
}

//software air conditioning timer
void handleTimerOnOff() {
  //if (timerAC.isTime()) {
  if (my_timer_is_time(&timerOnOff)) {
    Serial.println("[HANDLE] handleTimerOnOff");
    if (air_status.timer_mode_req == TIMER_SW_OFF) {
      //set power off
      print_log("[TIMER_SW] Power OFF");
      if (simulation_mode) {
        air_status.power = false;
      } else {
        air_set_power_off(&air_status);
      }
    } else if (air_status.timer_mode_req == TIMER_SW_ON) {
      //set power on
      print_log("[TIMER_SW] Power ON");
      if (simulation_mode) {
        air_status.power = true;
      } else {
        air_set_power_on(&air_status);
      }
    }
    yield();
  }
}

//for temp logging every temp_interval (30 mins by default)
void handleTemperature() {
  if (my_timer_is_time(&timerTemperature)) {
    Serial.println("[HANDLE] handleTemperature");
    if (!simulation_mode) {
      air_send_ping(&air_status);
    }

    // Read environmental sensors
    env_snapshot snapshot = readEnvironmentalSensors("LOG");

    ac_sensor_temperature[temp_idx] = air_status.remote_sensor_temp;

    // Store centralized readings with AC fallback for temperature
    sensor_temperature[temp_idx] = snapshot.temp_valid ? snapshot.temp : air_status.remote_sensor_temp;
    sensor_humidity[temp_idx] = snapshot.humidity_valid ? snapshot.humidity : -999.0;
    sensor_pressure[temp_idx] = snapshot.pressure_valid ? snapshot.pressure : -999.0;

    if (!simulation_mode) {
      air_query_sensor(&air_status, OUTDOOR_TE);
      air_parse_serial(&air_status);
    }

    ac_outdoor_te[temp_idx] = air_status.outdoor_te;

    timeClient.update();
    timestamps[temp_idx] = timeClient.getEpochTime();
    temp_idx = (temp_idx + 1) % MAX_LOG_DATA;

    #ifdef USE_SCREEN
    showDisplay();
    #endif
    yield();
  }
}

void handleSaveFile() {
  //if (timerSaveFile.isTime()) {
  if (my_timer_is_time(&timerSaveFile)) {
    Serial.println("[HANDLE] handleSaveFile");
    String txt = air_to_json(&air_status);
    save_file("/status.json", txt);
    print_log("Saving status.json");
  }
}

//used for status not for logging
void getTemperatureCurrent() {
  // Read environmental sensors
  env_snapshot snapshot = readEnvironmentalSensors("STATUS");

  // Update centralized current readings with AC fallback for temperature
  sensor_temperature_current = snapshot.temp_valid ? snapshot.temp : air_status.remote_sensor_temp;
  sensor_humidity_current = snapshot.humidity_valid ? snapshot.humidity : -999.0;
  sensor_pressure_current = snapshot.pressure_valid ? snapshot.pressure : -999.0;

  // Request outdoor temp (parsed elsewhere)
  air_query_sensor(&air_status, OUTDOOR_TE);
}

//get current status, temperature and sensors. not intended for logging
//mainly use for web interface status request
void handleStatus() {
  if (my_timer_is_time(&timerStatus)) {
    Serial.println("[HANDLE] handleStatus");
    getTemperatureCurrent();
    yield(); // Allow ESP8266 to handle WiFi stack
        
    if (simulation_mode) {
      // In test mode, skip querying sensors from AC unit
      Serial.println("[SIMULATION MODE] Skipping AC sensor queries");
    } else {
      air_query_sensors(&air_status, sensor_ids, sizeof(sensor_ids));
      yield();
    }

    air_print_status(&air_status);
    yield();
  }
}

void handleReadSerial() {
  if (my_timer_is_time(&timerReadSerial)) {
    Serial.println("[HANDLE] handleReadSerial");
    if (simulation_mode) {
      runSimulationMode(); // Simulate AC behavior in test mode
    } else {
      air_parse_serial(&air_status); // Normal AC operation
    }
    yield();
  }
}

void handleAutonomousMode() {
  if (my_timer_is_time(&timerAutonomous) && autonomous_mode) {
    Serial.println("[HANDLE] handleAutonomousMode");
    static bool send_ping = true;
    
    if (send_ping) {
      // Send ping message
      air_send_ping(&air_status);
      print_log(F("[AUTONOMOUS] Ping sent"));
    } else {
      // Send temperature message using best available external sensor
      float temp_to_send = air_status.remote_sensor_temp; // Default fallback
      String sensor_used = "AC sensor";
      
      #ifdef USE_AHT20
      // Prioritize AHT20 (most accurate)
      if (aht_t_current > -999.0) {
        temp_to_send = aht_t_current;
        sensor_used = "AHT20";
      } else
      #endif
      #ifdef USE_BMP280
      // Then BMP280
      if (bmp280_t_current > -999.0) {
        temp_to_send = bmp280_t_current;
        sensor_used = "BMP280";
      } else
      #endif
      {
        sensor_used = "AC sensor (fallback)";
      }
      
      air_send_remote_temp(&air_status, temp_to_send);
      print_logf("[AUTONOMOUS] Temperature sent: %.1f°C from %s", temp_to_send, sensor_used.c_str());
    }
    
    // Alternate between ping and temperature
    send_ping = !send_ping;
    yield();
  }
}

/*__________________________________________________________LOOP__________________________________________________________*/

void loop() {
  handleTimerOnOff();//yield(); //now yield is done inside the handlers
  handleTemperature();//yield();
  handleStatus();//yield();
  handleReadSerial();//yield();
  #ifdef USE_MQTT
  handleMQTT();//yield();
  #endif

  #ifdef USE_ASYNC
  // AsyncWebSocket is handled automatically by the server
  #else
  webSocket.loop();//yield();
  #endif

  server.handleClient();//yield();
  #ifdef USE_OTA
  ArduinoOTA.handle();//yield();
  #endif

  MDNS.update(); yield();
  
  #ifdef USE_SCREEN
  showDisplay();//yield();
  #endif
  
  handleSaveFile();yield();
  handleAutonomousMode();//yield();

  #ifdef USE_TELEGRAM
  handleTelegramMessages();//yield();
  #endif
}

/*__________________________________________________________START_FUNCTIONS__________________________________________________________*/

void startWiFi() { //fixed IP
  WiFi.mode(WIFI_STA);
  WiFi.config(ip, gateway, subnet, dns);
  WiFi.begin(w_ssid, w_passwd);
  Serial.print("Connected to:\t");
  Serial.println(w_ssid);

  // wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(200);
    Serial.print('.');
  }

  // show IP
  Serial.println("[WIFI] Connection stablished.");
  Serial.print("[WIFI] IP address:\t");
  Serial.println(WiFi.localIP());
}

#ifdef USE_OTA
void startOTA() { // Start the OTA service
  ArduinoOTA.setHostname(OTAName);
  ArduinoOTA.setPassword(OTAPassword);

  ArduinoOTA.onStart([]() {
    Serial.println("[OTA] Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\r\n[OTA] End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("[OTA] Ready");
}
#endif

void startLittleFS() { // Start the LittleFS and list all contents
  LittleFS.begin();                             // Start the SPI Flash File System (LittleFS)
  Serial.println("[LITTLEFS] Started. Contents:");
  {
    Dir dir = LittleFS.openDir("/");
    while (dir.next()) {                      // List the file system contents
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("\tFS File: %s, size: %s\r\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    Serial.printf("\n");
  }
}

void startWebSocket() { // Start a WebSocket server
#ifdef USE_ASYNC
  webSocket.onEvent(onWsAsyncEvent);
  server.addHandler(&webSocket);
  Serial.println("AsyncWebSocket server started.");
#else //use links2004
  webSocket.begin();                          // start the websocket server
  webSocket.onEvent(onWsEvent);          // if there's an incomming websocket message, go to function 'webSocketEvent'
  webSocket.enableHeartbeat(3000, 2000, 2); // short time (3000) to allow fast reception
  Serial.println("[WEBSOCKET] Server started.");
#endif
}

void stopWebSocket() {
#ifdef USE_ASYNC
  webSocket.closeAll();  // Close all client connections
  Serial.println("AsyncWebSocket handler removed.");
#else
  // For links2004 WebSocket server
  webSocket.disconnect();  // Disconnect all clients
  webSocket.close();       // Close the WebSocket server
  Serial.println("WebSocket server stopped.");
#endif
}

void startMDNS() { // Start the mDNS responder
  MDNS.begin(mdnsName);                        // start the multicast domain name server
  Serial.print("[MDNS] Responder started: http://");
  Serial.print(mdnsName);
  Serial.println(".local");
}

void startServer() { // Start a HTTP server with a file read handler and an upload handler
#ifdef USE_ASYNC
  Serial.print("Using Async");
  server.on("/edit.html",  HTTP_POST, [](AsyncWebServerRequest * request) { // If a POST request is sent to the /edit.html address,
    server.send(200, "text/plain", "");
  }, handleFileUpload);                       // go to 'handleFileUpload'

  server.onNotFound(handleNotFound);          // if someone requests any other file or page, go to function 'handleNotFound'
  // and check if the file exists

#else
  server.on("/edit.html",  HTTP_POST, []() {  // If a POST request is sent to the /edit.html address,
    server.send(200, "text/plain", "");
  }, handleFileUpload);                       // go to 'handleFileUpload'

  // Serve built-in file manager (replaces separate upload and delete pages)
  server.on("/upload", HTTP_GET, []() {
        server.send_P(200, "text/html", file_manager_html);
  });

  server.on("/upload", HTTP_POST, []() {  
    server.send(200, "text/plain", "Upload successful!");
  }, handleFileUpload);                       

  // Built-in file manager (same as upload for unified interface)
  server.on("/delete", HTTP_GET, []() {
    server.send_P(200, "text/html", file_manager_html);
  });

  // File manager endpoint (same as upload for unified interface)
  server.on("/filemanager", HTTP_GET, []() {
    server.send_P(200, "text/html", file_manager_html);
  });

  // API endpoint to list files
  server.on("/api/files", HTTP_GET, []() {
    JsonDocument doc;
    JsonArray files = doc["files"].to<JsonArray>();
    
    Dir dir = LittleFS.openDir("/");
    while (dir.next()) {
      JsonObject file = files.add<JsonObject>();
      file["name"] = dir.fileName();
      file["size"] = dir.fileSize();
    }
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  });

  // API endpoint to delete files
  server.on("/api/delete", HTTP_POST, []() {
    String body = server.arg("plain");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, body);
    
    if (!error && doc["filename"].is<const char*>()) {
      String filename = doc["filename"];
      
      // Add leading slash if not present
      if (!filename.startsWith("/")) {
        filename = "/" + filename;
      }
      
      // Prevent deletion of critical files
      // if (filename == "/index.html") {
      //   server.send(200, "application/json", "{\"success\":false,\"error\":\"Cannot delete critical system files\"}");
      //   return;
      // }
      
      if (LittleFS.exists(filename)) {
        if (LittleFS.remove(filename)) {
          print_logf("[DELETE] File deleted: %s", filename.c_str());
          server.send(200, "application/json", "{\"success\":true}");
        } else {
          server.send(200, "application/json", "{\"success\":false,\"error\":\"Failed to delete file\"}");
        }
      } else {
        server.send(200, "application/json", "{\"success\":false,\"error\":\"File not found\"}");
      }
    } else {
      server.send(200, "application/json", "{\"success\":false,\"error\":\"Invalid request format\"}");
    }
  });

  server.onNotFound(handleNotFound);          // if someone requests any other file or page, go to function 'handleNotFound'
  // and check if the file exists

#endif

  server.begin();                             // start the HTTP server
  Serial.println("[HTTP] Server started.");
}

void startWifiManager() {
  //set led pin as output
  pinMode(LED_BUILTIN, OUTPUT);
  // start ticker with long 0.6 pulses, we are in AP mode and try to connect
  ticker.attach(0.6, tick);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wm;
  //reset settings - for testing
  //wm.resetSettings();
  //wm.setSTAStaticIPConfig(ip, gateway, subnet);

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wm.setAPCallback(configModeCallback);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //and goes into a blocking loop awaiting configuration
  if (!wm.autoConnect("airAP")) {
    Serial.println("[WIFI] Failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(1000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("[WIFI] Connected :)");
  ticker.detach();
  //keep LED off
  digitalWrite(LED, HIGH);
}


//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("[WIFI] Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}

/*__________________________________________________________SERVER_HANDLERS__________________________________________________________*/

void handleNotFound() { // if the requested file or page doesn't exist, return a 404 not found error
  if (!handleFileRead(server.uri())) {        // check if the file exists in the flash memory (LittleFS), if so, send it
    server.send(404, "text/plain", "404: File Not Found: " + server.uri());
  }
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  bool ret;

  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";          // If a folder is requested, send the index file
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if (LittleFS.exists(pathWithGz) || LittleFS.exists(path)) { // If the file exists, either as a compressed archive, or normal
    if (LittleFS.exists(pathWithGz))                         // If there's a compressed version available
      path += ".gz";                                         // Use the compressed verion
    File file = LittleFS.open(path, "r");                    // Open the file
    if (!file) {
      Serial.println("[FILE] Open failed" + path);
      ret = false;
    } else {
      size_t sent = server.streamFile(file, contentType);    // Send it to the client
      (void)sent; // not to be used, but prevents compiler warning
      file.close();                                          // Close the file again
      Serial.println(String("[FILE] Sent file: ") + path);
      ret = true;
    }
  } else
    Serial.println(String("[FILE] File Not Found: ") + path);   // If the file doesn't exist, return false
  ret = false;

  return ret;
}

void handleFileUpload() { // upload a new file to the LittleFS
  HTTPUpload& upload = server.upload();
  String path;
  if (upload.status == UPLOAD_FILE_START) {
    path = upload.filename;
    if (!path.startsWith("/")) path = "/" + path;
    if (!path.endsWith(".gz")) {                         // The file server always prefers a compressed version of a file
      String pathWithGz = path + ".gz";                  // So if an uploaded file is not compressed, the existing compressed
      if (LittleFS.exists(pathWithGz))                     // version of that file must be deleted (if it exists)
        LittleFS.remove(pathWithGz);
    }
    Serial.print("[FILE] handleFileUpload Name: "); Serial.println(path);
    fsUploadFile = LittleFS.open(path, "w");            // Open the file for writing in LittleFS (create if it doesn't exist)
    path = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {                                   // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      Serial.print("[FILE] handleFileUpload Size: "); Serial.println(upload.totalSize);
      server.sendHeader("Location", "/filemanager?upload=success");     // Redirect back to file manager with success
      server.send(303);
    } else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
}

// websocket callback
#ifdef USE_ASYNC
//void onWsAsyncEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  //to be completed 
//}
#else
void onWsEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:             // if the websocket is disconnected
      Serial.printf("[WS (%u)] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {              // if a new websocket connection is established
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[WS (%u)] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      }
      break;
    case WStype_TEXT:                     // if new text data is received
      Serial.printf("[WS (%u)] Length %d Received: %s\n", num, length, payload);
      processRequest(payload); //TODO: might be better to set a var here call processRequest in loop
      break;
    case WStype_BIN:
    case WStype_PING:
    case WStype_PONG:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
      // No action needed for these cases
      break;
    default:
      // Optionally log unknown types
      Serial.printf("[WS (%u)] Unknown WebSocket event type: %d\n", num, type);
      break;
  }
}
#endif

int save_file(String name, String text) {
  File file = LittleFS.open(name, "w");

  if (!file) {
    Serial.println("[FILE] Error opening file for writing");
    return (0);
  }

  int bytes = file.print(text);
  file.close();

  return (bytes);
}


// Function to initialize simulagtion mode air status
void initSimulationMode() {
  if (!simulation_mode) return;
  
  Serial.println("[SIMULATION MODE] Initializing air conditioning simulator");
  
  // Initialize air_status with realistic test values
  air_status.power = true;
  air_status.mode = MODE_COOL;
  strcpy(air_status.mode_str,"COOL"); 
  air_status.target_temp = 22;
  air_status.fan = FAN_AUTO;
  strcpy(air_status.fan_str,"AUTO");
  air_status.remote_sensor_temp = 23.5;
  air_status.outdoor_te = 25.0;
  air_status.outdoor_to = 26.0;
  air_status.indoor_ta = 23.0;
  air_status.indoor_tcj = 22.0;
  air_status.indoor_tc = 21.5;
  air_status.outdoor_current = 0.0;
  air_status.outdoor_comp_freq = 0;
  air_status.timer_mode_req = TIMER_SW_OFF;
  air_status.timer_time_req = 0;    
  air_status.save = false;
  air_status.heat = false;
  air_status.preheat = false;
  air_status.cold = false;
}

void runSimulationMode() {
  if (!simulation_mode) return;
  
  // Simulate temperature changes based on AC operation
  static unsigned long lastTempUpdate = 0;
  unsigned long currentTime = millis();
  
  if (currentTime - lastTempUpdate > 30000) { // Update every 30 seconds
    if (air_status.power) {
      float targetTemp = air_status.target_temp;
      float currentTemp = air_status.remote_sensor_temp;
      float tempDiff = targetTemp - currentTemp;
      
      // Simulate gradual temperature change
      if (abs(tempDiff) > 0.1) {
        if (tempDiff > 0) {
          air_status.remote_sensor_temp += 0.2; // Heating
          air_status.outdoor_current = 2.5;
        } else {
          air_status.remote_sensor_temp -= 0.2; // Cooling
          air_status.outdoor_current = 3.2;
        }
      } else {
        air_status.outdoor_current = 1.0; // Standby power
      }
      
      // Simulate fan RPM based on fan setting and temperature difference
      int baseFanRpm = 0;
      switch (air_status.fan) {
        case FAN_LOW:
          baseFanRpm = 300;
          break;
        case FAN_MEDIUM:
          baseFanRpm = 500;
          break;
        case FAN_HIGH:
          baseFanRpm = 800;
          break;
        case FAN_AUTO:
          // Auto mode adjusts based on temperature difference
          if (abs(tempDiff) > 2.0) {
            baseFanRpm = 700; // High speed when big temp difference
          } else if (abs(tempDiff) > 1.0) {
            baseFanRpm = 450; // Medium speed for moderate difference
          } else {
            baseFanRpm = 250; // Low speed when close to target
          }
          break;
        default:
          baseFanRpm = 400;
          break;
      }
      
      // Add some variation to make it more realistic (±10%)
      int variation = random(-baseFanRpm/10, baseFanRpm/10);
      air_status.indoor_fan_speed = baseFanRpm + variation;
      
      // Ensure minimum speed when AC is running
      if (air_status.indoor_fan_speed < 200) {
        air_status.indoor_fan_speed = 200;
      }
      
      // Update other temperatures
      air_status.indoor_ta = air_status.remote_sensor_temp - 0.5;
      air_status.indoor_tcj = air_status.target_temp - 1.0;
      air_status.indoor_tc = air_status.target_temp - 1.5;
      
    } else {
      air_status.outdoor_current = 0.0; // AC is off
      air_status.indoor_fan_speed = 0;  // Fan stopped when AC is off
    }
    
    lastTempUpdate = currentTime;
  }
}
/*__________________________________________________________HELPER_FUNCTIONS__________________________________________________________*/

String formatBytes(size_t bytes) { // convert sizes in bytes to KB and MB
  String val;
  if (bytes < 1024) {
    val = String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    val = String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    val = String(bytes / 1024.0 / 1024.0) + "MB";
  }

  return val;
}

String getContentType(String filename) { // determine the filetype of a given filename, based on the extension
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

void tick()
{
  digitalWrite(LED, !digitalRead(LED));     // toggle state
}


////////////////////////////////////////////

void webSocketBroadcast(String message) {
#ifdef USE_ASYNC
    webSocket.textAll(message);
#else
    webSocket.broadcastTXT(message);
#endif
}
