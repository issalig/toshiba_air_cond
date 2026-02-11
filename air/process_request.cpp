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
#include "settings.h" 
#include <LittleFS.h>

extern air_status_t air_status;
#ifdef USE_ASYNC
extern AsyncWebSocket webSocket;
#else
extern WebSocketsServer webSocket;
#endif

extern my_timer timerOnOff;
// File fsUploadFile;                                    // a File variable to temporarily store the received file

extern my_timer timerTemperature;
extern int temperature_interval;

extern my_timer timerAutonomous;
extern bool autonomous_mode;

extern bool listen_mode;

extern my_timer timerMQTT;

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

extern my_timer timerStatus;
extern my_timer timerReadSerial;
extern my_timer timerSaveFile;


extern const char compile_date[];

#ifdef USE_MQTT
#include "mqtt.h"
#endif

extern bool simulation_mode;
void initSimulationMode();

void air_set_serial_config(air_status_t *air, uint8_t tx_pin, uint8_t rx_pin, uint8_t rx_buffer_size);
void air_set_i2c_config(air_status_t *air, uint8_t sda_pin, uint8_t scl_pin);


/*__________________________________________________________HELPER_FUNCTIONS__________________________________________________________*/

void serialize_array_float(float *ptr, JsonArray &arr, int idx)
{
  int i;
  if (idx > MAX_LOG_DATA)
    idx = 0;

  for (i = 0; i < MAX_LOG_DATA; i++)
  {
    // adds from the oldest to the newest
    arr.add(ptr[(idx + i) % MAX_LOG_DATA]);
  }
}

void serialize_array_ul_int(unsigned long *ptr, JsonArray &arr, int idx)
{
  int i;
  if (idx > MAX_LOG_DATA)
    idx = 0;

  for (i = 0; i < MAX_LOG_DATA; i++)
  {
    // adds from the oldest to the newest
    arr.add(ptr[(idx + i) % MAX_LOG_DATA]);
  }
}

void serialize_array_int(int *ptr, JsonArray &arr, int idx)
{
  int i;
  if (idx > MAX_LOG_DATA)
    idx = 0;

  for (i = 0; i < MAX_LOG_DATA; i++)
  {
    // adds from the oldest to the newest
    arr.add(ptr[(idx + i) % MAX_LOG_DATA]);
  }
}

String getCommandTypeString(uint8_t cmdType) {
    switch(cmdType) {
        case  0: return "[UNKNOWN]";
        case  1: return "[SENSOR_ERROR]";
        case  2: return "[STATUS]";
        case  3: return "[STATUS_EXT]";
        case  4: return "[MASTER_ACK]";
        case  5: return "[MASTER_ALIVE]";
        case  6: return "[STATUS_MODE]";
        case  7: return "[SENSOR_ANS]";
        case  8: return "[MODEL]";
        case  9: return "[TEMP_LIMITS]";
        case 10: return "[FEATURES]";
        case 11: return "[SETPOINT]";
        case 12: return "[DN_CODE]";
        case 13: return "[MASTER_BUSY]";
        case 14: return "[ANNOUNCE]";
        case 15: return "[SAVE_RATIO]";
        case 16: return "[SAVE_RATIO_ANSWER]";
        case 17: return "[TIME_COUNTER]";
        case 18: return "[DN_CODE_REQUEST]";
        case 19: return "[FAN_MODES_REQ]";
        case 20: return "[FAN_MODES_ANS]";
        case 21: return "[REMOTE_TEMP]";
        case 22: return "[REMOTE_PING]";
        case 23: return "[MASTER_PONG]";
        case 24: return "[MODEL_REQUEST]";
        case 25: return "[SETPOINT_REQUEST]";
        case 26: return "[TEMP_LIMITS_REQUEST]";
        case 27: return "[SENSOR_QUERY]";
        case 28: return "[POWER]";
        case 29: return "[SET_TEMP_FAN]";
        case 30: return "[SET_MODE]";
        case 31: return "[TIME_COUNTER_ANSWER]";
        default: return "[TYPE_" + String(cmdType) + "]";
    }
}

/*__________________________________________________________JSON_FUNCTIONS__________________________________________________________*/

