// mqtt code for home assistant integration
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "ac_protocol.h"
#include "process_request.h"
#include "my_timer.h"
#include "LittleFS.h"

// External variables from main program
extern air_status_t air_status;
extern float dht_t_current;
extern float dht_h_current;
extern my_timer timerMQTT;
extern bool simulation_mode;

#include "print_log.h"

// MQTT Configuration using String class
String mqtt_server_str = "homeassistant.local"; // Use your MQTT broker address
int mqtt_port = 1883;
String mqtt_user_str = "admin"; // Use your MQTT username
String mqtt_password_str = "1234";  // Use your MQTT password
String device_name_str = "toshiba_ac";

// MQTT Topics - using String objects
String temp_topic         = "homeassistant/ac/status/current_temperature";
String power_topic        = "homeassistant/ac/status/power";
String mode_topic         = "homeassistant/ac/status/mode";
String target_temp_topic  = "homeassistant/ac/status/temperature";
String command_topic      = "homeassistant/ac/command";

String mode_command_topic = "homeassistant/ac/set/mode";
String fan_mode_topic     = "homeassistant/ac/status/fan_mode";
String fan_mode_command_topic = "homeassistant/ac/set/fan_mode";
String temp_command_topic = "homeassistant/ac/set/temperature";

String climate_conf_topic = "homeassistant/climate/toshiba_ac/config";

WiFiClient espClient;
PubSubClient mqttClient(espClient);
bool mqttConnected = false;

// Function declarations
void mqttCallback(char* topic, byte* payload, unsigned int length);
void sendMQTTDiscovery();
void connectToMQTT();
void configureMQTT(const char* server, int port, const char* user, const char* password, const char* device);
bool getMQTTStatus();
void saveMQTTConfigToFile();
void loadMQTTConfigFromFile();

bool testMQTTConnection();
void resetMQTTDiscovery();
String getMQTTServer();
int getMQTTPort();
String getMQTTUser();
String getMQTTPassword();
String getMQTTDeviceName();

bool getMQTTStatus() {
  return mqttConnected;
}

void configureMQTT(const char* server, int port, const char* user, const char* password, const char* device) {
  mqtt_server_str = server;
  mqtt_port = port;
  mqtt_user_str = user;
  mqtt_password_str = password;
  device_name_str = device;
  
  // Save to file
  saveMQTTConfigToFile();
  
  // Restart MQTT with new configuration
  mqttClient.setServer(mqtt_server_str.c_str(), mqtt_port);
}

void saveMQTTConfigToFile() {
  JsonDocument doc;
  doc["server"] = mqtt_server_str;
  doc["port"] = mqtt_port;
  doc["username"] = mqtt_user_str;
  doc["password"] = mqtt_password_str;
  doc["device_name"] = device_name_str;
  
  String configJson;
  serializeJson(doc, configJson);
  
  File file = LittleFS.open("/mqtt_config.json", "w");
  if (file) {
    file.print(configJson);
    file.close();
    print_log("[MQTT] config saved to /mqtt_config.json");
  } else {
    print_log("[MQTT] Failed to save config");
  }
}

void loadMQTTConfigFromFile() {
  if (LittleFS.exists("/mqtt_config.json")) {
    File file = LittleFS.open("/mqtt_config.json", "r");
    if (file) {
      String configJson = file.readString();
      file.close();
      
      JsonDocument doc;
      if (deserializeJson(doc, configJson) == DeserializationError::Ok) {
        mqtt_server_str = doc["server"].as<String>();
        mqtt_port = doc["port"];
        mqtt_user_str = doc["username"].as<String>();
        mqtt_password_str = doc["password"].as<String>();
        device_name_str = doc["device_name"].as<String>();
        
        print_log("[MQTT] config loaded from file");
        print_logf("[MQTT] Server: %s, Port: %d, User: %s, Device: %s",
                  mqtt_server_str.c_str(), mqtt_port, mqtt_user_str.c_str(), device_name_str.c_str());
      }
    }
  }
}

void startMQTT() {
  mqttClient.setServer(mqtt_server_str.c_str(), mqtt_port);
  mqttClient.setCallback(mqttCallback);
  mqttClient.setBufferSize(1024); // Increase buffer size for discovery messages
  
  print_log("[MQTT] Configured");
}

