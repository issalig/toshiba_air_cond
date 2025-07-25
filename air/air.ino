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
                AHT20 + BMP280 connected to D1 (SCL) D2 (SDA) (optional)

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
#include "upload_html.h"


#include "LittleFS.h"
#include <ESP8266WiFi.h>

#ifdef USE_ASYNC //for ESP32
#include <ESPAsync_WiFiManager.h_>
#include <ESPAsyncTCP.h_>
#include <ESPAsyncWebServer.h_>
#else //use links2004 websocketserver
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h> //https://github.com/Links2004/arduinoWebSockets/
#endif

#include <SPI.h>

#ifdef USE_SCREEN
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#endif

#include <ArduinoOTA.h>
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
MySimpleTimer timerOnOff;

#ifdef USE_ASYNC
AsyncWebServer server(80);
AsyncWebSocket webSocket("/ws");
#else
ESP8266WebServer server(80);       // create a web server on port 80
WebSocketsServer webSocket(81);    // create a websocket server on port 81
#endif

File fsUploadFile;                                    // a File variable to temporarily store the received file


// Add these includes after your existing sensor includes
#ifdef USE_AHT20
#include <Adafruit_AHTX0.h>
Adafruit_AHTX0 aht;
#endif

#ifdef USE_BMP280
#include <Adafruit_BMP280.h>
Adafruit_BMP280 bmp280;
#endif

// Add current sensor reading variables
#ifdef USE_AHT20
float aht_t_current = -999.0;
#endif

#ifdef USE_BMP280
float bmp280_t_current = -999.0;
#endif

// Add sensor status variables
#ifdef USE_AHT20
bool humidity_status = false;
#endif

#ifdef USE_BMP280
bool pressure_status = false;
#endif

uint8_t bmp_status = 0;

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
MySimpleTimer timerTemperature;
MySimpleTimer timerStatus;
MySimpleTimer timerReadSerial;
MySimpleTimer timerSaveFile;
MySimpleTimer timerMQTT; 
MySimpleTimer timerAutonomous;
bool autonomous_mode = false;

int temp_interval = 1800; //in secs, 30 mins

#define RESET_MODE_PIN D4  //button to enter into wifi configuration

bool mqtt_enabled = true;




/*__________________________________________________________SETUP__________________________________________________________*/

void setup() {

  my_timer_init(&timerOnOff);
  my_timer_init(&timerTemperature);
  my_timer_init(&timerStatus);
  my_timer_init(&timerReadSerial);
  my_timer_init(&timerSaveFile);
  my_timer_init(&timerMQTT);
  my_timer_init(&timerAutonomous);

  Serial.begin(9600);        // Start the Serial communication to send messages to the computer
  //  gdbstub_init();
  delay(10);
  Serial.println("\r\n");
  print_log("Air conditioning starts!");
  #ifdef USE_SCREEN
  startDisplay();
  #endif  
  //startWiFi();                 // Start a Wi-Fi access point, and try to connect to some given access points. Then wait for either an AP or STA connection
  startWifiManager();          // Use wifimanager to connect
  //is_reset_button();
  startOTA();                  // Start the OTA service
  startLittleFS();               // Start the LittleFS and list all contents
  startWebSocket();            // Start a WebSocket server
  startMDNS();                 // Start the mDNS responder
  startServer();               // Start a HTTP server with a file read handler and an upload handler

  init_air_serial(&air_status);// Start air conditioning structure and software serial

  air_status.ip = WiFi.localIP().toString();
  #ifdef USE_SCREEN
  //showIPDisplay();
  #endif

  startReadSerial();           // Start timer for serial readings (1s)
  startStatus();               // Start timer for status print (10s)
  startTime();                 // Get time from NTP server
  startTemperature();          // Start timer for temperature readings (120s)

  #ifdef USE_MQTT
  if (mqtt_enabled && WiFi.status() == WL_CONNECTED) {
    loadMQTTConfigFromFile();
    startMQTT();
    startMQTTTimer();
  } else {
    print_log("MQTT disabled or WiFi not connected");
  }
  #endif //USE_MQTT
}

