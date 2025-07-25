/*
GNU GENERAL PUBLIC LICENSE

Version 2, June 1991

Copyright (C) 1989, 1991 Free Software Foundation, Inc.  
51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

Everyone is permitted to copy and distribute verbatim copies
of this license document, but changing it is not allowed.

*/

#include "config.h"
#include "process_request.h" 
#include "print_log.h"

extern air_status_t air_status;
extern WebSocketsServer webSocket;

extern MySimpleTimer timerOnOff;
//File fsUploadFile;                                    // a File variable to temporarily store the received file

extern MySimpleTimer timerTemperature;
extern int temp_interval;

extern MySimpleTimer timerAutonomous;
extern bool autonomous_mode;

extern const int DHTPin;

extern float ac_sensor_temperature[MAX_LOG_DATA];
extern int ac_outdoor_te[MAX_LOG_DATA];

extern float sensor_temperature[MAX_LOG_DATA];
extern float sensor_humidity[MAX_LOG_DATA];
extern float sensor_pressure[MAX_LOG_DATA];
extern unsigned long timestamps[MAX_LOG_DATA];
extern int temp_idx;
extern float sensor_temperature_current;
extern float sensor_humidity_current;
extern float sensor_pressure_current;


#ifdef USE_AHT20
extern float aht_t_current;
extern bool humidity_status;
#endif

#ifdef USE_BMP280
extern float bmp280_t_current;
extern bool pressure_status;
#endif

// Define NTP Client to get time
extern WiFiUDP ntpUDP;
extern const long utcOffsetInSeconds;
extern int timeOffset;
extern NTPClient timeClient;

extern MySimpleTimer timerStatus;
extern MySimpleTimer timerReadSerial;
extern MySimpleTimer timerSaveFile;


extern const char compile_date[];

#ifdef USE_MQTT
#include "mqtt.h"
#endif


/*__________________________________________________________HELPER_FUNCTIONS__________________________________________________________*/


void serialize_array_float(float *ptr, JsonArray &arr, int idx) {
  int i;
  if (idx > MAX_LOG_DATA) idx = 0;

  for (i = 0; i < MAX_LOG_DATA; i++) {
    //Serial.printf("%.2f ", ptr[(idx+i)%MAX_LOG_DATA]);
    arr.add(ptr[(idx + i) % MAX_LOG_DATA]);
  }
}

void serialize_array_ul_int(unsigned long *ptr, JsonArray &arr, int idx) {
  int i;
  if (idx > MAX_LOG_DATA) idx = 0;

  for (i = 0; i < MAX_LOG_DATA; i++) {
    //Serial.printf("%.2f ", ptr[(idx+i)%MAX_LOG_DATA]);
    arr.add(ptr[(idx + i) % MAX_LOG_DATA]);
  }
}

void serialize_array_int(int *ptr, JsonArray &arr, int idx) {
  int i;
  if (idx > MAX_LOG_DATA) idx = 0;

  for (i = 0; i < MAX_LOG_DATA; i++) {
    arr.add(ptr[(idx + i) % MAX_LOG_DATA]);
  }
}



/*__________________________________________________________JSON_FUNCTIONS__________________________________________________________*/