void startMQTTTimer() {
  my_timer_set_unit(&timerMQTT, 1000); // 1000ms
  my_timer_set_interval(&timerMQTT, 30); // update every X s
  my_timer_repeat(&timerMQTT);
  my_timer_start(&timerMQTT);
}

void connectToMQTT() {
  // Only try to connect if we have network connectivity
  if (WiFi.status() != WL_CONNECTED) {
    print_log("[MQTT] WiFi not connected, skipping MQTT");
    return;
  }
  
  // Add a simple ping test to MQTT server before attempting connection
  WiFiClient testClient;
  if (!testClient.connect(mqtt_server_str.c_str(), mqtt_port)) {
    print_log("[MQTT] server not reachable, skipping connection");
    mqttConnected = false;
    return;
  }
  testClient.stop();
  
  // Only attempt connection if server is reachable
  if (!mqttClient.connected()) {
    print_log("[MQTT] Attempting connection...");
    
    if (mqttClient.connect(device_name_str.c_str(), mqtt_user_str.c_str(), mqtt_password_str.c_str())) {
      print_log("[MQTT] connected");
      mqttConnected = true;

      // Subscribe to command topic
      mqttClient.subscribe(mode_command_topic.c_str());
      mqttClient.subscribe(fan_mode_command_topic.c_str());
      mqttClient.subscribe(temp_command_topic.c_str());

      // Send discovery messages
      sendMQTTDiscovery();
      
    } else {
      print_logf("[MQTT] connection failed, rc=%d - will retry later", mqttClient.state());
    }
  }

  air_status.mqtt = mqttConnected; // Update air_status with MQTT status
}