#ifdef USE_SCREEN
// https://javl.github.io/image2cpp/
// 48x48px
const unsigned char ac_icon_bitmap [] PROGMEM = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfe, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x3f, 0xff, 
0xff, 0xff, 0xff, 0xc0, 0x07, 0xff, 0xff, 0xff, 0xff, 0x0c, 0xe1, 0xff, 0x80, 0x0f, 0xfe, 0x60, 
0x78, 0xff, 0x80, 0x0f, 0xfc, 0xc0, 0x3e, 0x7f, 0x80, 0x0f, 0xf9, 0x80, 0x3f, 0x3f, 0x80, 0x0f, 
0xf3, 0x80, 0x7f, 0x9f, 0x80, 0x0f, 0xe7, 0x80, 0x7f, 0x9f, 0x80, 0x0f, 0xe7, 0xc0, 0x7f, 0xcf, 
0x80, 0x0f, 0xef, 0xc1, 0xff, 0xcf, 0x80, 0x0f, 0xcf, 0xe4, 0x7f, 0xef, 0x80, 0x0f, 0xcf, 0xf8, 
0x3f, 0xe7, 0x80, 0x0f, 0xcf, 0xf0, 0x31, 0xe7, 0x80, 0x0f, 0xcf, 0xf0, 0x00, 0x67, 0x80, 0x0f, 
0xcf, 0xf0, 0x20, 0x27, 0x80, 0x0f, 0xcc, 0x08, 0x20, 0x27, 0x80, 0x0f, 0xc8, 0x04, 0xc0, 0x2f, 
0x80, 0x0f, 0xe8, 0x03, 0xc0, 0x4f, 0x80, 0x0f, 0xe0, 0x03, 0xe0, 0x4f, 0x80, 0x0f, 0xe4, 0x03, 
0xe0, 0x1f, 0x80, 0x0f, 0xf2, 0x07, 0xf0, 0x9f, 0x80, 0x0f, 0xf9, 0x07, 0xf9, 0x3f, 0x80, 0x0f, 
0xfc, 0x8f, 0xfe, 0x7f, 0x80, 0x0f, 0xfe, 0x7f, 0xf8, 0xff, 0x80, 0x0f, 0xff, 0x0f, 0xe1, 0xff, 
0x80, 0x0f, 0xff, 0x80, 0x07, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfe, 
0x3f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x01, 0xf0, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void startDisplay() {
  // Clear the buffer
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    print_log(F("SSD1306 allocation failed"));
  else {
    showSplashScreen();
  }
}

void showSplashScreen() {
  display.clearDisplay();

  // Draw the 48x48 bitmap centered on screen
  // 128 - 48 = 80/2 = 40 for proper horizontal centering
  display.drawBitmap(40, 0, ac_icon_bitmap, 48, 48, SSD1306_WHITE);
  
  // Add title text below or overlay
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(34, 50);
  display.println(F("Starting..."));
  
  display.display();

  //delay(1000);
}

void showIPDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Connected!"));
  
  display.setCursor(0, 20);
  display.println(F("IP Address:"));
  display.setCursor(0, 35);
  display.println(air_status.ip);
  
  display.setCursor(0, 50);
  display.println(F("Ready to control"));
  
  display.display();
  delay(2000); // Show IP for 2 seconds before normal operation
}


#endif

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

