/*
GNU GENERAL PUBLIC LICENSE

Version 2, June 1991

Copyright (C) 1989, 1991 Free Software Foundation, Inc.  
51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

Everyone is permitted to copy and distribute verbatim copies
of this license document, but changing it is not allowed.

*/

#ifndef AC_PROTOCOL_H
#define AC_PROTOCOL_H

#include <SoftwareSerial.h>  // Add this line
#include <Arduino.h>         // Add this for basic Arduino types

// you may need to change those values if you use a different a/c model
#define MASTER  0x00         // master unit address
#define REMOTE  0x40         // remote unit address  
#define BCAST   0xFE         // broadcast address, used to send commands to all slaves

//position of bytes in packet
#define FROM  0
#define TO    1
#define COUNT 3


#define MAX_RX_BUFFER 128    //maximum rx buffer size
#define MAX_CMD_BUFFER 32    //maximum message size

#define FAN_AUTO   2
#define FAN_HIGH   3
#define FAN_MEDIUM 4
#define FAN_LOW    5

#define MODE_HEAT 1
#define MODE_COOL 2
#define MODE_FAN  3
#define MODE_DRY  4
#define MODE_AUTO 6//5

#define MSG_UNK           0
#define MSG_SENSOR_ERROR  1
#define MSG_STATUS        2
#define MSG_STATUS_EXT    3
#define MSG_MASTER_ACK    4
#define MSG_MASTER_ALIVE  5
#define MSG_STATUS_MODE   6
#define MSG_SENSOR_ANSWER 7

#define MAX_OBSERVED_REMOTES 8

#define TIMER_SW_OFF 0
#define TIMER_SW_ON  1
#define TIMER_SW_RESET  2

#define TIMER_HW_CANCEL     0x00
#define TIMER_HW_ON         0x07
#define TIMER_HW_REPEAT_OFF 0x06
#define TIMER_HW_OFF        0x05

//pg 18 http://www.toshiba-aircon.co.uk/assets/uploads/product_assets/20131115_IM_1115460101_Standard_Duct_RAV-SM_6BTP-E_EN.pdf
//indoor unit data
//#define INDOOR_ROOM           0x00 //Room Temp (Control Temp) (°C) 
//#define INDOOR_ROOM           0x01 //Room temperature (remote controller)
#define INDOOR_TA               0x02 //Indoor unit intake air temperature (TA)
#define INDOOR_TCJ              0x03 //Indoor unit heat exchanger (coil) temperature (TCJ) TCJ Coil Liquid Temp (°C)
#define INDOOR_TC               0x04 //Indoor unit heat exchanger (coil) temperature (TC)  Coil Vapour Temp (°C)
#define INDOOR_FAN_SPEED        0x07 //Fan Speed (rpm)
#define INDOOR_FAN_RUN_TIME     0xF2 //Fan Run Time (x 100h)
#define INDOOR_FILTER_TIME      0xF3 //Filter sign time x 1h
#define INDOOR_DISCHARGE_TEMP   0xF4 //Indoor discharge temperature*1  F8???
 
//outdoor unit data
#define OUTDOOR_TE              0x60 //Outdoor unit heat exchanger (coil) temperature (TE)
#define OUTDOOR_TO              0x61 //Outside air temperature (TO)
#define OUTDOOR_TD              0x62 //Compressor discharge temperature (TD)
#define OUTDOOR_TS              0x63 //Compressor suction temperature (TS)
#define OUTDOOR_THS             0x65 //Heatsink temperature (THS)
#define OUTDOOR_CURRENT         0x6A //Operating current (x1/10)
#define OUTDOOR_TL              0x6D //TL Liquid Temp (°C)
#define OUTDOOR_COMP_FREQ       0x70 //Compressor Frequency (rps)
#define OUTDOOR_LOWER_FAN_SPEED 0x72 //Fan Speed (Lower) (rpm
#define OUTDOOR_UPPER_FAN_SPEED 0x73 //Fan Speed (Upper) (rpm
#define OUTDOOR_HOURS           0xF1 //Compressor cumulative operating hours (x100 h)

//TA = Return Air Sensor; indoor unit
//TC = Coil Sensor; indoor unit
//TL = Liquid Pipe Sensor (fan speed); outdoor unit
//TCJ = Coil Sensor; indoor unit
//TE = Heat Exchange Sensor (defrost); outdoor unit
//TD = Discharge Pipe Sensor; outdoor unit
//TO = Ambient; outdoor unit
//TS = Suction; outdoor unitTK = Oil sensor (VRF)

