#pragma once
#include "config.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>

// for LED status
#include <Ticker.h>

#ifdef USE_ASYNC // for ESP32
#include <ESPAsync_WiFiManager.h_>
#else
#include <ESP8266WiFiMulti.h>
#include <WiFiManager.h>
#endif


namespace Wifi {
Ticker ticker;

// toggle state
void tick() {
  digitalWrite(LED, !digitalRead(LED)); // set pin to the opposite state
}

// gets called when WiFiManager enters configuration mode
void configModeCallback(WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  // if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  // entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}

void initManager() {
  // set led pin as output
  pinMode(LED_BUILTIN, OUTPUT);
  // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(0.6, tick);

  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it
  // around
  WiFiManager wm;
  // reset settings - for testing
  // wm.resetSettings();
  // wm.setSTAStaticIPConfig(ip, gateway, subnet);

  // set callback that gets called when connecting to previous WiFi fails, and
  // enters Access Point mode
  wm.setAPCallback(configModeCallback);

  // fetches ssid and pass and tries to connect
  // if it does not connect it starts an access point with the specified name
  // here  "AutoConnectAP"
  // and goes into a blocking loop awaiting configuration
  if (!wm.autoConnect("aircondAP")) {
    Serial.println("failed to connect and hit timeout");
    // reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(1000);
  }

  // if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  ticker.detach();
  // keep LED off
  digitalWrite(LED, HIGH);
}
} // namespace Wifi