void handleMQTT() {
  //if (timerMQTT.isTime()) {
  if (my_timer_is_time(&timerMQTT)) {
    // Only proceed if WiFi is connected
    if (WiFi.status() != WL_CONNECTED) {
      print_log("[MQTT] WiFi disconnected, skipping MQTT");
      mqttConnected = false;
      return;
    }
    
    if (!mqttClient.connected()) {
      mqttConnected = false;
      connectToMQTT();
    } 
    
    // Only send data if we're actually connected
    if (mqttClient.connected()) {
      mqttConnected = true;      
      
      // send mode
      String acMode = "off";
      if (air_status.power) {
        if (air_status.mode == MODE_COOL) {
          acMode = "cool";
        } else if (air_status.mode == MODE_HEAT) {
          acMode = "heat";
        } else if (air_status.mode == MODE_AUTO) {
          acMode = "auto";
        } else if (air_status.mode == MODE_FAN) {
          acMode = "fan_only";
        } else if (air_status.mode == MODE_DRY) {
          acMode = "dry";
        }
      }
      mqttClient.publish(mode_topic.c_str(), acMode.c_str());
      
      // send fan mode
      String fanMode = "auto"; // Default to auto
      if (air_status.fan == FAN_LOW) {
        fanMode = "low";
      } else if (air_status.fan == FAN_MEDIUM) {
        fanMode = "medium";
      } else if (air_status.fan == FAN_HIGH) {
        fanMode = "high";
      } else if (air_status.fan == FAN_AUTO) {
        fanMode = "auto";
      }
      mqttClient.publish(fan_mode_topic.c_str(), fanMode.c_str());

      // send target temperature
      mqttClient.publish(target_temp_topic.c_str(), String(air_status.target_temp).c_str());

      // send current temperature
      mqttClient.publish(temp_topic.c_str(), String(air_status.remote_sensor_temp).c_str());
      
      //combine all of the abose prints into a shorter one
      print_logf("[MQTT] Published - Target: %d, Current: %.1f, Mode: %s, Fan: %s",
                 air_status.target_temp, air_status.remote_sensor_temp, acMode.c_str(), fanMode.c_str());

    } else {
      print_log("[MQTT] not connected, skipping data send");
      mqttConnected = false;
    }
  }
  
  // Always call loop() to handle keep-alive, even if not connected
  mqttClient.loop();

  // Update connection status
  mqttConnected = mqttClient.connected();

  air_status.mqtt = mqttConnected; // Update air_status with MQTT status

  yield();
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  String message = "";
  for (unsigned int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }

  print_logf("[MQTT] received [%s]: %s", topic, message.c_str());

  bool statusChanged = false;

  if (String(topic) == temp_command_topic)
  {

    // Check if it's a plain temperature value first
    float tempFloat = message.toFloat();
    if (tempFloat > 0)
    { // toFloat() returns 0 if conversion fails
      int temp = (int)round(tempFloat);
      print_logf("[MQTT] Temperature command (plain): %.1f -> %d", tempFloat, temp);
      if (temp > 30)
        temp = 30;
      if (temp < 18)
        temp = 18;

      if (simulation_mode) {
        air_status.target_temp = temp;
        print_logf("[SIMULATION MODE] Temperature set to %d°C via MQTT", temp);
      } else {
        air_set_temp(&air_status, temp);
      }
      statusChanged = true;
    }
  }

  if (String(topic) == mode_command_topic)
  {
    if (message == "on")
    {
      print_log("[MQTT] Power ON command");
      if (simulation_mode) {
        air_status.power = true;
        print_log("[SIMULATION MODE] Power ON via MQTT");
      } else {
        air_set_power_on(&air_status);
      }
      statusChanged = true;
    }
    else if (message == "off")
    {
      print_log("[MQTT] Power OFF command");
      if (simulation_mode) {
        air_status.power = false;
        print_log("[SIMULATION MODE] Power OFF via MQTT");
      } else {
        air_set_power_off(&air_status);
      }
      statusChanged = true;
    }
    else if (message == "cool" || message == "heat" || message == "auto" || message == "fan_only" || message == "dry")
    {
      print_logf("[MQTT] Mode command: %s", message.c_str());
       if (simulation_mode) {
        air_status.power = true; // Turn on power in test mode
        if (message == "cool") {
          air_status.mode = MODE_COOL;
          strcpy(air_status.mode_str, "COOL");
          print_log("[SIMULATION MODE] Mode set to COOL via MQTT");
          statusChanged = true;
        } else if (message == "heat") {
          air_status.mode = MODE_HEAT;
          strcpy(air_status.mode_str, "HEAT");
          print_log("[SIMULATION MODE] Mode set to HEAT via MQTT");
          statusChanged = true;
        } else if (message == "auto") {
          air_status.mode = MODE_AUTO;
          strcpy(air_status.mode_str, "AUTO");
          print_log("[SIMULATION MODE] Mode set to AUTO via MQTT");
          statusChanged = true;
        } else if (message == "fan_only") {
          air_status.mode = MODE_FAN;
          strcpy(air_status.mode_str, "FAN");
          print_log("[SIMULATION MODE] Mode set to FAN via MQTT");
          statusChanged = true;
        } else if (message == "dry") {
          air_status.mode = MODE_DRY;
          strcpy(air_status.mode_str, "DRY");
          print_log("[SIMULATION MODE] Mode set to DRY via MQTT");
          statusChanged = true;
        }
      } else {
        // Normal mode - send commands to AC

        if (message == "cool")
        {
          air_set_power_on(&air_status);
          yield(); // Allow other tasks to run
          air_set_mode(&air_status, MODE_COOL);
          statusChanged = true;
        }
        else if (message == "heat")
        {
          air_set_power_on(&air_status);
          yield();
          air_set_mode(&air_status, MODE_HEAT);
          statusChanged = true;
        }
        else if (message == "auto")
        {
          air_set_power_on(&air_status);
          yield();
          //air_set_mode(&air_status, MODE_AUTO);
          byte data[] = {0x40, 0x00, 0x11, 0x03, 0x08, 0x42, 0x05, 0x1d};
          air_send_data(&air_status, data, sizeof(data));
          byte data2[] = {0x40, 0x00, 0x15, 0x02, 0x08, 0x42, 0x1d};      
          air_send_data(&air_status, data2, sizeof(data2));
          statusChanged = true;
        }
        else if (message == "fan_only")
        {
          air_set_power_on(&air_status);
          air_set_mode(&air_status, MODE_FAN);
          statusChanged = true;
        }
        else if (message == "dry")
        {
          air_set_power_on(&air_status);
          yield();
          air_set_mode(&air_status, MODE_DRY);
          statusChanged = true;
        }
      }
    }
  }

  if (String(topic) == fan_mode_command_topic)
  {

    if (message == "low" || message == "medium" || message == "high" || message == "auto")
    {
      // Handle fan speed commands
      print_logf("[MQTT] Fan speed command: %s", message.c_str());
      if (simulation_mode) {
        if (message == "low")
        {
          air_status.fan = FAN_LOW;
          strcpy(air_status.fan_str, "LOW");
          print_log("[SIMULATION MODE] Fan set to LOW via MQTT");
          statusChanged = true;
        }
        else if (message == "medium")
        {
          air_status.fan = FAN_MEDIUM;
          strcpy(air_status.fan_str, "MED");
          print_log("[SIMULATION MODE] Fan set to MEDIUM via MQTT");
          statusChanged = true;
        }
        else if (message == "high")
        {
          air_status.fan = FAN_HIGH;
          strcpy(air_status.fan_str, "HIGH");
          print_log("[SIMULATION MODE] Fan set to HIGH via MQTT");
          statusChanged = true;
        }
        else if (message == "auto")
        {
          air_status.fan = FAN_AUTO;
          strcpy(air_status.fan_str, "AUTO");
          print_log("[SIMULATION MODE] Fan set to AUTO via MQTT");
          statusChanged = true;
        }
      } else {
        if (message == "low")
        {
          air_set_fan(&air_status, FAN_LOW);
          statusChanged = true;
        }
        else if (message == "medium")
        {
          air_set_fan(&air_status, FAN_MEDIUM);
          statusChanged = true;
        }
        else if (message == "high")
        {
          air_set_fan(&air_status, FAN_HIGH);
          statusChanged = true;

        }
        else if (message == "auto")
        {
          air_set_fan(&air_status, FAN_AUTO);
          statusChanged = true;

        }
      } //end no test mode
    }

  }
    if (statusChanged)
    {
      if (simulation_mode) {
        print_log("[MQTT] command processed successfully (SIMULATION MODE)");
      } else {
        print_log("[MQTT] command processed successfully");
      }
      notifyWebSocketClients(); // Update status update to websocket clients
      yield();
      //my_timer_start(&timerMQTT); // Restart MQTT timer to ensure next update
      timerMQTT.start = 0; // Force timer to be ready
      handleMQTT(); // Call handleMQTT to send updated status to HA
    }
}