void startTemperature() {
  /*
  timerTemperature.setUnit(1000);
  timerTemperature.setInterval(temp_interval);
  timerTemperature.repeat();
  timerTemperature.start();
  */

  //new c api for timer
  my_timer_set_unit(&timerTemperature, 1000); // 1000ms
  my_timer_set_interval(&timerTemperature, temp_interval); //update every XX s
  my_timer_repeat(&timerTemperature);
  my_timer_start(&timerTemperature); 


  // Clear centralized arrays
  memset(ac_sensor_temperature, 0, MAX_LOG_DATA * sizeof(float));
  memset(ac_outdoor_te, 0, MAX_LOG_DATA * sizeof(float));
  memset(sensor_temperature, 0, MAX_LOG_DATA * sizeof(float));
  memset(sensor_humidity, 0, MAX_LOG_DATA * sizeof(float));
  memset(sensor_pressure, 0, MAX_LOG_DATA * sizeof(float));
  memset(timestamps, 0, MAX_LOG_DATA * sizeof(unsigned long));

  // Initialize sensors and get initial readings
  float initial_temp = -999.0;
  float initial_humidity = -999.0;
  float initial_pressure = -999.0;
  String temp_source = "None";
  String humidity_source = "None";
  String pressure_source = "None";

#ifdef USE_AHT20
  if (!aht.begin()) {
    print_log("Could not find a valid AHT20 sensor, check wiring!\n");
    humidity_status = false;
  } else {
    humidity_status = true;
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);
    aht_t_current = temp.temperature;
    sensor_humidity_current = humidity.relative_humidity;
    
    if (initial_temp <= -999.0) {
      initial_temp = aht_t_current;
      temp_source = "AHT20";
    }
    if (initial_humidity <= -999.0) {
      initial_humidity = sensor_humidity_current;
      humidity_source = "AHT20";
    }
    
    print_logf("AHT20 initialized - temp %.1f hum %.1f\n", aht_t_current, sensor_humidity_current);
  }
#endif

#ifdef USE_BMP280
  if (!bmp280.begin()) {
    print_log("Could not find a valid BMP280 sensor, check wiring!\n");
    pressure_status = false;
  } else {
    pressure_status = true;
    bmp280.setSampling(Adafruit_BMP280::MODE_NORMAL,
                      Adafruit_BMP280::SAMPLING_X2,
                      Adafruit_BMP280::SAMPLING_X16,
                      Adafruit_BMP280::FILTER_X16,
                      Adafruit_BMP280::STANDBY_MS_500);
    
    bmp280_t_current = bmp280.readTemperature();
    sensor_pressure_current = bmp280.readPressure() / 100.0F;
    
    // Use BMP280 temperature if no other sensor available
    if (initial_temp <= -999.0) {
      initial_temp = bmp280_t_current;
      temp_source = "BMP280";
    }
    // BMP280 is primary pressure source
    initial_pressure = sensor_pressure_current;
    pressure_source = "BMP280";
    
    print_logf("BMP280 initialized - temp %.1f press %.1f hPa\n", bmp280_t_current, sensor_pressure_current);
  }
#endif



  // Set centralized readings
  if (initial_temp > -999.0) {
    sensor_temperature[0] = initial_temp;
    sensor_temperature_current = initial_temp;
    print_logf("Using %s as temperature source: %.1f°C\n", temp_source.c_str(), initial_temp);
  } else {
    sensor_temperature[0] = air_status.remote_sensor_temp;
    sensor_temperature_current = air_status.remote_sensor_temp;
    print_logf("Using AC sensor as temperature fallback: %.1f°C\n", air_status.remote_sensor_temp);
  }

  if (initial_humidity > -999.0) {
    sensor_humidity[0] = initial_humidity;
    sensor_humidity_current = initial_humidity;
    print_logf("Using %s as humidity source: %.1f%%\n", humidity_source.c_str(), initial_humidity);
  } else {
    sensor_humidity[0] = -999.0;
    sensor_humidity_current = -999.0;
  }

  if (initial_pressure > -999.0) {
    sensor_pressure[0] = initial_pressure;
    sensor_pressure_current = initial_pressure;
    print_logf("Using %s as pressure source: %.1f hPa\n", pressure_source.c_str(), initial_pressure);
  } else {
    sensor_pressure[0] = -999.0;
    sensor_pressure_current = -999.0;
  }

////////////////
  air_send_ping(&air_status); // send ping to air conditioning
  ac_sensor_temperature[0] = air_status.remote_sensor_temp; // sensor_temp is room temperature

  air_query_sensor(&air_status, OUTDOOR_TE); // query outdoor temperature sensor
  ac_outdoor_te[0] = air_status.outdoor_te;

  timestamps[0] = 0; //timeClient.getEpochTime();
}

