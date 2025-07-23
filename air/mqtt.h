#ifndef MQTT_H
#define MQTT_H

#include <PubSubClient.h>
#include "ac_protocol.h"

// External variables that need to be accessible
extern WiFiClient espClient;
extern PubSubClient mqttClient;
extern air_status_t air_status;
extern float dht_t_current;
extern float dht_h_current;
//extern MySimpleTimer timerMQTT;

// Function declarations
void startMQTT();
void startMQTTTimer();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void connectToMQTT();
void handleMQTT();
void sendMQTTDiscovery();
bool getMQTTStatus();
void saveMQTTConfigToFile();
void loadMQTTConfigFromFile();
void configureMQTT(const char* server, int port, const char* user, const char* password, const char* device);
bool testMQTTConnection();
void resetMQTTDiscovery();
void sendMQTTDiscovery();

// Getter functions
String getMQTTServer();
int getMQTTPort();
String getMQTTUser();
String getMQTTPassword();
String getMQTTDeviceName();

#endif