String air_to_json(air_status_t *air)
{
  String response;
  JsonDocument jsonDoc;

  jsonDoc["id"] = "status";
  jsonDoc["save"] = air->save;
  jsonDoc["heat"] = air->heat;
  jsonDoc["preheat"] = air->preheat;
  jsonDoc["cold"] = air->cold;
  jsonDoc["target_temperature"] = air->target_temp;
  jsonDoc["ac_sensor_temperature"] = air->remote_sensor_temp;
  jsonDoc["fan"] = air->fan_str;
  jsonDoc["mode"] = air->mode_str;
  jsonDoc["power"] = air->power;

  jsonDoc["master"] = air->master;
  jsonDoc["remote"] = air->remote;
  jsonDoc["observed_master"] = air->observed_master;
  //jsonDoc["observed_remote"] = air->observed_remote;
  // Add all observed remotes as an array
  JsonArray observedRemotesArr = jsonDoc["observed_remotes"].to<JsonArray>();
  for (uint8_t i = 0; i < air->observed_remotes_count; i++) {
    observedRemotesArr.add(air->observed_remotes[i]);
  }
  
  jsonDoc["boot_time"] = air->boot_time;
  jsonDoc["filter_alert"] = air->filter_alert;

  //indoor unit data
  jsonDoc["indoor_room_temp"] = air->indoor_room_temp;
  jsonDoc["indoor_ta"] = air->indoor_ta;
  jsonDoc["indoor_tcj"] = air->indoor_tcj;
  jsonDoc["indoor_tc"] = air->indoor_tc;
  jsonDoc["indoor_filter_time"] = air->indoor_filter_time;
  jsonDoc["indoor_fan_run_time"] = air->indoor_fan_run_time;

  //outdoor unit data
  jsonDoc["outdoor_te"] = air->outdoor_te;
  jsonDoc["outdoor_to"] = air->outdoor_to;
  jsonDoc["outdoor_td"] = air->outdoor_td;
  jsonDoc["outdoor_ts"] = air->outdoor_ts;
  jsonDoc["outdoor_ths"] = air->outdoor_ths;
  jsonDoc["outdoor_current"] = air->outdoor_current;
  jsonDoc["power_consumption"] = air->power_consumption;
  jsonDoc["outdoor_cumhour"] = air->outdoor_cumhour;

  jsonDoc["indoor_fan_speed"] = air->indoor_fan_speed;

  jsonDoc["outdoor_tl"] = air->outdoor_tl;
  jsonDoc["outdoor_comp_freq"] = air->outdoor_comp_freq;
  jsonDoc["outdoor_lower_fan_speed"] = air->outdoor_lower_fan_speed;
  jsonDoc["outdoor_upper_fan_speed"] = air->outdoor_upper_fan_speed;

  jsonDoc["timer_mode"] = air->timer_mode_req;
  //jsonDoc["timer_enabled"] = timerAC.isEnabled();
  //jsonDoc["timer_pending"] = timerAC.pendingTime();
  //jsonDoc["timer_time"] = timerAC.getInterval();
  jsonDoc["timer_enabled"] = my_timer_is_enabled(&timerOnOff);
  jsonDoc["timer_pending"] = my_timer_pending_time(&timerOnOff);
  jsonDoc["timer_time"] = my_timer_get_interval(&timerOnOff);

  jsonDoc["sampling"] = temp_interval;
  jsonDoc["decode_errors"] = air->decode_errors;

  jsonDoc["heap"] = ESP.getFreeHeap();
  jsonDoc["compile"] = compile_date;
  jsonDoc["ip"] = air->ip;

  jsonDoc["mqtt"] = air->mqtt;
  jsonDoc["autonomous"] = autonomous_mode;

  #ifdef USE_AHT20
  jsonDoc["sensor_temperature"] = aht_t_current;
  jsonDoc["sensor_humidity"] = sensor_humidity_current;
  jsonDoc["aht20_status"] = humidity_status;
  #endif
  
  #ifdef USE_BMP280
  jsonDoc["sensor_temperature"] = bmp280_t_current;
  jsonDoc["sensor_pressure"] = sensor_pressure_current;
  jsonDoc["bmp280_status"] = pressure_status;
  #endif

  int i;
  String str;
  for (i = 0; (i < MAX_CMD_BUFFER) && (i < (air->last_cmd[3] + 5)) ; i++)
    str += ((air->last_cmd[i] < 0x10) ? "0" : "")  + String(air->last_cmd[i], HEX) + " ";

  str = "";

  for (i = 0; (i < MAX_RX_BUFFER) && (i < air->curr_w_idx); i++) //not round buffer
  {
    str += ((air->rx_data[i] < 0x10) ? "0" : "")  + String(air->rx_data[i], HEX) + " ";
  }

  jsonDoc["last_cmd"] = air->buffer_cmd;
  jsonDoc["last_cmd_type"] = air->last_cmd_type;
  jsonDoc["rx_data"] = air->buffer_rx;

  serializeJson(jsonDoc, response);

  return response;
}


