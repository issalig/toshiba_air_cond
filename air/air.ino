/*
  Name:       air.ino
  Created:    01/06/2020 initial version
              15/07/2020 Websockets
              27/08/2020 temperature graph
              05/09/2020 fix temp, fix heat mode
              01/10/2020 bmp180 support adds pressure

  Author:     issalig
  Description: Control toshiba air cond via web

              HW
              Connect circuit(see readme.md) for reading/writing to ESP8266

              SW
              Upload data directory with ESP8266SketchDataUpload and flash it with USB for the first time. Then, when installed you can use OTA updates.


  References: https://github.com/tttapa/ESP8266/
              https://github.com/luisllamasbinaburo/ESP8266-Examples
              https://diyprojects.io/esp8266-web-server-part-5-add-google-charts-gauges-and-charts/#.X0gBsIbtY5k


  Dependencies: https://github.com/plerup/espsoftwareserial
*/


#include <ESP8266WiFi.h>
//#include <ESP8266WiFiMulti.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <WebSocketsServer.h> //Links2004 ///arduino websockets gil maimon
#include <ArduinoJson.h>  //by Benoit Blanchon
#include <NTPClient.h> //install from arduino or get it from https://github.com/arduino-libraries/NTPClient
#include <WiFiUdp.h>
#include "toshiba_serial.hpp"
#include "MySimpleTimer.hpp"

#include "credentials.h" // set your wifi pass there

// allows you to set the realm of authentication Default:"Login Required"
const char* www_realm = "Custom Auth Realm";
// the Content of the HTML response in case of Unautherized Access Default:empty
String authFailResponse = "Authentication Failed";

IPAddress ip(192, 168, 2, 200 );
IPAddress gateway(192, 168, 2, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);

air_status_t air_status;
MySimpleTimer timerAC;

//ESP8266WiFiMulti wifiMulti;       // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'

ESP8266WebServer server(80);       // create a web server on port 80
WebSocketsServer webSocket(81);    // create a websocket server on port 81

File fsUploadFile;                                    // a File variable to temporarily store the received file

MySimpleTimer timerTemperature;
int temp_interval = 1200;//in secs

#include "DHT.h"
const int DHTPin = D3;
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPin, DHTTYPE);

#include <Wire.h>
#include <Adafruit_BMP085.h>
Adafruit_BMP085 bmp;
uint8_t bmp_status = 0;

#define MAX_LOG_DATA 144//288 // for one day
float dht_h[MAX_LOG_DATA];
float dht_t[MAX_LOG_DATA];
float ac_sensor[MAX_LOG_DATA];
float ac_target[MAX_LOG_DATA];
float bmp_t[MAX_LOG_DATA];
float bmp_p[MAX_LOG_DATA];
unsigned long timestamp[MAX_LOG_DATA];
int temp_idx = 0;
float dht_h_current, dht_t_current, bmp_t_current, bmp_p_current = 0;

// Define NTP Client to get time
WiFiUDP ntpUDP;
const long utcOffsetInSeconds = 3600; //1 hour for Europe/Brussels
int timeOffset = 0;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 0); //do not apply utc here

MySimpleTimer timerStatus;
MySimpleTimer timerReadSerial;

/*__________________________________________________________SETUP__________________________________________________________*/

void setup() {


  Serial.begin(115200);        // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println("\r\n");

  startWiFi();                 // Start a Wi-Fi access point, and try to connect to some given access points. Then wait for either an AP or STA connection

  startOTA();                  // Start the OTA service

  startSPIFFS();               // Start the SPIFFS and list all contents

  startWebSocket();            // Start a WebSocket server

  startMDNS();                 // Start the mDNS responder

  startServer();               // Start a HTTP server with a file read handler and an upload handler

  init_air_serial(&air_status);// Start air conditioning structure and software serial

  startReadSerial();           // Start timer for serial readings (1s)

  startStatus();               // Start timer for status print (10s)

  timeClient.begin();          //get NTP time

  startTemperature();          // Start timer for temperature readings (120s)
}

void startTemperature() {
  timerTemperature.setUnit(1000);
  timerTemperature.setInterval(temp_interval);
  timerTemperature.repeat();
  timerTemperature.start();

  dht.begin();
  dht_h[0] = dht.readHumidity();
  dht_t[0] = dht.readTemperature();

  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    bmp_status = 0;
  } else {
    bmp_t[0] = bmp.readTemperature();
    bmp_p[0] = bmp.readPressure();
    bmp_status = 1;
  }

  ac_sensor[0] = air_status.sensor_temp;
  ac_target[0] = air_status.target_temp * air_status.power;

  timestamp[0] = 0; //timeClient.getEpochTime();
}

