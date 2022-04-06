#include "config.h"

extern air_status_t air_status;
extern WebSocketsServer webSocket;

extern MySimpleTimer timerAC;
//File fsUploadFile;                                    // a File variable to temporarily store the received file

extern MySimpleTimer timerTemperature;
extern int temp_interval;

extern const int DHTPin;
//extern Adafruit_BMP085 bmp;
extern uint8_t bmp_status;

extern float dht_h[MAX_LOG_DATA];
extern float dht_t[MAX_LOG_DATA];
extern float ac_sensor[MAX_LOG_DATA];

extern int ac_outdoor_te[MAX_LOG_DATA];
extern float bmp_t[MAX_LOG_DATA];
extern float bmp_p[MAX_LOG_DATA];
extern unsigned long timestamps[MAX_LOG_DATA];
extern int temp_idx;
extern float dht_h_current, dht_t_current, bmp_t_current, bmp_p_current;

// Define NTP Client to get time
extern WiFiUDP ntpUDP;
extern const long utcOffsetInSeconds;
extern int timeOffset;
extern NTPClient timeClient;

extern MySimpleTimer timerStatus;
extern MySimpleTimer timerReadSerial;
extern MySimpleTimer timerSaveFile;


extern const char compile_date[];

/*__________________________________________________________HELPER_FUNCTIONS__________________________________________________________*/


void serialize_array_float(float *ptr, JsonArray &arr, int idx) {
  int i;
  if (idx > MAX_LOG_DATA) idx = 0;

  for (i = 0; i < MAX_LOG_DATA; i++) {
    //Serial.printf("%.2f ", ptr[(idx+i)%MAX_LOG_DATA]);
    arr.add(ptr[(idx + i) % MAX_LOG_DATA]);
  }

  //arr.add(arr.size());
  //arr.add(arr.memoryUsage());
}

void serialize_array_ul_int(unsigned long *ptr, JsonArray &arr, int idx) {
  int i;
  if (idx > MAX_LOG_DATA) idx = 0;

  for (i = 0; i < MAX_LOG_DATA; i++) {
    //Serial.printf("%.2f ", ptr[(idx+i)%MAX_LOG_DATA]);
    arr.add(ptr[(idx + i) % MAX_LOG_DATA]);
  }

  //arr.add(arr.size());
  //arr.add(arr.memoryUsage());

}

void serialize_array_int(int *ptr, JsonArray &arr, int idx) {
  int i;
  if (idx > MAX_LOG_DATA) idx = 0;

  for (i = 0; i < MAX_LOG_DATA; i++) {
    arr.add(ptr[(idx + i) % MAX_LOG_DATA]);
  }

  //arr.add(arr.size());
  //arr.add(arr.memoryUsage());
}



/*__________________________________________________________JSON_FUNCTIONS__________________________________________________________*/

String air_to_json(air_status_t *air)
{
  String response;
  StaticJsonDocument<800> jsonDoc; //400 without indoor/outdoor data

  jsonDoc["id"] = "status";
  jsonDoc["save"] = air->save;
  jsonDoc["heat"] = air->heat;
  jsonDoc["preheat"] = air->preheat;
  jsonDoc["cold"] = air->cold;
  jsonDoc["temp"] = air->target_temp;
  jsonDoc["sensor_temp"] = air->sensor_temp;
  jsonDoc["fan"] = air->fan_str;
  jsonDoc["mode"] = air->mode_str;
  jsonDoc["power"] = air->power;

  jsonDoc["boot_time"] = air->boot_time;
  //indoor unit data
  //jsonDoc["indoor_room_temp"] = air->indoor_room_temp;
  jsonDoc["indoor_ta"] = air->indoor_ta;
  jsonDoc["indoor_tcj"] = air->indoor_tcj;
  jsonDoc["indoor_tc"] = air->indoor_tc;
  //jsonDoc["indoor_filter_time"] = air->indoor_filter_time;

  //outdoor unit data
  jsonDoc["outdoor_te"] = air->outdoor_te;
  jsonDoc["outdoor_to"] = air->outdoor_to;
  //jsonDoc["outdoor_td"] = air->outdoor_td;
  //jsonDoc["outdoor_ts"] = air->outdoor_ts;
  //jsonDoc["outdoor_ths"] = air->outdoor_ths;
  jsonDoc["outdoor_current"] = air->outdoor_current;
  jsonDoc["power_consumption"] = air->power_consumption;
  //jsonDoc["outdoor_cumhour"] = air->outdoor_cumhour;

  jsonDoc["indoor_fan_speed"] = air->indoor_fan_speed;
  //jsonDoc["indoor_fan_run_time"] = air->indoor_fan_run_time;

  //jsonDoc["outdoor_tl"] = air->outdoor_tl;
  //jsonDoc["outdoor_comp_freq"] = air->outdoor_comp_freq;
  //jsonDoc["outdoor_lower_fan_speed"] = air->outdoor_lower_fan_speed;
  //jsonDoc["outdoor_upper_fan_speed"] = air->outdoor_upper_fan_speed;



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

  jsonDoc["heap"] = ESP.getFreeHeap();
  jsonDoc["compile"] = compile_date;
  jsonDoc["ip"] = air->ip;

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
  jsonDoc["rx_data"] = air->buffer_rx;

  serializeJson(jsonDoc, response);

  return response;
}