String string_to_json(String t)
{
  String str;
  String response;
  JsonDocument jsonDoc;
  jsonDoc["id"] = "txt";
  jsonDoc["txt"] = str;
  serializeJson(jsonDoc, response);

  return response;
}


String timeseries_to_json(String id, String val, void *data, int data_type, int temp_idx) {
  JsonDocument jsonDoc;
  String message = "";

  jsonDoc.clear();

  jsonDoc["id"] = id;
  jsonDoc["n"] = MAX_LOG_DATA;
  jsonDoc["val"] = val;


  JsonArray jsonArr = jsonDoc[val].to<JsonArray>();

  switch (data_type) {
    case 0:
      serialize_array_float((float*)data, jsonArr, temp_idx);
      break;
    case 1:
      serialize_array_int((int*)data, jsonArr, temp_idx);
      break;
    case 2:
      serialize_array_ul_int((unsigned long*)data, jsonArr, temp_idx);
      break;
    default:
      Serial.printf("[ERROR] Unknown data type %d\n", data_type);
      break;
  }

  serializeJson(jsonDoc, message);
  jsonDoc.clear();

  return message;
}

void processRequest( uint8_t *  payload) {
  //decode json
  JsonDocument jsonDoc;

  DeserializationError error = deserializeJson(jsonDoc, payload);
  if (error) {
    Serial.printf("[JSON] Could not deserialize JSON %s\n", payload);
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
      if (val < 18) val = 18;
      air_set_temp(&air_status, val);
      //} else if (jsonDoc["temp"] == "0") { //
    } else if (val == 0) {
      val = air_status.target_temp - 1;
      if (val > 30) val = 30;
      if (val < 18) val = 18;
      air_set_temp(&air_status, val);
    } else {
      if (val > 30) val = 30;
      if (val < 18) val = 18;
      air_set_temp(&air_status, val);
    }
  }
  else if (jsonDoc["id"] == "timer") {
    int val = atoi(jsonDoc["timer_time"]); //time is a string :)
    if (val == TIMER_SW_RESET) {
      //timerAC.disable();
      my_timer_disable(&timerOnOff);
      air_status.timer_mode_req = jsonDoc["timer_mode"];
      air_status.timer_time_req = 0;
    }
    else {
      //timerAC.setInterval(val);
      //timerAC.start();
      my_timer_set_interval(&timerOnOff, val);
      my_timer_start(&timerOnOff);
      air_status.timer_mode_req = jsonDoc["timer_mode"];
      air_status.timer_time_req = val;
    }
  }
  else if (jsonDoc["id"] == "mode") {
    if (jsonDoc["value"] == "cool") {
      air_set_mode(&air_status, MODE_COOL);
    } else if (jsonDoc["value"] == "dry") {
      air_set_mode(&air_status, MODE_DRY);
    } else if (jsonDoc["value"] == "fan") {
      air_set_mode(&air_status, MODE_FAN);
    } else if (jsonDoc["value"] == "heat") {
      air_set_mode(&air_status, MODE_HEAT);
    } else if (jsonDoc["value"] == "auto") {      
      byte data[] = {0x40, 0x00, 0x11, 0x03, 0x08, 0x42, 0x05, 0x1d};
      air_send_data(&air_status, data, sizeof(data));
      byte data2[] = {0x40, 0x00, 0x15, 0x02, 0x08, 0x42, 0x1d};      
      air_send_data(&air_status, data2, sizeof(data2));
    }
  }
  else if (jsonDoc["id"] == "fan") {
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
    if (jsonDoc["value"] == "1") {
      air_set_save_on(&air_status);
    } else if (jsonDoc["value"] == "0") {
      air_set_save_off(&air_status);
    }
  }
  //sets hw timer
  else if (jsonDoc["id"] == "timer_hw") {
    int val = atoi(jsonDoc["time"]);
    if (jsonDoc["value"] == "cancel") {
      air_set_timer(&air_status, TIMER_HW_CANCEL, val);
    } else if (jsonDoc["value"] == "off") {
      air_set_timer(&air_status, TIMER_HW_OFF, val);
    } else if (jsonDoc["value"] == "repeat_off") {
      air_set_timer(&air_status, TIMER_HW_REPEAT_OFF, val);
    } else if (jsonDoc["value"] == "on") {
      air_set_timer(&air_status, TIMER_HW_ON, val);
    }
  }
  //status sends current values
  else if (jsonDoc["id"] == "status") {
    String message;
    Serial.println(message);
    message = air_to_json(&air_status);
    webSocket.broadcastTXT(message);
    //reset serial buffers
    air_status.buffer_cmd = String("");
    air_status.buffer_rx = String("");
  }
  //sets sampling period for data logging
  else if (jsonDoc["id"] == "sampling") {
    temp_interval = jsonDoc["value"];
    // timerTemperature.setUnit(1000);
    // timerTemperature.setInterval(temp_interval);
    // timerTemperature.repeat();
    // timerTemperature.start();
    my_timer_set_unit(&timerTemperature, 1000);
    my_timer_set_interval(&timerTemperature, temp_interval);
    my_timer_repeat(&timerTemperature);
    my_timer_start(&timerTemperature);
  }

  else if (jsonDoc["id"] == "timeseries") {
    //send data but not in a whole BIG json
    String message = "";

    message = timeseries_to_json("timeseries", "timestamp", timestamps, 2, temp_idx);
    webSocket.broadcastTXT(message);

    // if there is an alternative temperature sensor
    #if defined(USE_AHT20) || defined(USE_BMP280)
    message = timeseries_to_json("timeseries", "sensor_temperature", sensor_temperature, 0, temp_idx);
    webSocket.broadcastTXT(message);
    #endif

    #ifdef USE_AHT20
    message = timeseries_to_json("timeseries", "sensor_humidity", sensor_humidity, 0, temp_idx);
    webSocket.broadcastTXT(message);
    #endif
    #ifdef USE_BMP280
    message = timeseries_to_json("timeseries", "sensor_pressure", sensor_pressure, 0, temp_idx);
    webSocket.broadcastTXT(message);
    #endif

    message = timeseries_to_json("timeseries", "ac_external_temperature", ac_outdoor_te, 1, temp_idx);
    webSocket.broadcastTXT(message);

    message = timeseries_to_json("timeseries", "ac_sensor_temperature", ac_sensor_temperature, 0, temp_idx);
    webSocket.broadcastTXT(message);

  } //end if timeseries