void startStatus() {
  timerStatus.setUnit(1000);
  timerStatus.setInterval(10);
  timerStatus.repeat();
  timerStatus.start();
}

void startReadSerial() {
  timerReadSerial.setUnit(1000);
  timerReadSerial.setInterval(1);
  timerReadSerial.repeat();
  timerReadSerial.start();
}

void handleTimer() {
  if (timerAC.isTime()) {
    if (air_status.timer_mode_req == TIMER_POWER_OFF) {
      //set power off
      Serial.println("TIMER - POWER OFF");
      air_set_power_off(&air_status);
    } else {
      //set power on
      Serial.println("TIMER - POWER ON");
      air_set_power_on(&air_status);
    }
  }
}

void handleTemperature() {
  if (timerTemperature.isTime()) {
    // Reading temperature or humidity takes about 250 milliseconds!
    dht_h[temp_idx] = dht.readHumidity();
    dht_t[temp_idx] = dht.readTemperature();
    Serial.printf("DHT temp %.1f hum %.1f\n", dht_t[temp_idx], dht_h[temp_idx]);

    if (!bmp_status) //try again
      bmp_status = bmp.begin();

    if (bmp_status) {
      bmp_t[temp_idx] = bmp.readTemperature();
      bmp_p[temp_idx] = bmp.readPressure() / 100; //in mb
      Serial.printf("BMP temp %.1f press %.1f\n", bmp_t[temp_idx], bmp_p[temp_idx]);
    } else {
      bmp_t[temp_idx] = bmp_t[temp_idx] - 1;
      bmp_p[temp_idx] = bmp_p[temp_idx] - 1;
      Serial.printf("BMP temp %.1f press %.1f\n", bmp_t[temp_idx], bmp_p[temp_idx]);
    }

    ac_target[temp_idx] = air_status.target_temp * air_status.power; //0 if powered off
    ac_sensor[temp_idx] = air_status.sensor_temp;

    timeClient.update();
    timestamp[temp_idx] = timeClient.getEpochTime();

    temp_idx = (temp_idx + 1) % MAX_LOG_DATA;
  }
}

void getTemperatureCurrent() {
  // Reading temperature or humidity takes about 250 milliseconds!
  dht_h_current = dht.readHumidity();
  dht_t_current = dht.readTemperature();
  Serial.printf("DHT temp %.1f hum %.1f\n", dht_t_current, dht_h_current);
  
  if (!bmp_status) //try again
    bmp_status = bmp.begin();

  if (bmp_status) {
    bmp_t_current = bmp.readTemperature();
    bmp_p_current = bmp.readPressure() / 100; //in mb   
  } 

  Serial.printf("BMP temp %.1f press %.1f\n", bmp_t_current, bmp_p_current);
}

void handleStatus() {
  if (timerStatus.isTime()) {
    getTemperatureCurrent();
    air_print_status(&air_status);
  }
}

void handleReadSerial() {
  if (timerReadSerial.isTime()) {
    air_parse_serial(&air_status);
  }
}

/*__________________________________________________________LOOP__________________________________________________________*/

void loop() {

  handleTimer();
  handleTemperature();
  handleStatus();
  handleReadSerial();

  webSocket.loop();                           // constantly check for websocket events
  server.handleClient();                      // run the server
  ArduinoOTA.handle();                        // listen for OTA events
}

/*__________________________________________________________SETUP_FUNCTIONS__________________________________________________________*/

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

/*
  void startWiFi_ap() { // Start a Wi-Fi access point, and try to connect to some given access points. Then wait for either an AP or STA connection
  WiFi.softAP(ssid, password);             // Start the access point
  Serial.print("Access Point \"");
  Serial.print(ssid);
  Serial.println("\" started\r\n");

  wifiMulti.addAP(w_ssid, w_passwd);   // add Wi-Fi networks you want to connect to
  //wifiMulti.addAP("ssid_from_AP_2", "your_password_for_AP_2");
  //wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");

  Serial.println("Connecting");
  while (wifiMulti.run() != WL_CONNECTED && WiFi.softAPgetStationNum() < 1) {  // Wait for the Wi-Fi to connect
    delay(250);
    Serial.print('.');
  }
  Serial.println("\r\n");
  if (WiFi.softAPgetStationNum() == 0) {     // If the ESP is connected to an AP
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());             // Tell us what network we're connected to
    Serial.print("IP address:\t");
    Serial.print(WiFi.localIP());            // Send the IP address of the ESP8266 to the computer
  } else {                                   // If a station is connected to the ESP SoftAP
    Serial.print("Station connected to ESP8266 AP");
  }
  Serial.println("\r\n");
  }
*/