void startStatus() {
  /*
  timerStatus.setUnit(1000); // 1000ms
  timerStatus.setInterval(120); //update every XX s
  timerStatus.repeat();
  timerStatus.start();
  */

  //new c api for timer
  my_timer_set_unit(&timerStatus, 1000); // 1000ms
  my_timer_set_interval(&timerStatus, 120); //update every XX s
  my_timer_repeat(&timerStatus);
  my_timer_start(&timerStatus);

  air_query_sensors(&air_status, sensor_ids, sizeof(sensor_ids)); //query sensors on start
}

void startReadSerial() {
  //timerReadSerial.setUnit(1000);
  //timerReadSerial.setInterval(1);
  //timerReadSerial.repeat();
  //timerReadSerial.start();

  //new c api for timer
  my_timer_set_unit(&timerReadSerial, 1000); // 1000ms
  my_timer_set_interval(&timerReadSerial, 1); //update every 1 s
  my_timer_repeat(&timerReadSerial);
  my_timer_start(&timerReadSerial);
}

//save status every hour
void startSaveFile() {
  // timerSaveFile.setUnit(1000);
  // timerSaveFile.setInterval(60);
  // timerSaveFile.repeat();
  // timerSaveFile.start();

  // new c api for timer
  my_timer_set_unit(&timerSaveFile, 1000); // 1000ms
  my_timer_set_interval(&timerSaveFile, 60); //update every 60 s
  my_timer_repeat(&timerSaveFile);
  my_timer_start(&timerSaveFile);  
}

void startAutonomous() {
  // timerAutonomous.setUnit(1000);
  // timerAutonomous.setInterval(8);
  // timerAutonomous.repeat();
  // timerAutonomous.start();

  // new c api for timer
  my_timer_set_unit(&timerAutonomous, 1000); // 1000ms
  my_timer_set_interval(&timerAutonomous, 8); //update every 8 s
  my_timer_repeat(&timerAutonomous);
  my_timer_start(&timerAutonomous);  
}

//check power each minute
//void startCheckPowerConsumption() {
//  timerCheckPowerConsumption.setUnit(1000);
//  timerCheckPowerConsumption.setInterval(60);
//  timerCheckPowerConsumption.repeat();
//  timerCheckPowerConsumption.start();
//}

//software air conditioning timer
void handleTimer() {
  //if (timerAC.isTime()) {
  if (my_timer_is_time(&timerOnOff)) {
    if (air_status.timer_mode_req == TIMER_SW_OFF) {
      //set power off
      print_log("[TIMER_SW] Power OFF");
      air_set_power_off(&air_status);
    } else if (air_status.timer_mode_req == TIMER_SW_ON) {
      //set power on
      print_log("[TIMER_SW] Power ON");
      air_set_power_on(&air_status);
    }
  }
}