#ifdef USE_MQTT
    else if (jsonDoc["id"] == "mqtt_config") {
      // Handle MQTT configuration
      String server = jsonDoc["server"];
      int port = jsonDoc["port"];
      String username = jsonDoc["username"];
      String password = jsonDoc["password"];
      String device_name = jsonDoc["device_name"];
      
      Serial.printf("MQTT Config received: %s:%d, user:%s, device:%s\n", 
                    server.c_str(), port, username.c_str(), device_name.c_str());
      
      // Configure MQTT with new settings
      configureMQTT(server.c_str(), port, username.c_str(), password.c_str(), device_name.c_str());
      
      // Send response
      JsonDocument responseDoc;
      responseDoc["id"] = "mqtt_config";
      responseDoc["status"] = "saved";
      responseDoc["message"] = "MQTT configuration updated successfully";
      
      String response;
      serializeJson(responseDoc, response);
      webSocket.broadcastTXT(response);
    }
    else if (jsonDoc["id"] == "mqtt_test") {
      // Handle MQTT connection test
      Serial.println("Testing MQTT connection...");
      
      // Test MQTT connection
      bool testResult = testMQTTConnection();
      
      // Send response
      JsonDocument responseDoc;
      responseDoc["id"] = "mqtt_test";
      if (testResult) {
        responseDoc["status"] = "connected";
        responseDoc["message"] = "MQTT connection successful";
      } else {
        responseDoc["status"] = "failed";
        responseDoc["message"] = "MQTT connection failed";
      }
      
      String response;
      serializeJson(responseDoc, response);
      webSocket.broadcastTXT(response);
    }
    else if (jsonDoc["id"] == "mqtt_get_config") {
      // Handle request for current MQTT configuration
      Serial.println("Sending current MQTT configuration...");
      
      JsonDocument responseDoc;
      responseDoc["id"] = "status";
      
      // Add MQTT configuration to the response
      JsonObject mqttConfig = responseDoc["mqtt_config"].to<JsonObject>();
      mqttConfig["server"] = getMQTTServer();
      mqttConfig["port"] = getMQTTPort();
      mqttConfig["username"] = getMQTTUser();
      mqttConfig["password"] = getMQTTPassword();
      mqttConfig["device_name"] = getMQTTDeviceName();
      
      String response;
      serializeJson(responseDoc, response);
      webSocket.broadcastTXT(response);
    }
    else if (jsonDoc["id"] == "mqtt_discovery") {
      String value = jsonDoc["value"];
      
      if (value == "send") {
        // Send MQTT discovery messages
        Serial.println("Sending MQTT discovery messages...");
        sendMQTTDiscovery();
        
        JsonDocument responseDoc;
        responseDoc["id"] = "mqtt_discovery";
        responseDoc["status"] = "sent";
        responseDoc["message"] = "Discovery messages sent to Home Assistant";
        
        String response;
        serializeJson(responseDoc, response);
        webSocket.broadcastTXT(response);
        
      } else if (value == "reset") {
        // Reset/remove MQTT discovery
        Serial.println("Resetting MQTT discovery...");
        resetMQTTDiscovery();
        
        JsonDocument responseDoc;
        responseDoc["id"] = "mqtt_discovery";
        responseDoc["status"] = "reset";
        responseDoc["message"] = "Discovery messages reset";
        
        String response;
        serializeJson(responseDoc, response);
        webSocket.broadcastTXT(response);
      }
    }