void startOTA() { // Start the OTA service
  ArduinoOTA.setHostname(OTAName);
  ArduinoOTA.setPassword(OTAPassword);

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\r\nEnd");
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

void startSPIFFS() { // Start the SPIFFS and list all contents
  SPIFFS.begin();                             // Start the SPI Flash File System (SPIFFS)
  Serial.println("SPIFFS started. Contents:");
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {                      // List the file system contents
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("\tFS File: %s, size: %s\r\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    Serial.printf("\n");
  }
}

void startWebSocket() { // Start a WebSocket server
  webSocket.begin();                          // start the websocket server
  webSocket.onEvent(webSocketEvent);          // if there's an incomming websocket message, go to function 'webSocketEvent'
  Serial.println("WebSocket server started.");
}

void startMDNS() { // Start the mDNS responder
  MDNS.begin(mdnsName);                        // start the multicast domain name server
  Serial.print("mDNS responder started: http://");
  Serial.print(mdnsName);
  Serial.println(".local");
}

bool isInternalIP(ESP8266WebServer *myserver) {
  IPAddress clientIP = myserver->client().remoteIP();
  IPAddress serverIP = WiFi.localIP();

  Serial.println(myserver->client().remoteIP());
  Serial.println(WiFi.localIP());

  Serial.println(myserver->client().remoteIP()[0]);
  Serial.println(WiFi.localIP()[0]);

  Serial.println(myserver->client().remoteIP()[1]);
  Serial.println(WiFi.localIP()[1]);
  Serial.println(clientIP[0] == serverIP[0] && clientIP[1] == serverIP[1]);

  return (clientIP[0] == serverIP[0] && clientIP[1] == serverIP[1]);
}

void startServer() { // Start a HTTP server with a file read handler and an upload handler
  server.on("/edit.html",  HTTP_POST, []() {  // If a POST request is sent to the /edit.html address,
    server.send(200, "text/plain", "");
  }, handleFileUpload);                       // go to 'handleFileUpload'

  server.onNotFound(handleNotFound);          // if someone requests any other file or page, go to function 'handleNotFound'
  // and check if the file exists

  /*
    server.on("/", []() {
    if (!isInternalIP(&server)){ //ask for autheitcation if not inside local net
    if (!server.authenticate(http_user, http_passwd))
      //Basic Auth Method with Custom realm and Failure Response
      //return server.requestAuthentication(BASIC_AUTH, www_realm, authFailResponse);
      //Digest Auth Method with realm="Login Required" and empty Failure Response
      //return server.requestAuthentication(DIGEST_AUTH);
      //Digest Auth Method with Custom realm and empty Failure Response
      //return server.requestAuthentication(DIGEST_AUTH, www_realm);
      //Digest Auth Method with Custom realm and Failure Response
    {
      return server.requestAuthentication(DIGEST_AUTH, www_realm, authFailResponse);
    }}
    //server.sendHeader("Location", "/index.html");     // Redirect the client to main page
    //server.send(303);

    //server.send(200, "text/html", inde);
    });
  */

  server.begin();                             // start the HTTP server
  Serial.println("HTTP server started.");
}

/*__________________________________________________________SERVER_HANDLERS__________________________________________________________*/

void handleNotFound() { // if the requested file or page doesn't exist, return a 404 not found error
  if (!handleFileRead(server.uri())) {        // check if the file exists in the flash memory (SPIFFS), if so, send it
    server.send(404, "text/plain", "404: File Not Found");
  }
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";          // If a folder is requested, send the index file
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) { // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz))                         // If there's a compressed version available
      path += ".gz";                                         // Use the compressed verion
    File file = SPIFFS.open(path, "r");                    // Open the file
    size_t sent = server.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
  return false;
}

void handleFileUpload() { // upload a new file to the SPIFFS
  HTTPUpload& upload = server.upload();
  String path;
  if (upload.status == UPLOAD_FILE_START) {
    path = upload.filename;
    if (!path.startsWith("/")) path = "/" + path;
    if (!path.endsWith(".gz")) {                         // The file server always prefers a compressed version of a file
      String pathWithGz = path + ".gz";                  // So if an uploaded file is not compressed, the existing compressed
      if (SPIFFS.exists(pathWithGz))                     // version of that file must be deleted (if it exists)
        SPIFFS.remove(pathWithGz);
    }
    Serial.print("handleFileUpload Name: "); Serial.println(path);
    fsUploadFile = SPIFFS.open(path, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
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

/*void json2air(String request, air_status_t *air)
  {
  StaticJsonDocument<200> jsonDoc;
  DeserializationError error = deserializeJson(jsonDoc, request);
  if (error) {
    return;
  }
  }*/

String air_to_json(air_status_t *air)
{
  String response;
  StaticJsonDocument<400> jsonDoc;

  jsonDoc["id"] = "status";
  jsonDoc["save"] = air->save;
  jsonDoc["heat"] = air->heat;
  jsonDoc["cold"] = air->cold;
  jsonDoc["temp"] = air->target_temp;
  jsonDoc["sensor_temp"] = air->sensor_temp;
  jsonDoc["fan"] = air->fan_str;
  jsonDoc["mode"] = air->mode_str;
  jsonDoc["power"] = air->power;
  jsonDoc["timer_mode"] = air->timer_mode_req;
  jsonDoc["timer_enabled"] = timerAC.isEnabled();
  jsonDoc["timer_pending"] = timerAC.pendingTime();
  jsonDoc["timer_time"] = timerAC.getInterval();
  jsonDoc["sampling"] = temp_interval;
  jsonDoc["dht_temp"] = dht_t_current;//[(temp_idx - 1) % MAX_LOG_DATA];
  jsonDoc["dht_hum"] = dht_h_current;//[(temp_idx - 1) % MAX_LOG_DATA];
  jsonDoc["bmp_temp"] = bmp_t_current;//[(temp_idx - 1) % MAX_LOG_DATA];
  jsonDoc["bmp_press"] = bmp_p_current;//[(temp_idx - 1) % MAX_LOG_DATA];
  jsonDoc["decode_errors"] = air->decode_errors;

  int i;
  String str;
  for (i = 0; (i < MAX_CMD_BUFFER) && (i < (air->last_cmd[3] + 5)) ; i++)
    str += ((air->last_cmd[i] < 0x10) ? "0" : "")  + String(air->last_cmd[i], HEX) + " ";
  jsonDoc["last_cmd"] = str;

  str = "";
  //for (i = air->curr_r_idx; (i < MAX_RX_BUFFER) && (i< air->curr_w_idx); i++) round buffer
  for (i = 0; (i < MAX_RX_BUFFER) && (i < air->curr_w_idx); i++) //not round buffer
    //for (i = air->curr_r_idx;  i!= air->curr_w_idx; i=(i+1)%MAX_RX_BUFFER)
    str += ((air->rx_data[i] < 0x10) ? "0" : "")  + String(air->rx_data[i], HEX) + " ";
  jsonDoc["rx_data"] = str;

  //jsonDoc["tx_data"] = air->tx_data;

  serializeJson(jsonDoc, response);

  return response;
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) { // When a WebSocket message is received
  switch (type) {
    case WStype_DISCONNECTED:             // if the websocket is disconnected
      Serial.printf("WS [%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {              // if a new websocket connection is established
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("WS [%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      }
      break;
    case WStype_TEXT:                     // if new text data is received
      Serial.printf("WS [%u] Received: %s\n", num, payload);

      //decode json
      StaticJsonDocument<200> jsonDoc;
      DeserializationError error = deserializeJson(jsonDoc, payload);
      if (error) {
        return;
      }

      if (jsonDoc["id"] == "power") {            // power
        if (jsonDoc["value"] == "on") {
          air_set_power_on(&air_status);
        } else if (jsonDoc["value"] == "off") {
          air_set_power_off(&air_status);
        }

      } else if (jsonDoc["id"] == "temp") {
        int val = atoi(jsonDoc["temp"]);

        //if (jsonDoc["temp"] == "1") { //
        if (val == 1) {
          val = air_status.target_temp + 1;
          if (val > 30) val = 30;
          if (val < 19) val = 19;
          air_set_temp(&air_status, val);
          //} else if (jsonDoc["temp"] == "0") { //
        } else if (val == 0) {
          val = air_status.target_temp - 1;
          if (val > 30) val = 30;
          if (val < 19) val = 19;
          air_set_temp(&air_status, val);
        } else {
          if (val > 30) val = 30;
          if (val < 19) val = 19;
          air_set_temp(&air_status, val);
        }
      }
      else if (jsonDoc["id"] == "timer") {
        Serial.println("Received timer");
        int val = atoi(jsonDoc["timer_time"]);
        timerAC.setInterval(val);
        timerAC.start();
        air_status.timer_mode_req = jsonDoc["timer_mode"];
        air_status.timer_time_req = val;
      }
      else if (jsonDoc["id"] == "mode") {
        Serial.println("Received mode");
        if (jsonDoc["value"] == "cool") {
          air_set_mode(&air_status, MODE_COOL);
        } else if (jsonDoc["value"] == "dry") {
          air_set_mode(&air_status, MODE_DRY);
        } else if (jsonDoc["value"] == "fan") {
          air_set_mode(&air_status, MODE_FAN);
        } else if (jsonDoc["value"] == "heat") {
          air_set_mode(&air_status, MODE_HEAT);
        }
      }
      else if (jsonDoc["id"] == "fan") {
        Serial.println("Received fan");
        if (jsonDoc["value"] == "low") {
          air_set_fan(&air_status, FAN_LOW);
        } else if (jsonDoc["value"] == "medium") {
          air_set_fan(&air_status, FAN_MEDIUM);
        } else if (jsonDoc["value"] == "high") {
          air_set_fan(&air_status, FAN_HIGH);
        } else if (jsonDoc["value"] == "auto") {
          air_set_fan(&air_status, FAN_AUTO);
        }
      }
      else if (jsonDoc["id"] == "save") {
        Serial.println("Received save");
        if (jsonDoc["value"] == "1") {
          air_set_save_on(&air_status);
        } else if (jsonDoc["value"] == "0") {
          air_set_save_off(&air_status);
        }
      }
      else if (jsonDoc["id"] == "status") {
        Serial.println("Status");

        String message;
        //int i;
        //for (i = 0; i < MAX_CMD_BUFFER && i < (air_status.last_cmd[3] + 5) ; i++)
        //  message += ((air_status.last_cmd[i] < 0x10) ? "0" : "" ) + String(air_status.last_cmd[i], HEX) + " ";
        //Serial.println(message);

        message = air_to_json(&air_status);
        webSocket.broadcastTXT(message);
      }
      else if (jsonDoc["id"] == "sampling") {
        Serial.println("Sampling time");
        temp_interval = jsonDoc["value"];
        timerTemperature.setUnit(1000);
        timerTemperature.setInterval(temp_interval);
        timerTemperature.repeat();
        timerTemperature.start();

      }
      else if (jsonDoc["id"] == "timeseries") {
        Serial.println("TimeSeries");
        String message;

        //use assistant to estimate size https://arduinojson.org/v6/assistant/
        //https://arduinojson.org/v6/how-to/determine-the-capacity-of-the-jsondocument/
        //3000 holds around 150 vals
        DynamicJsonDocument docTimeSeries(16100);

        docTimeSeries["id"] = "timeseries";
        docTimeSeries["n"] = MAX_LOG_DATA;

        JsonArray arrt = docTimeSeries.createNestedArray("timestamp");
        int i;
        /*arrt.add(timestamp[temp_idx]);
          for (i = (temp_idx + 1) % MAX_LOG_DATA; i != temp_idx; i = (i + 1) % MAX_LOG_DATA) {
          arrt.add(timestamp[i]);
          }*/
        serialize_array_int(timestamp, arrt, temp_idx);

        JsonArray arr = docTimeSeries.createNestedArray("dht_t");
        serialize_array_float(dht_t, arr, temp_idx);

        JsonArray arr2 = docTimeSeries.createNestedArray("dht_h");
        serialize_array_float(dht_h, arr2, temp_idx);

        JsonArray arr4 = docTimeSeries.createNestedArray("ac_sensor_t");
        serialize_array_float(ac_sensor, arr4, temp_idx);

        JsonArray arr6 = docTimeSeries.createNestedArray("bmp_p");
        serialize_array_float(bmp_p, arr6, temp_idx);

        serializeJson(docTimeSeries, message);
        webSocket.broadcastTXT(message);

      } //end if timeseries
      break;
  }
}


void serialize_array_float(float *ptr, JsonArray &arr, int idx) {
  int i;
  arr.add(ptr[idx]);
  for (i = (idx + 1) % MAX_LOG_DATA; i != idx; i = (i + 1) % MAX_LOG_DATA) {
    arr.add(ptr[i]);
  }
}

void serialize_array_int(unsigned long int *ptr, JsonArray &arr, int idx) {
  int i;
  arr.add(ptr[idx]);
  for (i = (idx + 1) % MAX_LOG_DATA; i != idx; i = (i + 1) % MAX_LOG_DATA) {
    arr.add(ptr[i]);
  }
}


int save_string(String text) {

  File file = SPIFFS.open("/temp.dat", "w");

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
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  }
}

String getContentType(String filename) { // determine the filetype of a given filename, based on the extension
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}