String air_to_json(air_status_t *air)
{
  String response;
  JsonDocument jsonDoc;

  jsonDoc["id"] = "status";

  jsonDoc["power"] = air->power;
  jsonDoc["mode"] = air->mode_str;
  jsonDoc["fan"] = air->fan_str;
  jsonDoc["heat"] = air->heat;
  jsonDoc["cold"] = air->cold;
  jsonDoc["save"] = air->save;
  jsonDoc["preheat"] = air->preheat;
  jsonDoc["filter_alert"] = air->filter_alert;

  jsonDoc["target_temperature"] = air->target_temp;
  jsonDoc["ac_sensor_temperature"] = air->remote_sensor_temp;

  // Add announcement state to status
  const char *announce_state_str;
  switch (air->announce_state)
  {
  case ANNOUNCE_UNLINKED:
    announce_state_str = "UNLINKED";
    break;
  case ANNOUNCE_BUSY:
    announce_state_str = "BUSY";
    break;
  case ANNOUNCE_LINKED:
    announce_state_str = "LINKED";
    break;
  default:
    announce_state_str = "UNKNOWN";
    break;
  }
  jsonDoc["announce_state"] = announce_state_str;

  jsonDoc["simulation_mode"] = simulation_mode;


  jsonDoc["boot_time"] = air->boot_time;

  jsonDoc["timer_mode"] = air->timer_mode_req;
  jsonDoc["timer_enabled"] = my_timer_is_enabled(&timerOnOff);
  jsonDoc["timer_pending"] = my_timer_pending_time(&timerOnOff);
  jsonDoc["timer_time"] = my_timer_get_interval(&timerOnOff);

  jsonDoc["sampling"] = temperature_interval;
  
  //jsonDoc["discarded_bytes"] = air->discarded_bytes;
  jsonDoc["discarded_msgs"] = air->discarded_msgs; 
  //jsonDoc["decoded_bytes"] = air->decoded_bytes;
  jsonDoc["decoded_msgs"] = air->decoded_msgs;
  jsonDoc["recovered_bytes"] = air->recovered_bytes;
  jsonDoc["recovered_msgs"] = air->recovered_msgs;

  jsonDoc["heap"] = ESP.getFreeHeap();
  jsonDoc["compile"] = compile_date;
  jsonDoc["ip"] = air->ip;

  jsonDoc["tx_pin"] = air->txpin;
  jsonDoc["rx_pin"] = air->rxpin;
  jsonDoc["rx_buffer_size"] = air->rxsize;

  jsonDoc["sda_pin"] = air->sda_pin;
  jsonDoc["scl_pin"] = air->scl_pin;

  jsonDoc["mqtt"] = air->mqtt;
  jsonDoc["mqtt_enabled"] = air->mqtt_enabled;

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
  for (i = 0; (i < MAX_CMD_BUFFER) && (i < (air->last_cmd[3] + 5)); i++)
    str += ((air->last_cmd[i] < 0x10) ? "0" : "") + String(air->last_cmd[i], HEX) + " ";

  str = "";

  for (i = 0; (i < MAX_RX_BUFFER) && (i < air->curr_w_idx); i++) // not round buffer
  {
    str += ((air->rx_data[i] < 0x10) ? "0" : "") + String(air->rx_data[i], HEX) + " ";
  }

  //jsonDoc["last_cmd"] = air->buffer_cmd;
  //jsonDoc["last_cmd_type"] = air->last_cmd_type;
  //jsonDoc["rx_data"] = air->buffer_raw;

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

#define JSON_FLOAT 0
#define JSON_INT 1
#define JSON_UL_INT 2

String timeseries_to_json(String id, String val, void *data, int data_type, int temp_idx)
{
  JsonDocument jsonDoc;
  String message = "";

  jsonDoc.clear();

  jsonDoc["id"] = id;
  jsonDoc["n"] = MAX_LOG_DATA;
  jsonDoc["val"] = val;

  JsonArray jsonArr = jsonDoc[val].to<JsonArray>();

  switch (data_type)
  {
  case JSON_FLOAT: // 0:
    serialize_array_float((float *)data, jsonArr, temp_idx);
    break;
  case JSON_INT: // 1:
    serialize_array_int((int *)data, jsonArr, temp_idx);
    break;
  case JSON_UL_INT: // 2:
    serialize_array_ul_int((unsigned long *)data, jsonArr, temp_idx);
    break;
  default:
    Serial.printf("[ERROR] Unknown data type %d\n", data_type);
    break;
  }

  serializeJson(jsonDoc, message);
  jsonDoc.clear();

  return message;
}

String txrx_data_to_json(air_status_t *air)
{
  String response;
  JsonDocument jsonDoc;

  jsonDoc["id"] = "txrx";
  jsonDoc["last_cmd"] = air->buffer_cmd;
  jsonDoc["last_cmd_type"] = air->last_cmd_type;
  jsonDoc["last_cmd_type_str"] = getCommandTypeString(air->last_cmd_type);
  jsonDoc["rx_data"] = air->buffer_raw;

  serializeJson(jsonDoc, response);
  
  // Clear buffers after sending
  air->buffer_cmd = String("");
  air->buffer_raw = String("");

  return response;
}

void notifyTXRXData() {
  // Only send if there's data to send
  if (air_status.buffer_cmd.length() > 0 || air_status.buffer_raw.length() > 0) {
    String message = txrx_data_to_json(&air_status);
    webSocket.broadcastTXT(message);
  }
}

String master_info_to_json(air_status_t *air)
{
  JsonDocument jsonDoc;
  String response;

  jsonDoc["id"] = "master_info";

  if (air->model_str[0] != '\0')
  {
    jsonDoc["model_str"] = air->model_str;
  }

  jsonDoc["temp_limit_min"] = air->temp_limit_min;
  jsonDoc["temp_limit_max"] = air->temp_limit_max;
  jsonDoc["temp_frost_protection"] = air->temp_frost_protection;

  jsonDoc["temp_default_auto"] = air->temp_default_auto;
  jsonDoc["temp_default_heat"] = air->temp_default_heat;
  jsonDoc["temp_default_dry"] = air->temp_default_dry;
  jsonDoc["temp_default_cool"] = air->temp_default_cool;

  serializeJson(jsonDoc, response);
  return response;
}

String sensors_to_json(air_status_t *air)
{
  JsonDocument jsonDoc;
  String response;

  jsonDoc["id"] = "sensors";

  // Indoor sensors
  jsonDoc["indoor_room_temp"] = air->indoor_room_temp;
  jsonDoc["indoor_ta"] = air->indoor_ta;
  jsonDoc["indoor_tcj"] = air->indoor_tcj;
  jsonDoc["indoor_tc"] = air->indoor_tc;
  jsonDoc["indoor_filter_time"] = air->indoor_filter_time;
  jsonDoc["indoor_fan_run_time"] = air->indoor_fan_run_time;
  jsonDoc["indoor_fan_speed"] = air->indoor_fan_speed;

  // Outdoor sensors
  jsonDoc["outdoor_te"] = air->outdoor_te;
  jsonDoc["outdoor_to"] = air->outdoor_to;
  jsonDoc["outdoor_td"] = air->outdoor_td;
  jsonDoc["outdoor_ts"] = air->outdoor_ts;
  jsonDoc["outdoor_ths"] = air->outdoor_ths;
  jsonDoc["outdoor_current"] = air->outdoor_current;
  jsonDoc["outdoor_cumhour"] = air->outdoor_cumhour;
  jsonDoc["outdoor_tl"] = air->outdoor_tl;
  jsonDoc["outdoor_comp_freq"] = air->outdoor_comp_freq;
  jsonDoc["outdoor_lower_fan_speed"] = air->outdoor_lower_fan_speed;
  jsonDoc["outdoor_upper_fan_speed"] = air->outdoor_upper_fan_speed;
  // jsonDoc["power_consumption"] = air->power_consumption;

  serializeJson(jsonDoc, response);
  return response;
}

String addresses_to_json(air_status_t *air)
{
  JsonDocument jsonDoc;
  String response;

  jsonDoc["id"] = "addresses";
  jsonDoc["master"] = air->master;
  jsonDoc["remote"] = air->remote;
  jsonDoc["observed_master"] = air->observed_master;

  JsonArray observedRemotesArr = jsonDoc["observed_remotes"].to<JsonArray>();
  for (uint8_t i = 0; i < air->observed_remotes_count; i++)
  {
    observedRemotesArr.add(air->observed_remotes[i]);
  }

  serializeJson(jsonDoc, response);
  return response;
}

String system_info_to_json(air_status_t *air)
{
  JsonDocument jsonDoc;
  String response;

  jsonDoc["id"] = "system_info";
  jsonDoc["boot_time"] = air->boot_time;
  jsonDoc["heap"] = ESP.getFreeHeap();
  jsonDoc["compile"] = compile_date;
  jsonDoc["ip"] = air->ip;
  //jsonDoc["discarded_bytes"] = air->discarded_bytes;

  serializeJson(jsonDoc, response);
  return response;
}

void processRequest(uint8_t *payload)
{
  // decode json
  JsonDocument jsonDoc;

  DeserializationError error = deserializeJson(jsonDoc, payload);
  if (error)
  {
    Serial.printf("[JSON] Could not deserialize JSON %s\n", payload);
    return;
  }

  if (jsonDoc["id"] == "power")
  { // power
    if (jsonDoc["value"] == "on")
    {
      if (simulation_mode)
      {
        air_status.power = true;
        Serial.println("[SIMULATION MODE] Power ON");
      }
      else
      {
        air_set_power_on(&air_status);
      }
    }
    else if (jsonDoc["value"] == "off")
    {
      if (simulation_mode)
      {
        air_status.power = false;
        Serial.println("[SIMULATION MODE] Power OFF");
      }
      else
      {
        air_set_power_off(&air_status);
      }
    }
  }
  else if (jsonDoc["id"] == "temp")
  {
    int val = atoi(jsonDoc["temp"]);

    if (val == 1)
    {
      val = air_status.target_temp + 1;
      if (val > 29)
        val = 29;
      if (val < 18)
        val = 18;
    }
    else if (val == 0)
    {
      val = air_status.target_temp - 1;
      if (val > 29)
        val = 29;
      if (val < 18)
        val = 18;
    }
    else
    {
      if (val > 29)
        val = 29;
      if (val < 18)
        val = 18;
    }

    if (simulation_mode)
    {
      air_status.target_temp = val;
      Serial.printf("[SIMULATION MODE] Temperature set to %d°C\n", val);
    }
    else
    {
      air_set_temp(&air_status, val);
    }
  }
  else if (jsonDoc["id"] == "timer")
  {
    int timer_val = atoi(jsonDoc["timer_time"]); // time is a string :)
    int timer_mode = jsonDoc["timer_mode"];      // timer_mode is an integer
    // TIMER_SW_OFF   0
    // TIMER_SW_ON    1
    // TIMER_SW_RESET 2

    if (timer_mode == TIMER_SW_RESET)
    {
      my_timer_disable(&timerOnOff);
      air_status.timer_mode_req = timer_mode;
      air_status.timer_time_req = 0;
      if (simulation_mode)
      {
        Serial.printf("[SIMULATION MODE] Timer reset (mode %d)\n", (int)air_status.timer_mode_req);
      }
    }
    else
    {
      my_timer_set_interval(&timerOnOff, timer_val);
      my_timer_start(&timerOnOff);
      air_status.timer_mode_req = timer_mode;
      air_status.timer_time_req = timer_val;
      if (simulation_mode)
      {
        Serial.printf("[SIMULATION MODE] Timer set: %d seconds, mode: %d\n", timer_val, (int)air_status.timer_mode_req);
      }
    }
  }
  else if (jsonDoc["id"] == "mode")
  {
    if (simulation_mode)
    {
      if (jsonDoc["value"] == "cool")
      {
        air_status.mode = MODE_COOL;
        strcpy(air_status.mode_str, "COOL");
        Serial.println("[SIMULATION MODE] Mode set to COOL");
      }
      else if (jsonDoc["value"] == "dry")
      {
        air_status.mode = MODE_DRY;
        strcpy(air_status.mode_str, "DRY");
        Serial.println("[SIMULATION MODE] Mode set to DRY");
      }
      else if (jsonDoc["value"] == "fan")
      {
        air_status.mode = MODE_FAN;
        strcpy(air_status.mode_str, "FAN");
        Serial.println("[SIMULATION MODE] Mode set to FAN");
      }
      else if (jsonDoc["value"] == "heat")
      {
        air_status.mode = MODE_HEAT;
        strcpy(air_status.mode_str, "HEAT");
        Serial.println("[SIMULATION MODE] Mode set to HEAT");
      }
      else if (jsonDoc["value"] == "auto")
      {
        air_status.mode = MODE_AUTO;
        strcpy(air_status.mode_str, "AUTO");
        Serial.println("[SIMULATION MODE] Mode set to AUTO");
      }
    }
    else
    {
      // Normal mode - send commands to AC
      if (jsonDoc["value"] == "cool")
      {
        air_set_mode(&air_status, MODE_COOL);
      }
      else if (jsonDoc["value"] == "dry")
      {
        air_set_mode(&air_status, MODE_DRY);
      }
      else if (jsonDoc["value"] == "fan")
      {
        air_set_mode(&air_status, MODE_FAN);
      }
      else if (jsonDoc["value"] == "heat")
      {
        air_set_mode(&air_status, MODE_HEAT);
      }
      else if (jsonDoc["value"] == "auto")
      {
        air_set_mode(&air_status, MODE_AUTO); //should be 5 but reports as 6?

        //byte data2[] = {0x40, 0x00, 0x15, 0x02, 0x08, 0x42, 0x1d}; // sent after setting auto ??
        //air_send_data(&air_status, data2, sizeof(data2));
        
      }
    }
  }
  else if (jsonDoc["id"] == "fan")
  {
    if (simulation_mode)
    {
      if (jsonDoc["value"] == "low")
      {
        air_status.fan = FAN_LOW;
        strcpy(air_status.fan_str, "LOW");
        Serial.println("[SIMULATION MODE] Fan set to LOW");
      }
      else if (jsonDoc["value"] == "medium")
      {
        air_status.fan = FAN_MEDIUM;
        strcpy(air_status.fan_str, "MED");
        Serial.println("[SIMULATION MODE] Fan set to MEDIUM");
      }
      else if (jsonDoc["value"] == "high")
      {
        air_status.fan = FAN_HIGH;
        strcpy(air_status.fan_str, "HIGH");
        Serial.println("[SIMULATION MODE] Fan set to HIGH");
      }
      else if (jsonDoc["value"] == "auto")
      {
        air_status.fan = FAN_AUTO;
        strcpy(air_status.fan_str, "AUTO");
        Serial.println("[SIMULATION MODE] Fan set to AUTO");
      }
    }
    else
    {
      if (jsonDoc["value"] == "low")
      {
        air_set_fan(&air_status, FAN_LOW);
      }
      else if (jsonDoc["value"] == "medium")
      {
        air_set_fan(&air_status, FAN_MEDIUM);
      }
      else if (jsonDoc["value"] == "high")
      {
        air_set_fan(&air_status, FAN_HIGH);
      }
      else if (jsonDoc["value"] == "auto")
      {
        air_set_fan(&air_status, FAN_AUTO);
      }
    }
  }
  else if (jsonDoc["id"] == "save")
  {
    if (simulation_mode)
    {
      if (jsonDoc["value"] == "1")
      {
        air_status.save = true;
        Serial.println("[SIMULATION MODE] Save mode ON");
      }
      else if (jsonDoc["value"] == "0")
      {
        air_status.save = false;
        Serial.println("[SIMULATION MODE] Save mode OFF");
      }
    }
    else
    {
      if (jsonDoc["value"] == "1")
      {
        air_set_save_on(&air_status);
      }
      else if (jsonDoc["value"] == "0")
      {
        air_set_save_off(&air_status);
      }
    }
  }
  // sets hw timer
  else if (jsonDoc["id"] == "timer_hw")
  {
    int val = atoi(jsonDoc["time"]);
    if (simulation_mode)
    {
      Serial.printf("[SIMULATION MODE] Hardware timer command ignored in simulation mode\n");
    }
    else
    {
      if (jsonDoc["value"] == "cancel")
      {
        air_set_timer(&air_status, TIMER_HW_CANCEL, val);
      }
      else if (jsonDoc["value"] == "off")
      {
        air_set_timer(&air_status, TIMER_HW_OFF, val);
      }
      else if (jsonDoc["value"] == "repeat_off")
      {
        air_set_timer(&air_status, TIMER_HW_REPEAT_OFF, val);
      }
      else if (jsonDoc["value"] == "on")
      {
        air_set_timer(&air_status, TIMER_HW_ON, val);
      }
    }
  }

  // Add simulation mode control
  else if (jsonDoc["id"] == "simulation_mode")
  {
    if (jsonDoc["value"] == "enable")
    {
      simulation_mode = true;
      Serial.println("[SIMULATION MODE] Simulation mode ENABLED");

      initSimulationMode();

      // Send confirmation response
      JsonDocument responseDoc;
      responseDoc["id"] = "simulation_mode";
      responseDoc["status"] = "enabled";
      responseDoc["message"] = "Simulation mode activated - AC simulation enabled";

      String response;
      serializeJson(responseDoc, response);
      webSocket.broadcastTXT(response);
    }
    else if (jsonDoc["value"] == "disable")
    {
      simulation_mode = false;
      Serial.println("[SIMULATION MODE] Simulation mode DISABLED");

      // Send confirmation response
      JsonDocument responseDoc;
      responseDoc["id"] = "simulation_mode";
      responseDoc["status"] = "disabled";
      responseDoc["message"] = "Simulation mode deactivated - Normal AC operation";

      String response;
      serializeJson(responseDoc, response);
      webSocket.broadcastTXT(response);
    }
  }

  // status sends current values
  else if (jsonDoc["id"] == "status")
  {
    String message;
    message = air_to_json(&air_status);
    webSocket.broadcastTXT(message);
    // reset serial buffers
    air_status.buffer_cmd = String("");
    air_status.buffer_raw = String("");
  }

  // master_info sends master unit information
  else if (jsonDoc["id"] == "master_info")
  {
    request_master_info(&air_status);

    String message;
    message = master_info_to_json(&air_status);
    webSocket.broadcastTXT(message);
  }

  // sensors sends all sensor values
  else if (jsonDoc["id"] == "sensors")
  {
    //sensors are not requested each time to reduce traffic, it just sends the last values
    String message;
    message = sensors_to_json(&air_status);
    webSocket.broadcastTXT(message);
  }

  // addresses sends all address values
  else if (jsonDoc["id"] == "addresses")
  {
    String message;
    message = addresses_to_json(&air_status);
    webSocket.broadcastTXT(message);
  }

  // system_info sends system information
  else if (jsonDoc["id"] == "system_info")
  {
    String message;
    message = system_info_to_json(&air_status);
    webSocket.broadcastTXT(message);
  }

  // sets sampling period for data logging
  else if (jsonDoc["id"] == "sampling")
  {
    temperature_interval = jsonDoc["value"];
    my_timer_set_unit(&timerTemperature, 1000);
    my_timer_set_interval(&timerTemperature, temperature_interval);
    my_timer_repeat(&timerTemperature);
    my_timer_start(&timerTemperature);
  }

  else if (jsonDoc["id"] == "timeseries")
  {
    // send data but not in a whole BIG json
    String message = "";

    message = timeseries_to_json("timeseries", "timestamp", timestamps, JSON_UL_INT, temp_idx);
    webSocket.broadcastTXT(message);

// if there is an alternative temperature sensor
#if defined(USE_AHT20) || defined(USE_BMP280)
    message = timeseries_to_json("timeseries", "sensor_temperature", sensor_temperature, JSON_FLOAT, temp_idx);
    webSocket.broadcastTXT(message);
#endif

#ifdef USE_AHT20
    message = timeseries_to_json("timeseries", "sensor_humidity", sensor_humidity, JSON_FLOAT, temp_idx);
    webSocket.broadcastTXT(message);
#endif
#ifdef USE_BMP280
    message = timeseries_to_json("timeseries", "sensor_pressure", sensor_pressure, JSON_FLOAT, temp_idx);
    webSocket.broadcastTXT(message);
#endif

    message = timeseries_to_json("timeseries", "ac_external_temperature", ac_outdoor_te, JSON_INT, temp_idx);
    webSocket.broadcastTXT(message);

    message = timeseries_to_json("timeseries", "ac_sensor_temperature", ac_sensor_temperature, JSON_FLOAT, temp_idx);
    webSocket.broadcastTXT(message);

  } // end if timeseries

else if (jsonDoc["id"] == "explore_sensor")
{
  uint8_t sensor_id = jsonDoc["sensor_id"];

  print_logf("[SENSOR_EXPLORE] Exploring sensor 0x%02X\n", sensor_id);

  if (simulation_mode)
  {
    Serial.println("[SIMULATION MODE] Sensor explore ignored in simulation mode");

    JsonDocument responseDoc;
    responseDoc["id"] = "explore_sensor";
    responseDoc["status"] = "ignored";
    responseDoc["sensor_id"] = sensor_id;

    String response;
    serializeJson(responseDoc, response);
    webSocket.broadcastTXT(response);
  }
  else
  {
    // Query the sensor
    air_query_sensor(&air_status, sensor_id);

    // Wait a bit for response
    unsigned long time_now = millis();
    while (millis() - time_now < 100)
    {
      air_parse_serial(&air_status);
      yield();
    }

    // Send response with the result
    JsonDocument responseDoc;
    responseDoc["id"] = "explore_sensor";
    responseDoc["status"] = "valid";
    responseDoc["sensor_id"] = sensor_id;

    if (air_status.sensor_val != -1)
    {
      responseDoc["value"] = air_status.sensor_val;
    }
    else
    {
      responseDoc["value"] = -1;
      responseDoc["message"] = "No response received";
    }

    String response;
    serializeJson(responseDoc, response);
    webSocket.broadcastTXT(response);
  }
}


#ifdef USE_MQTT
  else if (jsonDoc["id"] == "mqtt_enable")
  {
    // Handle MQTT enable/disable
    bool enable = jsonDoc["value"];
    
    setMQTTEnabled(enable);
    
    Serial.printf("[MQTT] MQTT %s via web UI\n", enable ? "enabled" : "disabled");
    
    // Send confirmation response
    JsonDocument responseDoc;
    responseDoc["id"] = "mqtt_enable";
    responseDoc["status"] = enable ? "enabled" : "disabled";
    responseDoc["message"] = enable ? "MQTT enabled" : "MQTT disabled";
    responseDoc["mqtt_enabled"] = air_status.mqtt_enabled;
    responseDoc["mqtt_status"] = getMQTTStatus();
    
    String response;
    serializeJson(responseDoc, response);
    webSocket.broadcastTXT(response);
  }
  else if (jsonDoc["id"] == "mqtt_config")
  {
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
  else if (jsonDoc["id"] == "mqtt_test")
  {
    // Handle MQTT connection test
    Serial.println("Testing MQTT connection...");

    // Test MQTT connection
    bool testResult = testMQTTConnection();

    // Send response
    JsonDocument responseDoc;
    responseDoc["id"] = "mqtt_test";
    if (testResult)
    {
      responseDoc["status"] = "connected";
      responseDoc["message"] = "MQTT connection successful";
    }
    else
    {
      responseDoc["status"] = "failed";
      responseDoc["message"] = "MQTT connection failed";
    }

    String response;
    serializeJson(responseDoc, response);
    webSocket.broadcastTXT(response);
  }
  else if (jsonDoc["id"] == "mqtt_get_config")
  {
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
  else if (jsonDoc["id"] == "mqtt_discovery")
  {
    String value = jsonDoc["value"];

    if (value == "send")
    {
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
    }
    else if (value == "reset")
    {
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
#endif // USE_MQTT

  // handle raw bytes command
  else if (jsonDoc["id"] == "raw_bytes")
  {
    if (simulation_mode)
    {
      Serial.println("[SIMULATION MODE] Raw bytes command ignored in simulation mode");

      // Send response indicating simulation mode
      JsonDocument responseDoc;
      responseDoc["id"] = "raw_bytes";
      responseDoc["status"] = "ignored";
      responseDoc["message"] = "Raw bytes ignored in simulation mode";

      String response;
      serializeJson(responseDoc, response);
      webSocket.broadcastTXT(response);
    }
    else
    {
      Serial.println("Processing raw bytes command...");

      // Get the bytes array from JSON
      JsonArray bytesArray = jsonDoc["bytes"];
      if (bytesArray.size() == 0)
      {
        Serial.println("Error: No bytes provided");
        return;
      }

      // Convert JSON array to byte array
      byte rawData[bytesArray.size()];
      for (size_t i = 0; i < bytesArray.size(); i++)
      {
        rawData[i] = bytesArray[i];
      }

      // Print debug info
      Serial.printf("Sending %d raw bytes: ", bytesArray.size());
      for (size_t i = 0; i < bytesArray.size(); i++)
      {
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
  }

  // handle autonomous mode
  else if (jsonDoc["id"] == "autonomous")
  {
    if (jsonDoc["value"] == "start")
    {
      autonomous_mode = true;
      // Ensure timer is properly configured before starting
      // startAutonomous();
      my_timer_set_unit(&timerAutonomous, 1000);  // 1000ms
      my_timer_set_interval(&timerAutonomous, 8); // update every 8 s
      my_timer_repeat(&timerAutonomous);
      my_timer_start(&timerAutonomous);

      Serial.println("[AUTO] Autonomous mode started");
      print_log("[AUTO] Autonomous mode activated");

      // Send confirmation response
      JsonDocument responseDoc;
      responseDoc["id"] = "autonomous";
      responseDoc["status"] = "started";
      responseDoc["message"] = "Autonomous mode activated";

      String response;
      serializeJson(responseDoc, response);
      webSocket.broadcastTXT(response);
    }
    else if (jsonDoc["value"] == "stop")
    {
      autonomous_mode = false;
      my_timer_disable(&timerAutonomous);
      Serial.println("[AUTO] Autonomous mode stopped");
      print_log("[AUTO] Autonomous mode deactivated");

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
  else if (jsonDoc["id"] == "sync_addresses")
  {
    // Copy observed addresses to active addresses
    if (air_status.observed_master != 0xFF)
    {
      air_status.master = air_status.observed_master;
    }
    // Use the first observed remote if available
    if (air_status.observed_remotes_count > 0)
    {
      air_status.remote = air_status.observed_remotes[0];
    }

    print_logf("Address sync: Master=0x%02X, Remote=0x%02X",
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
    for (uint8_t i = 0; i < air_status.observed_remotes_count; i++)
    {
      observedRemotesArr.add(air_status.observed_remotes[i]);
    }

    String response;
    serializeJson(responseDoc, response);
    webSocket.broadcastTXT(response);
  }

  else if (jsonDoc["id"] == "reset_addresses")
  {
    // Reset to default addresses
    air_status.master = MASTER; // 0x00
    air_status.remote = REMOTE; // 0x40

    print_logf("Address reset: Master=0x%02X, Remote=0x%02X",
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

  else if (jsonDoc["id"] == "set_addresses")
  {
    // Set manual addresses
    bool changed = false;

    if (jsonDoc["master"].is<int>())
    {
      air_status.master = jsonDoc["master"];
      changed = true;
    }
    if (jsonDoc["remote"].is<int>())
    {
      air_status.remote = jsonDoc["remote"];
      changed = true;
    }

    if (changed)
    {
      print_logf("Manual address set: Master=0x%02X, Remote=0x%02X",
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
  else if (jsonDoc["id"] == "reset_observed_remotes")
  {
    // Reset observed remotes
    reset_observed_remotes(&air_status);

    print_logf("Observed remotes reset");

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
  else if (jsonDoc["id"] == "announce_remote")
  {
    print_log("[ANNOUNCE] Starting remote announcement process");

    if (simulation_mode)
    {
      Serial.println("[SIMULATION MODE] Announce remote ignored in simulation mode");

      JsonDocument responseDoc;
      responseDoc["id"] = "announce_remote";
      responseDoc["status"] = "ignored";
      responseDoc["message"] = "Announce remote ignored in simulation mode";

      String response;
      serializeJson(responseDoc, response);
      webSocket.broadcastTXT(response);
    }
    else
    {
      // Start the announcement state machine
      air_start_announcement(&air_status);
      air_announce_remote(&air_status);

      // Send initial status
      JsonDocument responseDoc;
      responseDoc["id"] = "announce_remote";
      responseDoc["status"] = "started";
      responseDoc["message"] = "Announcement process started";
      responseDoc["state"] = "UNLINKED";
      responseDoc["remote"] = air_status.remote;

      String response;
      serializeJson(responseDoc, response);
      webSocket.broadcastTXT(response);
    }
  }

  // Add new handler for unlink remote
  else if (jsonDoc["id"] == "unlink_remote")
  {
    print_log("[ANNOUNCE] Unlinking remote");

    if (simulation_mode)
    {
      Serial.println("[SIMULATION MODE] Unlink remote ignored in simulation mode");
    }
    else
    {
      // Unlink the remote
      air_unlink_remote(&air_status);
    }
  }

  // Add listen mode handlers
  else if (jsonDoc["id"] == "listen_mode")
  {
    if (jsonDoc["value"] == "enable")
    {
      listen_mode = true;
      Serial.println("[LISTEN MODE] Enabled - prioritizing serial communication");

      JsonDocument responseDoc;
      responseDoc["id"] = "listen_mode";
      responseDoc["status"] = "enabled";
      responseDoc["message"] = "Listen mode activated - only essential operations";

      String response;
      serializeJson(responseDoc, response);
      webSocket.broadcastTXT(response);
    }
    else if (jsonDoc["value"] == "disable")
    {
      listen_mode = false;
      Serial.println("[LISTEN MODE] Disabled - normal operation resumed");

      JsonDocument responseDoc;
      responseDoc["id"] = "listen_mode";
      responseDoc["status"] = "disabled";
      responseDoc["message"] = "Listen mode deactivated - full functionality restored";

      String response;
      serializeJson(responseDoc, response);
      webSocket.broadcastTXT(response);
    }
  }

  else if (jsonDoc["id"] == "explore_dn_code")
  {
    uint8_t code = jsonDoc["code"];

    print_logf("[DN_EXPLORE] Exploring code 0x%02X\n", code);

    if (simulation_mode)
    {
      Serial.println("[SIMULATION MODE] DN explore ignored in simulation mode");

      JsonDocument responseDoc;
      responseDoc["id"] = "explore_dn_code";
      responseDoc["status"] = "ignored";
      responseDoc["code"] = code;

      String response;
      serializeJson(responseDoc, response);
      webSocket.broadcastTXT(response);
    }
    else
    {
      // Request the DN code
      air_request_dn_code_current(&air_status, code);

      // Wait a bit for response
      unsigned long time_now = millis();
      while (millis() - time_now < 100)
      {
        air_parse_serial(&air_status);
        yield();
      }

      // Send response with the result
      JsonDocument responseDoc;
      responseDoc["id"] = "explore_dn_code";
      responseDoc["status"] = "valid";
      responseDoc["code"] = code;

      // Find the DN code in the results
      bool found = false;
      for (uint8_t i = 0; i < air_status.dn_codes_count; i++)
      {
        if (air_status.dn_codes[i].code == code)
        {
          responseDoc["value"] = air_status.dn_codes[i].value;
          found = true;
          break;
        }
      }

      if (!found)
      {
        responseDoc["value"] = -1;
        responseDoc["message"] = "No response received";
      }

      String response;
      serializeJson(responseDoc, response);
      webSocket.broadcastTXT(response);
    }
  }

  else if (jsonDoc["id"] == "serial_config") {
      uint8_t tx_pin = jsonDoc["tx_pin"] | air_status.txpin;
      uint8_t rx_pin = jsonDoc["rx_pin"] | air_status.rxpin;
      uint8_t rx_buffer_size = jsonDoc["rx_buffer_size"] | air_status.rxsize;
      
      air_set_serial_config(&air_status, tx_pin, rx_pin, rx_buffer_size);
      
      // Send confirmation back to webpage
      JsonDocument responseDoc;
      responseDoc["id"] = "serial_config";
      responseDoc["status"] = "applied";
      responseDoc["tx_pin"] = air_status.txpin;
      responseDoc["rx_pin"] = air_status.rxpin;
      responseDoc["rx_buffer_size"] = air_status.rxsize;
      
      String response;
      serializeJson(responseDoc, response);
      webSocket.broadcastTXT(response);
  }

else if (jsonDoc["id"] == "i2c_config") {
    uint8_t sda_pin = jsonDoc["sda_pin"] | air_status.sda_pin;
    uint8_t scl_pin = jsonDoc["scl_pin"] | air_status.scl_pin;
    
    air_set_i2c_config(&air_status, sda_pin, scl_pin);
    
    // Send confirmation back to webpage
    JsonDocument responseDoc;
    responseDoc["id"] = "i2c_config";
    responseDoc["status"] = "applied";
    responseDoc["sda_pin"] = air_status.sda_pin;
    responseDoc["scl_pin"] = air_status.scl_pin;
    
    String responseStr;
    serializeJson(responseDoc , responseStr);
    webSocket.broadcastTXT(responseStr);
}

  else if (jsonDoc["id"] == "save_settings") {
    settings_t settings;
    getSettings(&air_status, &settings);
    
    //print_log some settings for debug
    print_logf("[SETTINGS] Saving settings: txpin=%d, rxpin=%d, rxsize=%d, sda_pin=%d, scl_pin=%d\n",
               settings.txpin, settings.rxpin, settings.rxsize,
               settings.sda_pin, settings.scl_pin);

    bool success = saveSettings(&air_status, &settings);
    
    print_logf("[SETTINGS] Settings save %s\n", success ? "successful" : "failed");

    JsonDocument responseDoc;
    responseDoc["id"] = "save_settings";
    responseDoc["status"] = success ? "success" : "error";
    responseDoc["message"] = success ? "Settings saved successfully" : "Failed to save settings";
    
    String response;
    serializeJson(responseDoc, response);
    webSocket.broadcastTXT(response);
  }
  
  else if (jsonDoc["id"] == "load_settings") {
    settings_t settings;
    bool success = loadSettings(&air_status, &settings);
    
    if (success) {
      applySettings(&air_status, &settings);
    }
    
    JsonDocument responseDoc;
    responseDoc["id"] = "load_settings";
    responseDoc["status"] = success ? "success" : "error";
    responseDoc["message"] = success ? "Settings loaded and applied" : "Failed to load settings";
    
    String response;
    serializeJson(responseDoc, response);
    webSocket.broadcastTXT(response);
  }
  
  else if (jsonDoc["id"] == "reset_settings") {
    if (LittleFS.exists(SETTINGS_FILENAME)) {
      LittleFS.remove(SETTINGS_FILENAME);
    }
    
    JsonDocument responseDoc;
    responseDoc["id"] = "reset_settings";
    responseDoc["status"] = "success";
    responseDoc["message"] = "Settings file deleted. Will use defaults on restart.";
    
    String response;
    serializeJson(responseDoc, response);
    webSocket.broadcastTXT(response);
  }


  else if (jsonDoc["id"] == "restart")
  {
    Serial.println("[SYSTEM] Restart requested via web UI");
    webSocket.broadcastTXT("{\"id\":\"restart\",\"status\":\"restarting\"}");
    delay(500);
    ESP.restart();
  }

#ifdef USE_MQTT

  bool shouldPublishMQTT = false;

  // should publish is true if json id is power, temp, mode, fan
  if (jsonDoc["id"] == "power" || jsonDoc["id"] == "temp" ||
      jsonDoc["id"] == "mode" || jsonDoc["id"] == "fan" ||
      jsonDoc["id"] == "save" || jsonDoc["id"] == "timer" ||
      jsonDoc["id"] == "timer_hw" || jsonDoc["id"] == "autonomous")
  {
    shouldPublishMQTT = true;
  }

  if (shouldPublishMQTT)
  {
    // Force immediate MQTT update by temporarily manipulating timer
    timerMQTT.start = 0; // Force timer to be ready
    handleMQTT();        // This will now execute the MQTT publish immediately
    yield();

    Serial.println("[WEB] WebScoket info pulbished to MQTT");
  }
#endif
}

void notifyWebSocketClients()
{
  String message;
  // update status
  air_parse_serial(&air_status);
  message = air_to_json(&air_status);
  webSocket.broadcastTXT(message);
  // webSocketBroadcast(message);
  // reset serial buffers
  air_status.buffer_cmd = String("");
  air_status.buffer_raw = String("");
}
