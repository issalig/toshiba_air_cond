#include "main.h"



//not necessary because now we use WifiManager
//change it according to your network
IPAddress ip(192, 168, 2, 200);
IPAddress gateway(192, 168, 2, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);

air_status_t air_status;
MySimpleTimer timerAC;

#ifdef USE_ASYNC
AsyncWebServer server(80);
AsyncWebSocket webSocket("/ws");
#else
ESP8266WebServer server(80);       // create a web server on port 80
WebSocketsServer webSocket(81);    // create a websocket server on port 81
#endif

File fsUploadFile;                                    // a File variable to temporarily store the received file

MySimpleTimer timerTemperature;
int temp_interval = 1800;//in secs, 30 mins


DHT dht(DHTPin, DHTTYPE);

#include <Wire.h>
#include <Adafruit_BMP085.h>
Adafruit_BMP085 bmp;
uint8_t bmp_status = 0;

float dht_h[MAX_LOG_DATA];
float dht_t[MAX_LOG_DATA];
float ac_sensor[MAX_LOG_DATA];
//float ac_target[MAX_LOG_DATA];
int ac_outdoor_te[MAX_LOG_DATA];
float bmp_t[MAX_LOG_DATA];
float bmp_p[MAX_LOG_DATA];
unsigned long timestamps[MAX_LOG_DATA];
int temp_idx = 0;
float dht_h_current, dht_t_current, bmp_t_current, bmp_p_current = 0;

// Define NTP Client to get time
WiFiUDP ntpUDP;
const long utcOffsetInSeconds = 3600; //1 hour for Europe/Brussels
int timeOffset = 0;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 0); //do not apply utc here

MySimpleTimer timerStatus;
MySimpleTimer timerReadSerial;
MySimpleTimer timerSaveFile;

#define RESET_MODE_PIN D4  //button to enter into wifi configuration


void configModeCallback (WiFiManager *myWiFiManager);
void handleNotFound();
bool handleFileRead(String path);
void handleFileUpload();
void onWsEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
int save_file(String name, String text);
void tick();
void startWifiManager();
void startOTA();
void startSPIFFS();
void startWebSocket();
void startMDNS();
void startServer();
void startReadSerial();           // Start timer for serial readings (1s)
void handleTimer();
void handleTemperature();
void handleSaveFile();
void getTemperatureCurrent();
void handleStatus();
void handleReadSerial();
void startStatus();               // Start timer for status print (10s)
void startTime();                 // Get time from NTP server
void startTemperature();



void setup() {

  Serial.begin(115200);        // Start the Serial communication to send messages to the computer
  //  gdbstub_init();
  delay(10);
  Serial.println("\r\n");
  Serial.println("Air conditioning starts!");

  //startWiFi();                 // Start a Wi-Fi access point, and try to connect to some given access points. Then wait for either an AP or STA connection
  startWifiManager();          // Use wifimanager to connect

  //is_reset_button();

  startOTA();                  // Start the OTA service

  startSPIFFS();               // Start the SPIFFS and list all contents

  startWebSocket();            // Start a WebSocket server

  startMDNS();                 // Start the mDNS responder

  startServer();               // Start a HTTP server with a file read handler and an upload handler

  init_air_serial(&air_status);// Start air conditioning structure and software serial

  air_status.ip = WiFi.localIP().toString();

  startReadSerial();           // Start timer for serial readings (1s)

  startStatus();               // Start timer for status print (10s)

  startTime();                 // Get time from NTP server

  startTemperature();          // Start timer for temperature readings (120s)
  
}

void loop() {
  handleTimer();                              // Set all handlers
  handleTemperature();
  handleStatus();
  handleReadSerial();

#ifndef USE_ASYNC
  webSocket.loop();                           // constantly check for websocket events
  server.handleClient();                      // run the server
#endif

  ArduinoOTA.handle();                        // listen for OTA events
}

void startTime() {
  long int boot_time;

  timeClient.begin();          // Get NTP time
  //timeClient.update();
  for (int i = 0; i < 5; i++) {
    timeClient.update();                        // update time
    boot_time = timeClient.getEpochTime();
    if (boot_time > 3600) break;                  // check we are not still in 1970
  }
  air_status.boot_time = boot_time;
}

void startTemperature() {
  timerTemperature.setUnit(1000);
  timerTemperature.setInterval(temp_interval);
  timerTemperature.repeat();
  timerTemperature.start();

  //clear arrays
  memset(dht_h, 0, MAX_LOG_DATA * sizeof(float));
  memset(dht_t, 0, MAX_LOG_DATA * sizeof(float));
  memset(ac_sensor, 0, MAX_LOG_DATA * sizeof(float));
  memset(ac_outdoor_te, 0, MAX_LOG_DATA * sizeof(float));
  memset(bmp_p, 0, MAX_LOG_DATA * sizeof(float));
  memset(bmp_t, 0, MAX_LOG_DATA * sizeof(float));
  memset(timestamps, 0, MAX_LOG_DATA * sizeof(unsigned long));


  dht.begin();
  dht_h[0] = dht.readHumidity();
  dht_t[0] = dht.readTemperature();
  dht_t[0] = dht_h[0] = 0;//-1;

  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    bmp_status = 0;
  } else {
    bmp_t[0] = bmp.readTemperature();
    bmp_p[0] = bmp.readPressure();
    bmp_status = 1;
  }

  ac_sensor[0] = air_status.sensor_temp;
  //ac_target[0] = air_status.target_temp * air_status.power;

  air_query_sensor(&air_status, OUTDOOR_TE);
  ac_outdoor_te[0] = air_status.outdoor_te;

  timestamps[0] = 0; //timeClient.getEpochTime();
}