// climate:
//   - name: "Toshiba Air Conditioner"
//     unique_id: "toshiba_air_conditioner"
    
//     # Device information
//     device:
//       identifiers:
//         - "toshiba_ac"
//       name: "Toshiba AC"
//       model: "ESP8266"
//       manufacturer: "Custom"
    
//     # Supported modes
//     modes:
//       - "off"
//       - "fan_only"
//       - "cool"
//       - "dry"
//       - "heat"
//       - "auto"
    
//     # Supported fan modes
//     fan_modes:
//       - "low"
//       - "medium"
//       - "high"
//       - "auto"
    
//     # MQTT Topics
//     temperature_command_topic: "homeassistant/ac/set/temperature"
//     temperature_state_topic: "homeassistant/ac/status/temperature"
//     mode_command_topic: "homeassistant/ac/set/mode"
//     mode_state_topic: "homeassistant/ac/status/mode"
//     fan_mode_command_topic: "homeassistant/ac/set/fan_mode"
//     fan_mode_state_topic: "homeassistant/ac/status/fan_mode"
//     current_temperature_topic: "homeassistant/ac/status/current_temperature"
    
//     # Temperature settings
//     min_temp: 16
//     max_temp: 30
//     temp_step: 1.0
//     temperature_unit: "C"
//     precision: 0.5
    