String string_to_json(String t)
{
  int i;
  String str;
  String response;
  StaticJsonDocument<800> jsonDoc; //400 without indoor/outdoor data

  jsonDoc["id"] = "txt";
  jsonDoc["txt"] = str;
  serializeJson(jsonDoc, response);

  return response;
}


String timeseries_to_json(String id, String val, void *data, int data_type, int temp_idx) {
  DynamicJsonDocument jsonDoc(2800); //For more than 1kb Dynamic is better
  String message = "";

  jsonDoc.clear();

  jsonDoc["id"] = id;
  jsonDoc["n"] = MAX_LOG_DATA;
  jsonDoc["val"] = val;

  JsonArray jsonArr = jsonDoc.createNestedArray(val);

  switch (data_type) {
    case 0:
      serialize_array_float((float*)data, jsonArr, temp_idx);
    case 1:
      serialize_array_int((int*)data, jsonArr, temp_idx);
    case 2:
      serialize_array_ul_int((unsigned long*)data, jsonArr, temp_idx);
  }

  jsonDoc.garbageCollect();
  serializeJson(jsonDoc, message);
  jsonDoc.clear();

  return message;
}

void processRequest( uint8_t *  payload) {
  //decode json
  StaticJsonDocument<200> jsonDoc;
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
      timerAC.disable();
      air_status.timer_mode_req = jsonDoc["timer_mode"];
      air_status.timer_time_req = 0;
    }
    else {
      timerAC.setInterval(val);
      timerAC.start();
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
    timerTemperature.setUnit(1000);
    timerTemperature.setInterval(temp_interval);
    timerTemperature.repeat();
    timerTemperature.start();

  }
  //request timeseries values for graphic
  else if (jsonDoc["id"] == "timeseriesOLD") {
    String message;

    //use assistant to estimate size https://arduinojson.org/v6/assistant/
    //https://arduinojson.org/v6/how-to/determine-the-capacity-of-the-jsondocument/
    //3000 holds around 150 vals
    DynamicJsonDocument docTimeSeries(19100); //16100

    docTimeSeries["id"] = "timeseries";
    docTimeSeries["n"] = MAX_LOG_DATA;

    JsonArray arrt = docTimeSeries.createNestedArray("timestamp");
    int i;
    /*arrt.add(timestamps[temp_idx]);
      for (i = (temp_idx + 1) % MAX_LOG_DATA; i != temp_idx; i = (i + 1) % MAX_LOG_DATA) {
      arrt.add(timestamps[i]);
      }*/
    serialize_array_ul_int(timestamps, arrt, temp_idx);

    JsonArray arr = docTimeSeries.createNestedArray("dht_t");
    serialize_array_float(dht_t, arr, temp_idx);

    JsonArray arr2 = docTimeSeries.createNestedArray("dht_h");
    serialize_array_float(dht_h, arr2, temp_idx);

    JsonArray arr4 = docTimeSeries.createNestedArray("ac_sensor_t");
    serialize_array_float(ac_sensor, arr4, temp_idx);

    JsonArray arr6 = docTimeSeries.createNestedArray("bmp_p");
    serialize_array_float(bmp_p, arr6, temp_idx);

    JsonArray arr7 = docTimeSeries.createNestedArray("te");
    serialize_array_int(ac_outdoor_te, arr7, temp_idx);

    serializeJson(docTimeSeries, message);
    webSocket.broadcastTXT(message);

  } //end if timeseries
  else if (jsonDoc["id"] == "timeseries_query") {
    String message;
    StaticJsonDocument <200> doc; //16100

    doc["id"] = "timeseries_query";

    JsonArray arr = doc.createNestedArray("values");
    arr.add("time_stamp");
    arr.add("dht_t");
    arr.add("dht_h");
    arr.add("ac_sensor_t");
    arr.add("bmp_p");
    arr.add("to");

    serializeJson(doc, message);
    webSocket.broadcastTXT(message);

  }
  else if (jsonDoc["id"] == "timeseries") {
    //send data but not in a whole BIG json
    String message = "";

    message = timeseries_to_json("timeseries", "timestamp", timestamps, 2, temp_idx);
    webSocket.broadcastTXT(message);

    /*
      uint8_t m[WEBSOCKETS_MAX_HEADER_SIZE+2000];
      int l = message.length();
      strncpy((char*)m[WEBSOCKETS_MAX_HEADER_SIZE],message.c_str(),l);

      //bool broadcastTXT(uint8_t * payload, size_t length = 0, bool headerToPayload = false);
      webSocket.broadcastTXT(m,(l - WEBSOCKETS_MAX_HEADER_SIZE),true);
    */
    message = timeseries_to_json("timeseries", "dht_t", dht_t, 0, temp_idx);
    webSocket.broadcastTXT(message);

    message = timeseries_to_json("timeseries", "dht_h", dht_h, 0, temp_idx);
    webSocket.broadcastTXT(message);

    message = timeseries_to_json("timeseries", "bmp_p", bmp_p, 0, temp_idx);
    webSocket.broadcastTXT(message);

    message = timeseries_to_json("timeseries", "te", ac_outdoor_te, 1, temp_idx);
    webSocket.broadcastTXT(message);

    message = timeseries_to_json("timeseries", "ac_sensor_t", ac_sensor, 0, temp_idx);
    webSocket.broadcastTXT(message);


  } //end if timeseries
}