void startStatus() {
  timerStatus.setUnit(1000); // 1000ms
  timerStatus.setInterval(120); //update every XX s
  timerStatus.repeat();
  timerStatus.start();

  air_query_sensors(&air_status); //query sensors on start
}

void startReadSerial() {
  timerReadSerial.setUnit(1000);
  timerReadSerial.setInterval(1);
  timerReadSerial.repeat();
  timerReadSerial.start();
}

//save status every hour
void startSaveFile() {
  timerSaveFile.setUnit(1000);
  timerSaveFile.setInterval(60);
  timerSaveFile.repeat();
  timerSaveFile.start();
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
  if (timerAC.isTime()) {
    if (air_status.timer_mode_req == TIMER_SW_OFF) {
      //set power off
      Serial.println("TIMER - POWER OFF");
      air_set_power_off(&air_status);
    } else if (air_status.timer_mode_req == TIMER_SW_ON) {
      //set power on
      Serial.println("TIMER - POWER ON");
      air_set_power_on(&air_status);
    }
  }
}

//we will call this every sampling time
void handleTemperature() {
  if (timerTemperature.isTime()) {
    Serial.printf("[TEMP] Reading sensors\n");
    // Reading temperature or humidity takes about 250 milliseconds!
    dht_h[temp_idx] = dht.readHumidity();
    dht_t[temp_idx] = dht.readTemperature();
    Serial.printf("DHT %d temp %.1f hum %.1f\n", temp_idx, dht_t[temp_idx], dht_h[temp_idx]);

    if (!bmp_status) //try again
      bmp_status = bmp.begin();

    if (bmp_status) {
      bmp_t[temp_idx] = bmp.readTemperature();
      bmp_p[temp_idx] = bmp.readPressure() / 100; //in mb
      Serial.printf("BMP %d temp %.1f press %.1f\n",  temp_idx, bmp_t[temp_idx], bmp_p[temp_idx]);
    } else {
      //set to -1 when not available
      bmp_t[temp_idx] = bmp_t[temp_idx] - 1;
      bmp_p[temp_idx] = bmp_p[temp_idx] - 1;
      Serial.printf("BMP %d temp %.1f press %.1f\n", temp_idx, bmp_t[temp_idx], bmp_p[temp_idx]);
    }

    //ac_target[temp_idx] = air_status.target_temp * air_status.power; //0 if powered off
    ac_sensor[temp_idx] = air_status.sensor_temp;

    air_query_sensor(&air_status, OUTDOOR_TE);
    ac_outdoor_te[temp_idx] = air_status.outdoor_te;

    timeClient.update();
    timestamps[temp_idx] = timeClient.getEpochTime();

    temp_idx = (temp_idx + 1) % MAX_LOG_DATA;
  }
}

void handleSaveFile() {
  if (timerSaveFile.isTime()) {
    String txt = air_to_json(&air_status);
    save_file("/status.json", txt);
    Serial.print("Saving status.json");
  }
}

//get temp without logging it
void getTemperatureCurrent() {
  // Reading temperature or humidity takes about 250 milliseconds!
  dht_h_current = dht.readHumidity();
  dht_t_current = dht.readTemperature();
  Serial.printf("[DHT] temp %.1f hum %.1f\n", dht_t_current, dht_h_current);

  if (!bmp_status) //try again
    bmp_status = bmp.begin();

  if (bmp_status) {
    bmp_t_current = bmp.readTemperature();
    bmp_p_current = bmp.readPressure() / 100; //in mb
  }

  Serial.printf("[BMP] temp %.1f press %.1f\n", bmp_t_current, bmp_p_current);
}

//get current status, temperature and sensors. not intended for logging
void handleStatus() {
  if (timerStatus.isTime()) {
    getTemperatureCurrent();
    air_query_sensors(&air_status); //query few sensors, if this takes too much time it can crash wifi
    air_print_status(&air_status);
    //air_explore_all_sensors(&air_status); //it takes a lot of time, use it just to discover sensors
    air_status.power_consumption += air_status.outdoor_current * 10 / (3600 / temp_interval); //30 readings per hour
  }
}

void handleReadSerial() {
  int val;
  if (timerReadSerial.isTime()) {
    val = air_parse_serial(&air_status);
  }
}

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

  server.onNotFound(handleNotFound);          // if someone requests any other file or page, go to function 'handleNotFound'
  // and check if the file exists

#endif

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

void startWifiManager() {
  //set led pin as output
  pinMode(LED_BUILTIN, OUTPUT);
  // start ticker with 0.5 because we start in AP mode and try to connect
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
  if (!handleFileRead(server.uri())) {        // check if the file exists in the flash memory (SPIFFS), if so, send it
    server.send(404, "text/plain", "404: File Not Found: " + server.uri());
  }
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  bool ret;

  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";          // If a folder is requested, send the index file
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) { // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz))                         // If there's a compressed version available
      path += ".gz";                                         // Use the compressed verion
    File file = SPIFFS.open(path, "r");                    // Open the file
    if (!file) {
      Serial.println("file open failed" + path);
      ret = false;
    } else {
      size_t sent = server.streamFile(file, contentType);    // Send it to the client
      file.close();                                          // Close the file again
      Serial.println(String("\tSent file: ") + path);
      ret = true;
    }
  } else
    Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
  ret = false;

  return ret;
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
  }
}


int save_file(String name, String text) {
  File file = SPIFFS.open(name, "w");

  if (!file) {
    Serial.println("Error opening file for writing");
    return (0);
  }


  int bytes = file.print(text);

  file.close();

  return (bytes);
}

void tick()
{
  //toggle state
  digitalWrite(LED, !digitalRead(LED));     // set pin to the opposite state
}