#endif //USE_MQTT


  //handle raw bytes command
  else if (jsonDoc["id"] == "raw_bytes") {
    Serial.println("Processing raw bytes command...");
    
    // Get the bytes array from JSON
    JsonArray bytesArray = jsonDoc["bytes"];
    if (bytesArray.size() == 0) {
      Serial.println("Error: No bytes provided");
      return;
    }
    
    // Convert JSON array to byte array
    byte rawData[bytesArray.size()];
    for (size_t i = 0; i < bytesArray.size(); i++) {
      rawData[i] = bytesArray[i];
    }
    
    // Print debug info
    Serial.printf("Sending %d raw bytes: ", bytesArray.size());
    for (size_t i = 0; i < bytesArray.size(); i++) {
      Serial.printf("%02X ", rawData[i]);
    }
    Serial.println();
    
    // Send the raw data using air_send_data
    air_send_data(&air_status, rawData, bytesArray.size());
    
    // Send confirmation response
    JsonDocument responseDoc;
    responseDoc["id"] = "raw_bytes";
    responseDoc["status"] = "sent";
    responseDoc["bytes_count"] = bytesArray.size();
    
    String response;
    serializeJson(responseDoc, response);
    webSocket.broadcastTXT(response);
  }

  // handle autonomous mode
  else if (jsonDoc["id"] == "autonomous") {
    if (jsonDoc["value"] == "start") {
      autonomous_mode = true;
      my_timer_start(&timerAutonomous);
      Serial.println("[AUTO] Autonomous mode started");
      
      // Send confirmation response
      JsonDocument responseDoc;
      responseDoc["id"] = "autonomous";
      responseDoc["status"] = "started";
      responseDoc["message"] = "Autonomous mode activated";
      
      String response;
      serializeJson(responseDoc, response);
      webSocket.broadcastTXT(response);
      
    } else if (jsonDoc["value"] == "stop") {
      autonomous_mode = false;
      my_timer_disable(&timerAutonomous);
      Serial.println("[AUTO] Autonomous mode stopped");
      
      // Send confirmation response
      JsonDocument responseDoc;
      responseDoc["id"] = "autonomous";
      responseDoc["status"] = "stopped";
      responseDoc["message"] = "Autonomous mode deactivated";
      
      String response;
      serializeJson(responseDoc, response);
      webSocket.broadcastTXT(response);
    }
  }
  // handle address synchronization and reset
  else if (jsonDoc["id"] == "sync_addresses") {
    // Copy observed addresses to active addresses
    if (air_status.observed_master != 0xFF) {
      air_status.master = air_status.observed_master;
    }
    // Use the first observed remote if available
    if (air_status.observed_remotes_count > 0) {
      air_status.remote = air_status.observed_remotes[0];
    }
    
    print_logf("Address sync: Master=0x%02X, Remote=0x%02X\n", 
               air_status.master, air_status.remote);
    
    // Send confirmation response
    JsonDocument responseDoc;
    responseDoc["id"] = "sync_addresses";
    responseDoc["status"] = "success";
    responseDoc["message"] = "Addresses synchronized";
    responseDoc["master"] = air_status.master;
    responseDoc["remote"] = air_status.remote;
    
    // Also send all observed remotes
    JsonArray observedRemotesArr = responseDoc["observed_remotes"].to<JsonArray>();
    for (uint8_t i = 0; i < air_status.observed_remotes_count; i++) {
      observedRemotesArr.add(air_status.observed_remotes[i]);
    }

    String response;
    serializeJson(responseDoc, response);
    webSocket.broadcastTXT(response);
  }
  
  else if (jsonDoc["id"] == "reset_addresses") {
    // Reset to default addresses
    air_status.master = MASTER;   // 0x00
    air_status.remote = REMOTE;   // 0x40
    
    print_logf("Address reset: Master=0x%02X, Remote=0x%02X\n", 
               air_status.master, air_status.remote);
    
    // Send confirmation response
    JsonDocument responseDoc;
    responseDoc["id"] = "reset_addresses";
    responseDoc["status"] = "success";
    responseDoc["message"] = "Addresses reset to default";
    responseDoc["master"] = air_status.master;
    responseDoc["remote"] = air_status.remote;
    
    String response;
    serializeJson(responseDoc, response);
    webSocket.broadcastTXT(response);
  }
  
  else if (jsonDoc["id"] == "set_addresses") {
    // Set manual addresses
    bool changed = false;
    
    if (jsonDoc["master"].is<int>()) {
      air_status.master = jsonDoc["master"];
      changed = true;
    }
    if (jsonDoc["remote"].is<int>()) {
      air_status.remote = jsonDoc["remote"];
      changed = true;
    }
    
    if (changed) {
      print_logf("Manual address set: Master=0x%02X, Remote=0x%02X\n", 
                 air_status.master, air_status.remote);
      
      // Send confirmation response
      JsonDocument responseDoc;
      responseDoc["id"] = "set_addresses";
      responseDoc["status"] = "success";
      responseDoc["message"] = "Addresses set manually";
      responseDoc["master"] = air_status.master;
      responseDoc["remote"] = air_status.remote;
      
      String response;
      serializeJson(responseDoc, response);
      webSocket.broadcastTXT(response);
    }
  }
  else if (jsonDoc["id"] == "reset_observed_remotes") {
    // Reset observed remotes
    reset_observed_remotes(&air_status);
    
    print_logf("Observed remotes reset\n");
    
    // Send confirmation response
    JsonDocument responseDoc;
    responseDoc["id"] = "reset_observed_remotes";
    responseDoc["status"] = "success";
    responseDoc["message"] = "Observed remotes cleared";
    responseDoc["observed_remotes_count"] = 0;
    
    // Empty array
    responseDoc["observed_remotes"].to<JsonArray>();
    
    String response;
    serializeJson(responseDoc, response);
    webSocket.broadcastTXT(response);
  }
}

void notifyWebSocketClients() {
    String message;
    Serial.println(message);
    message = air_to_json(&air_status);
    webSocket.broadcastTXT(message);
    //reset serial buffers
    air_status.buffer_cmd = String("");
    air_status.buffer_rx = String("");
}