typedef struct {
  byte data[1];//MAX_RX_BUFFER];
  int idx_r = 0;
  int idx_w = 0;
  //idx_r is always BEFORE idx_w
  //idx_w
} rb_t;

typedef struct {
  uint8_t master;
  uint8_t remote;
  uint8_t observed_master;
  uint8_t observed_remotes[MAX_OBSERVED_REMOTES];
  uint8_t observed_remotes_count;
  uint8_t save;                   // save/eco mode
  uint8_t heat;
  uint8_t cold;
  uint8_t target_temp;            //target temperature
  float remote_sensor_temp;              //indoor temperature
  uint8_t fan;
  char fan_str[5];
  uint8_t mode; // 
  char mode_str[5];
  uint8_t preheat;                //preheat flag
  uint8_t filter_alert;           //filter alert flag
  uint8_t power;
  byte last_cmd[MAX_CMD_BUFFER];
  uint8_t last_cmd_type; //type of last command sent

  String buffer_cmd;              //buffer for decoded commands
  String buffer_rx;               //buffer for raw received data
  byte rx_data[MAX_RX_BUFFER];    //serial rx data
  byte rx_data_count = 0;
  
  int curr_w_idx = 0;
  int curr_r_idx = 0;
  uint8_t timer_mode_req;
  uint8_t timer_time_req;
  bool timer_enabled;
  int decode_errors = 0;

  unsigned long boot_time;

  int sensor_val = 0;     //sensor value
  int sensor_id = 1;      //sensor id to query, 0x01 means indoor temp, 0xff means no sensor is selected
                          
  int error_id =0;        
  int error_val=0;
  int error_type=0;

  //indoor unit data
  int indoor_room_temp;    //01 Room temperature (remote controller)
  int indoor_ta = 0;       //02 Indoor unit intake air temperature (TA)
  int indoor_tcj = 0;      //03 Indoor unit heat exchanger (coil) temperature (TCJ)
  int indoor_tc = 0;       //04 Indoor unit heat exchanger (coil) temperature (TC)
  int indoor_filter_time = 0;   //F3 Filter sign time

  int indoor_fan_speed = 0;//07 Fan Speed (rpm)
  int indoor_fan_run_time = 0;

  //outdoor unit data
  int outdoor_te = 0;      //60 Outdoor unit heat exchanger (coil) temperature (TE)
  int outdoor_to = 0;      //61 Outside air temperature (TO)
  int outdoor_td = 0;      //62 Compressor discharge temperature (TD)
  int outdoor_ts = 0;      //63 Compressor suction temperature (TS)
  int outdoor_ths = 0;     //64—65 Heatsink temperature (THS)
  int outdoor_current = 0; //6A Operating current (x1/10)
  int outdoor_cumhour = 0; //F1 Compressor cumulative operating hours (x100 h)

  int outdoor_tl = 0;
  int outdoor_comp_freq = 0;
  int outdoor_lower_fan_speed = 0;
  int outdoor_upper_fan_speed = 0; 
  
  float power_consumption = 0;
  String ip;   // IP address of the device
  rb_t rb;     // ring buffer for received data

  bool mqtt = false; // MQTT connection status

  SoftwareSerial serial;
} air_status_t;


void air_set_temp(air_status_t *air, uint8_t temp);
void air_set_power_on(air_status_t *air);
void air_set_power_off(air_status_t *air);
void air_set_mode_cold(air_status_t *air);
void air_set_mode_heat(air_status_t *air);
void air_set_mode_auto(air_status_t *air);
void air_set_mode_fan(air_status_t *air);
void air_set_mode_dry(air_status_t *air);
void air_set_fan(air_status_t *air, uint8_t value);
void air_set_save_on(air_status_t *air);
void air_set_save_off(air_status_t *air);
void air_set_mode(air_status_t *air, uint8_t mode);
void air_set_fan_speed(air_status_t *air, uint8_t fan);
void air_send_data(air_status_t *air, byte *data, int len);
void air_set_timer(air_status_t *air, uint8_t mode, uint8_t time);
void init_air_serial(air_status_t *air);
void air_send_ping(air_status_t *air);
void air_send_remote_temp(air_status_t *air, uint8_t value);
void air_query_sensor(air_status_t *air, uint8_t id);
void air_query_sensors(air_status_t *air, const byte *ids, size_t num_ids);
void reset_observed_remotes(air_status_t *air);
void air_print_status(air_status_t *s);
int air_parse_serial(air_status_t *air);
#endif // AC_PROTOCOL_H