//     # MQTT settings
//     retain: false
//     qos: 1


    void sendMQTTDiscovery()
    {
      print_log("[MQTT] Sending discovery messages...");

      // Helper function to publish with retry
      auto publishWithRetry = [](const char *topic, const String &payload) -> bool
      {
        for (int i = 0; i < 3; i++)
        {
          if (mqttClient.publish(topic, payload.c_str(), true))  //retain true
          {
            return true;
          }
          //delay(100);
          yield();
        }
        return false;
      };

      // Climate device discovery
      JsonDocument climateDoc;
      climateDoc["name"] = "Toshiba Air Conditioner";
      climateDoc["unique_id"] = "toshiba_air_conditioner";

      // Device information
      JsonObject climateDevice = climateDoc["device"].to<JsonObject>();
      climateDevice["identifiers"][0] = "toshiba_ac";
      climateDevice["name"] = "Toshiba AC";
      climateDevice["model"] = "ESP8266";
      climateDevice["manufacturer"] = "Custom";

      // Supported modes
      JsonArray modes = climateDoc["modes"].to<JsonArray>();
      modes.add("off");
      modes.add("fan_only");
      modes.add("cool");
      modes.add("dry");
      modes.add("heat");
      modes.add("auto");

      // Supported fan modes
      JsonArray fanModes = climateDoc["fan_modes"].to<JsonArray>();
      fanModes.add("low");
      fanModes.add("medium");
      fanModes.add("high");
      fanModes.add("auto");

      // Topics
      climateDoc["temperature_command_topic"] = temp_command_topic;
      climateDoc["temperature_state_topic"] = target_temp_topic;
      climateDoc["mode_command_topic"] = mode_command_topic;
      climateDoc["mode_state_topic"] = mode_topic;
      climateDoc["fan_mode_command_topic"] = fan_mode_command_topic;
      climateDoc["fan_mode_state_topic"] = fan_mode_topic;
      climateDoc["current_temperature_topic"] = temp_topic;

      // Temperature settings
      climateDoc["min_temp"] = 16;
      climateDoc["max_temp"] = 30;
      climateDoc["temp_step"] = 1.0; //change target temperature in 1 degree steps
      climateDoc["temperature_unit"] = "C";
      climateDoc["precision"] = 0.5; //temperature from AC sensor has 0.5 precision

      // MQTT settings
      climateDoc["retain"] = false;
      climateDoc["qos"] = 1;

      // Publish discovery
      String climateConfig;
      serializeJson(climateDoc, climateConfig);
      bool climateResult = publishWithRetry(climate_conf_topic.c_str(), climateConfig);
      print_logf("[MQTT] Climate discovery: %s (size: %d)", climateResult ? "SUCCESS" : "FAILED", climateConfig.length());
    }

    // mqtt configuration from web
    bool testMQTTConnection()
    {
      print_log("[MQTT] Testing connection...");

      // Try to connect to MQTT broker
      if (WiFi.status() != WL_CONNECTED)
      {
        print_log("[MQTT] WiFi not connected");
        return false;
      }

      // Test connection
      WiFiClient testClient;
      if (!testClient.connect(mqtt_server_str.c_str(), mqtt_port))
      {
        print_log("[MQTT] Cannot reach MQTT server");
        return false;
      }
      testClient.stop();

      // Try MQTT connection
      PubSubClient testMqtt(testClient);
      testMqtt.setServer(mqtt_server_str.c_str(), mqtt_port);

      if (testMqtt.connect(device_name_str.c_str(), mqtt_user_str.c_str(), mqtt_password_str.c_str()))
      {
        print_log("[MQTT] Test connection successful");
        testMqtt.disconnect();
        return true;
      }
      else
      {
        print_logf("[MQTT] Test connection failed, rc=%d", testMqtt.state());
        return false;
      }
    }

    void resetMQTTDiscovery()
    {
      print_log("[MQTT] Resetting discovery...");

      if (!mqttClient.connected())
      {
        print_log("[MQTT] Not connected, cannot reset discovery");
        return;
      }

      // Send empty payloads to remove discovery
      //String climateTopic = "homeassistant/climate/" + device_name_str + "/config";      
      String climateTopic=climate_conf_topic;
      mqttClient.publish(climateTopic.c_str(), "", true);
      
      print_log("[MQTT] Discovery reset messages sent");
    }

    // Getter functions for current MQTT configuration
    String getMQTTServer()
    {
      return mqtt_server_str;
    }

    int getMQTTPort()
    {
      return mqtt_port;
    }

    String getMQTTUser()
    {
      return mqtt_user_str;
    }

    String getMQTTPassword()
    {
      return mqtt_password_str;
    }

    String getMQTTDeviceName()
    {
      return device_name_str;
    }