//for temp logging every temp_interval (30 mins by default)
void handleTemperature() {
  if (my_timer_is_time(&timerTemperature)) {
    air_send_ping(&air_status);

    float best_temp = -999.0;
    float best_humidity = -999.0;
    float best_pressure = -999.0;

#ifdef USE_AHT20
    if (humidity_status) {
        sensors_event_t humidity, temp;
        if (aht.getEvent(&humidity, &temp)) {
            aht_t_current = temp.temperature;
            sensor_humidity_current = humidity.relative_humidity;
            
            best_temp = aht_t_current;      // AHT20 is primary temperature source
            best_humidity = sensor_humidity_current;  // AHT20 is primary humidity source
            
            print_logf("AHT20 %d temp %.1f hum %.1f\n", temp_idx, aht_t_current, sensor_humidity_current);
        } else {
            print_log("[AHT20] Failed to read sensor");
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
        
        // Use BMP280 temperature only if AHT20 failed
        if (best_temp <= -999.0) {
            best_temp = bmp280_t_current;
        }
        best_pressure = sensor_pressure_current; // BMP280 is primary pressure source
        
        print_logf("BMP280 %d temp %.1f press %.1f hPa\n", temp_idx, bmp280_t_current, sensor_pressure_current);
    }
    yield();
#endif

    ac_sensor_temperature[temp_idx] = air_status.remote_sensor_temp;

    // Store centralized readings
    sensor_temperature[temp_idx] = (best_temp > -999.0) ? best_temp : air_status.remote_sensor_temp;
    sensor_humidity[temp_idx] = best_humidity;
    sensor_pressure[temp_idx] = best_pressure;

    air_query_sensor(&air_status, OUTDOOR_TE);
    //parse serial
    air_parse_serial(&air_status);

    ac_outdoor_te[temp_idx] = air_status.outdoor_te;

    timeClient.update();
    timestamps[temp_idx] = timeClient.getEpochTime();
    temp_idx = (temp_idx + 1) % MAX_LOG_DATA;

    #ifdef USE_SCREEN
    showDisplay();
    #endif
  }
}

#ifdef USE_SCREEN
void showDisplay(void) {
  static unsigned long lastDisplayUpdate = 0;
  static unsigned long lastInfoRotation = 0;
  static int infoIndex = 0;
  const unsigned long INFO_ROTATION_INTERVAL = 3000; // 3 seconds per info

  if (millis() - lastDisplayUpdate < 1000) return; // Limit display updates
  lastDisplayUpdate = millis();

  // Rotate information every 3 seconds
  if (millis() - lastInfoRotation > INFO_ROTATION_INTERVAL) {
    infoIndex = (infoIndex + 1) % 4; // 4 different info screens
    lastInfoRotation = millis();
  }

  //Turn off display when AC is OFF
  if (air_status.power == 0) {
    display.clearDisplay();
    display.display();
    return;
  }

  display.clearDisplay();

  // Top section - AC Status (lines 0-31)
  if (air_status.power != 0) {
    // Large target temperature
    display.setTextSize(3);
    display.setCursor(3, 3);
    display.setTextColor(SSD1306_WHITE);
    display.printf("%d", air_status.target_temp);
    
    // Mode indicator
    display.setTextSize(2);
    display.setCursor(60, 0);
    display.printf("%s", air_status.mode_str);
    
    // Fan speed
    display.setTextSize(1);
    display.setCursor(60, 16);
    display.printf("%s", air_status.fan_str);
    
    // Current AC sensor temperature
    display.setCursor(90, 16);
    display.printf("%.1f", air_status.remote_sensor_temp);
  } 

  // Separator line
  display.drawLine(0, 31, 127, 31, SSD1306_WHITE);

  // Middle section - Environmental data (lines 32-50)
  int idx = (temp_idx > 0) ? temp_idx - 1 : 0;

  // Room temperature and humidity from centralized arrays
  display.setTextSize(1);
  display.setCursor(0, 34);
  if (sensor_temperature[idx] > -999 && sensor_humidity[idx] > -999) {
    display.printf("Room: %.1f C %.0f%%", sensor_temperature[idx], sensor_humidity[idx]);
  } else if (sensor_temperature[idx] > -999) {
    display.printf("Room: %.1f C --%% ", sensor_temperature[idx]);
  } else {
    display.printf("Room: -- C --%% (No sensor)");
  }

  // Outdoor temperature
  display.setTextSize(1);
  display.setCursor(0, 44);
  if (air_status.outdoor_te > -999) {
    display.printf("Out: %d C", air_status.outdoor_te);
  } else {
    display.printf("Out: -- C");
  }

  // Pressure from centralized array
  display.setCursor(70, 44);
  if (sensor_pressure[idx] > -999) {
    display.printf("%.0fmb", sensor_pressure[idx]);
  } else {
    display.printf("--mb");
  }

  // Bottom rotating information line (line 57)
  display.setTextSize(1);
  display.setCursor(0, 57);
  
  switch (infoIndex) {
    case 0: // Connection status
      if (WiFi.status() == WL_CONNECTED) {
        display.print("WiFi");
      } else {
        display.print("No WiFi");
      }
      
      #ifdef USE_MQTT
      if (mqtt_enabled && getMQTTStatus()) {
        display.print(" MQTT");
      } else if (mqtt_enabled) {
        display.print(" MQTT-ERR");
      }
      #endif
      break;
      
    case 1: 
    
      if (!autonomous_mode) {
        //Indoor sensors, ta, tcj, tc
        //outdoor sensors, te, to, td, ts, ths
        display.printf("TA: %d C TCJ: %d C TC: %d C",
                     air_status.indoor_ta, air_status.indoor_tcj, air_status.indoor_tc);
      }
      // Show outdoor temperatures
      else {
        display.printf("TE: %d C TO: %d C TD: %d C TS: %d C THS: %d C",
                     air_status.outdoor_te, air_status.outdoor_to,
                     air_status.outdoor_td, air_status.outdoor_ts,
                     air_status.outdoor_ths);
        }
      break;
      
    case 2: // Timer info or IP
      if (air_status.timer_enabled) {
        display.printf("Timer %d %s", air_status.timer_time_req, 
                       air_status.timer_mode_req == TIMER_SW_OFF ? "OFF" : "ON");
      } else {
        display.print("IP: ");
        display.print(air_status.ip);
      }
      break;
      
    case 3: // Autonomous mode or outdoor current
      if (autonomous_mode) {
        display.printf("Master %x02 Remote %x02", air_status.master, air_status.remote);
      } else {
        // Show current and RPM
        display.printf("Current: %d A RPM: %d", air_status.outdoor_current, air_status.indoor_fan_speed);
      }
      break;
  }

  display.display();
}
#endif


void handleSaveFile() {
  //if (timerSaveFile.isTime()) {
  if (my_timer_is_time(&timerSaveFile)) {
    String txt = air_to_json(&air_status);
    save_file("/status.json", txt);
    print_log("Saving status.json");
  }
}


void getTemperatureCurrent() {
  float best_temp = -999.0;
  float best_humidity = -999.0;
  float best_pressure = -999.0;

#ifdef USE_AHT20
  if (humidity_status) {
    sensors_event_t humidity, temp;
    if (aht.getEvent(&humidity, &temp)) {
      aht_t_current = temp.temperature;
      sensor_humidity_current = humidity.relative_humidity;
      
      best_temp = aht_t_current;
      best_humidity = sensor_humidity_current;
      
      print_logf("[AHT20] temp %.1f hum %.1f\n", aht_t_current, sensor_humidity_current);
    } else {
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
    
    // Use BMP280 temperature only if AHT20 failed
    if (best_temp <= -999.0) {
        best_temp = bmp280_t_current;
    }
    best_pressure = sensor_pressure_current;
    
    print_logf("[BMP280] temp %.1f press %.1f hPa\n", bmp280_t_current, sensor_pressure_current);
  }
  yield();
#endif

  // Update centralized current readings
  sensor_temperature_current = (best_temp > -999.0) ? best_temp : air_status.remote_sensor_temp;
  sensor_humidity_current = best_humidity;
  sensor_pressure_current = best_pressure;

  ac_sensor_temperature[temp_idx] = air_status.remote_sensor_temp;
  air_query_sensor(&air_status, OUTDOOR_TE);
  ac_outdoor_te[temp_idx] = air_status.outdoor_te;

  // FIX this. I do not want to create a new entry but report this on status

  //timeClient.update();
  //timestamps[temp_idx] = timeClient.getEpochTime();
  //temp_idx = (temp_idx + 1) % MAX_LOG_DATA;
}

//get current status, temperature and sensors. not intended for logging
void handleStatus() {
  //if (timerStatus.isTime()) {
  if (my_timer_is_time(&timerStatus)) {
    getTemperatureCurrent();
    yield(); // Allow ESP8266 to handle WiFi stack
        
    air_query_sensors(&air_status, sensor_ids, sizeof(sensor_ids));
    yield(); // Allow ESP8266 to handle WiFi stack

    air_print_status(&air_status);
    yield(); // Allow ESP8266 to handle WiFi stack

    //air_explore_all_sensors(&air_status); //it takes a lot of time, use it just to discover sensors
    //air_status.power_consumption += air_status.outdoor_current * 10 / (3600 / temp_interval); //30 readings per hour

  }
}

void handleReadSerial() {
  //int val;
  //if (timerReadSerial.isTime()) {
  if (my_timer_is_time(&timerReadSerial)) {
    //val = 
    air_parse_serial(&air_status);
  }
}

void handleAutonomousMode() {
  if (my_timer_is_time(&timerAutonomous) && autonomous_mode) {
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
  }
}

/*__________________________________________________________LOOP__________________________________________________________*/

void loop() {
  handleTimer();                              // Set all handlers
  yield();
  handleTemperature();
  yield();
  handleStatus();
  yield();
  handleReadSerial();
  yield();
#ifdef USE_MQTT
  if (mqtt_enabled) {
    handleMQTT();
  }
  yield();
#endif //USE_MQTT

  webSocket.loop();                           // constantly check for websocket events
  server.handleClient();                      // run the server

  ArduinoOTA.handle();                        // listen for OTA events
  MDNS.update();
  #ifdef USE_SCREEN
  showDisplay();
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
  Serial.println("Connection stablished.");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
}

void startOTA() { // Start the OTA service
  ArduinoOTA.setHostname(OTAName);
  ArduinoOTA.setPassword(OTAPassword);

  ArduinoOTA.onStart([]() {
    Serial.println("Start OTA");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\r\nEnd OTA");
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
  Serial.println("OTA ready\r\n");
}

void startLittleFS() { // Start the LittleFS and list all contents
  LittleFS.begin();                             // Start the SPI Flash File System (LittleFS)
  Serial.println("LittleFS started. Contents:");
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
#else //use links2004
  webSocket.begin();                          // start the websocket server
  webSocket.onEvent(onWsEvent);          // if there's an incomming websocket message, go to function 'webSocketEvent'
#endif
  Serial.println("WebSocket server started.");
}

void startMDNS() { // Start the mDNS responder
  MDNS.begin(mdnsName);                        // start the multicast domain name server
  Serial.print("mDNS responder started: http://");
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

  // Serve built-in editor
  server.on("/upload.html", HTTP_GET, []() {
        server.send_P(200, "text/html", upload_html);
  });

  server.on("/upload.html", HTTP_POST, []() {  
    server.send(200, "text/plain", "Upload successful!");
  }, handleFileUpload);                       


  server.onNotFound(handleNotFound);          // if someone requests any other file or page, go to function 'handleNotFound'
  // and check if the file exists

#endif



  server.begin();                             // start the HTTP server
  Serial.println("HTTP server started.");
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
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wm.autoConnect("aircondAP")) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(1000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  ticker.detach();
  //keep LED off
  digitalWrite(LED, HIGH);
}


//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
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
      Serial.println("file open failed" + path);
      ret = false;
    } else {
      size_t sent = server.streamFile(file, contentType);    // Send it to the client
      (void)sent; // not to be used, but prevents compiler warning
      file.close();                                          // Close the file again
      Serial.println(String("\tSent file: ") + path);
      ret = true;
    }
  } else
    Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
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
    Serial.print("handleFileUpload Name: "); Serial.println(path);
    fsUploadFile = LittleFS.open(path, "w");            // Open the file for writing in LittleFS (create if it doesn't exist)
    path = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {                                   // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
      server.sendHeader("Location", "/success.html");     // Redirect the client to the success page
      server.send(303);
    } else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
}

/*
    websocket callback
*/
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
      //pendingWSRequest
      processRequest(payload);
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


int save_file(String name, String text) {
  File file = LittleFS.open(name, "w");

  if (!file) {
    Serial.println("Error opening file for writing");
    return (0);
  }

  int bytes = file.print(text);
  file.close();

  return (bytes);
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
