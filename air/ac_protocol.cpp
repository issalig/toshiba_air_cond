/*
  GNU GENERAL PUBLIC LICENSE

  Version 2, June 1991

  Copyright (C) 1989, 1991 Free Software Foundation, Inc.
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

  Everyone is permitted to copy and distribute verbatim copies
  of this license document, but changing it is not allowed.

*/

#include "SoftwareSerial.h" //https://github.com/plerup/espsoftwareserial
#include "ac_protocol.h"
#include "print_log.h"

#include "process_request.h"

#define MIN_PACKET_SIZE 6 // src dest op len cmd crc

#define DEBUG 1

/*____________________________UTILS_______________________________ */

// Format byte as 2-digit uppercase hex with space
String format_hex_byte(uint8_t byte) {
  char hex[4];  // "XX "
  snprintf(hex, sizeof(hex), "%02X ", byte);
  return String(hex);
}

char converttoascii(char c)
{
  if (c < 10)
  {
    return c + '0';
  } else {
    return (c - 10) + 'A';
  }
}

uint8_t XORChecksum8(const byte *data, size_t len)
{
  uint8_t value = 0;
  for (size_t i = 0; i < len; i++)
  {
    value ^= (uint8_t)data[i];
  }
  return value;
}

int check_crc(const byte *data, size_t len) {
  uint8_t crc = 1, my_crc = 2;
  unsigned int packet_len;
  
  if (len >= MIN_PACKET_SIZE) { //minimal packet len
    //byte count is byte 3
    packet_len = data[3] + 5; //whole packet with CRC
    if (len == packet_len) {
      crc    = data[packet_len - 1]; //CRC is last byte
      my_crc = XORChecksum8(data, packet_len - 1); //CRC does not include own CRC
    }
  }

  return (my_crc == crc);
}

void air_print_status(air_status_t *s) {
  Serial.print("[STATUS]");
  Serial.print(" Power: "); Serial.print(s->power);
  Serial.print(" Mode: "); Serial.print(s->mode_str);
  Serial.print(" Fan: "); Serial.print(s->fan_str);
  Serial.print(" SensorTemp: "); Serial.print(s->remote_sensor_temp);
  Serial.print(" Temp: "); Serial.print(s->target_temp);
  Serial.print(" Cold: "); Serial.print(s->cold);
  Serial.print(" Heat: "); Serial.print(s->heat);
  Serial.print(" Save: "); Serial.print(s->save);
  Serial.print(" Errors: "); Serial.print(s->discarded_bytes);
  //Serial.print(" INDOOR_ROOM:  "); Serial.print(s->indoor_room_temp);
  Serial.print(" INDOOR_TA: "); Serial.print(s->indoor_ta);
  Serial.print(" INDOOR_TCJ: "); Serial.print(s->indoor_tcj);
  Serial.print(" INDOOR_TC: "); Serial.print(s->indoor_tc);
  //Serial.print(" INDOOR_FILTER_TIME:  "); Serial.print(s->indoor_filter_time);
  Serial.print(" OUTDOOR_TE: "); Serial.print(s->outdoor_te);
  Serial.print(" OUTDOOR_TO: "); Serial.print(s->outdoor_to);
  //Serial.print(" OUTDOOR_TD:   "); Serial.print(s->outdoor_td);
  //Serial.print(" OUTDOOR_TS:   "); Serial.print(s->outdoor_ts);
  //Serial.print(" OUTDOOR_THS:  "); Serial.print(s->outdoor_ths);
  Serial.print(" OUTDOOR_CURRENT: "); Serial.print(s->outdoor_current);
  //Serial.print(" OUTDOOR_HOURS:    "); Serial.print(s->outdoor_cumhour);

  Serial.print(" INDOOR_FAN_SPEED: "); Serial.print(s->indoor_fan_speed);
  //Serial.print(" INDOOR_FAN_RUN_TIME: "); Serial.print(s->indoor_fan_run_time);

  //Serial.print(" OUTDOOR_TL: "); Serial.print(s->outdoor_tl);
  //Serial.print(" OUTDOOR_COMP_FREQ: "); Serial.print(s->outdoor_comp_freq);
  //Serial.print(" OUTDOOR_LOWER_FAN_SPEED: "); Serial.print(s->outdoor_lower_fan_speed);
  //Serial.print(" OUTDOOR_UPPER_FAN_SPEED: "); Serial.print(s->outdoor_upper_fan_speed);

  Serial.println("");
}

/*
   Hard-coded byte streams
*/
void air_set_power_on(air_status_t *air) {
  //             00           01           02    03    04    05    06    CRC
  byte data[] = {air->remote, air->master, 0x11, 0x03, 0x08, 0x41, 0x03, 0x18};
  
  data[7] = XORChecksum8(data, sizeof(data) - 1);
  air_send_data(air, data, sizeof(data));

  air->power = 1; //update internal status
}

void air_set_power_off(air_status_t *air)  {
  //             00           01           02    03    04    05    06    CRC
  byte data[] = {air->remote, air->master, 0x11, 0x03, 0x08, 0x41, 0x02, 0x19};
  data[7] = XORChecksum8(data, sizeof(data) - 1);
  air_send_data(air, data, sizeof(data));

  air->power = 0; //update internal status
}

//send ping
void air_send_ping(air_status_t *air)  {
  //             00           01           02    03    04    05    06    07    08    09    10    CRC
  byte data[] = {air->remote, air->master, 0x15, 0x07, 0x08, 0x0c, 0x81, 0x00, 0x00, 0x48, 0x00, 0x9f};
  air_send_data(air, data, sizeof(data));
}

void air_set_save_off(air_status_t *air) {
  //bit0 from 7th in remote message
  //             00           01           02    03    04    05    06    07    CRC
  byte data[] = {air->remote, air->master, 0x11, 0x04, 0x08, 0x54, 0x01, 0x00, 0x08};
  data[8] = XORChecksum8(data, sizeof(data) - 1);
  air_send_data(air, data, sizeof(data));

  air->save = 0; //update internal status
}

void air_set_save_on(air_status_t *air) {
  //bit0 from 7th in remote message
  //             00           01           02    03    04    05    06    07    CRC
  byte data[] = {air->remote, air->master, 0x11, 0x04, 0x08, 0x54, 0x01, 0x01, 0x09};
  data[8] = XORChecksum8(data, sizeof(data) - 1);
  air_send_data(air, data, sizeof(data));

  air->save = 1; //update internal status
}

void air_set_temp(air_status_t *air, uint8_t target_temp)  {
  //       byte  00           01           02    03    04    05    06    07    08    09    10    11    CRC
  byte data[] = {air->remote, air->master, 0x11, 0x08, 0x08, 0x4C, 0x0C, 0x1D, 0x7A, 0x00, 0x33, 0x33, 0x76};

  //set mode   0C is byte for dry 1100  -> 100, 0A is cool 1010 ->  10
  data[6] = air->mode | 0b1000;
  data[7] = air->fan | 0b11000; //FAN set bit3 to 1 and bit4 to 1
  data[8] = ((target_temp) + 35) << 1; //temp is bit7-bit1
  data[10] = data[11] = 0x01 + 0x04 * air->heat + 0x02 * air->cold;

  data[12] = XORChecksum8(data, sizeof(data) - 1);

  air_send_data(air, data, sizeof(data));

  air->target_temp = target_temp; //update internal status
}

void air_set_fan(air_status_t *air, uint8_t value)  {
  //Master 7th byte bit4=1 bit3-bit1  auto:0x010 med:011 high:110 low:101
  //             00           01           02    03    04    05    06    07    08    09    10    11    CRC
  byte data[] = {air->remote, air->master, 0x11, 0x08, 0x08, 0x4C, 0x13, 0x1D, 0x7A, 0x00, 0x33, 0x33, 0x6E}; //LOW, FAN

  data[6] = 0x10 + air->mode; //set bit4 to 1 for MODE
  data[7] = 0b11000 + value; //set bit4 and bit3 to 1 for FAN
  data[8] = ((air->target_temp) + 35) << 1; //temp is bit7-bit1
  data[10] = data[11] = air->heat ? 0x55 : 0x33;

  data[12] = XORChecksum8(data, sizeof(data) - 1);

  air_send_data(air, data, sizeof(data));

  air->fan = value; //update internal status
}

void air_set_mode(air_status_t *air, uint8_t value)  {
  //From remote (Mode is bit3-bit0 from byte 6) heat:001 cool:010 fan:011 dry: 100 auto 101 
  //             00           01           02    03    04    05    06    CRC
  byte data[] = {air->remote, air->master, 0x11, 0x03, 0x08, 0x42, 0x04, 0x1C}; // dry
  //set mode
  data[6] = value;

  //compose new message
  //compute crc
  data[7] = XORChecksum8(data, sizeof(data) - 1);

  air_send_data(air, data, sizeof(data));

  air->mode = value; //update internal status
}

void air_send_remote_temp(air_status_t *air, uint8_t value)  {
  //             00           01           02    03    04    05    06    07    08    CRC
  byte data[] = {air->remote, air->master, 0x55, 0x05, 0x08, 0x81, 0x00, 0x7C, 0x00, 0xE5};
  //data[2]==0x55 data[5]==0x81   opcodes for this message
  //sensor temp
  data[7] = int((value) + 35) << 1;
  //compute crc
  data[9] = XORChecksum8(data, sizeof(data) - 1);

  air_send_data(air, data, sizeof(data));
}

//does not work correctly, maybe wall unit sets some internal register
void air_set_timer(air_status_t *air, uint8_t timer_mode, uint8_t timer_value)  {
  //             00           01           02    03    04    05    06    07    08    09    10    11    12    CRC
  byte data[] = {air->remote, air->master, 0x11, 0x09, 0x08, 0x0c, 0x82, 0x00, 0x00, 0x30, 0x07, 0x02, 0x02, 0xe9};
  //                                                                                                  |----- number of 30 minutes
  //                                                                                             |----- repeated
  //                                                                                       |------ 07 poweron,   06 poweroff repeat, 05 poweroff,  00 cancel

  data[10] = timer_mode;

  if (timer_mode == TIMER_HW_CANCEL) {
    data[11] = 0x4;
    data[12] = 0x1;
  } else {
    data[11] = timer_value;
    data[12] = timer_value;
  }

  //compose new message
  //compute crc
  data[13] = XORChecksum8(data, sizeof(data) - 1);

  air_send_data(air, data, sizeof(data));
}

/*____________________________CORE_______________________________ */

/*
  air_parse_serial
  air_decode_command

*/

void init_air_serial(air_status_t *air) {

  air->master = MASTER; //default master address
  air->remote = REMOTE; //default remote address

  air->observed_master = 0xFF; //dummy value
  air->observed_remotes_count = 0;
  for (int i = 0; i < MAX_OBSERVED_REMOTES; i++) {
      air->observed_remotes[i] = 0xFF;
  }

  // Initialize ring buffer
  rb_init(&air->rb);
  air->in_discard_sequence = false;

  //air->serial.begin(2400, SWSERIAL_8E1, D7, D8, false);//, 256, 11*16);
  air->serial.begin(2400, SWSERIAL_8E1, air->rxpin, air->txpin, false, air->rxsize, 128);
  //begin(uint32_t baud, SoftwareSerialConfig config, int8_t rxPin, int8_t txPin, bool invert, int bufCapacity = 64, int isrBufCapacity = 0);

  // high speed half duplex, turn off interrupts during tx
  air->serial.enableIntTx(false);

}

int air_decode_command(byte * data, air_status_t *s) {

#ifdef DEBUG_PLUS
  if (data[0] == air->master) {
    Serial.println("[CMD] From master ");
    //get status
  }
  else if (data[0] == air->remote) {
    Serial.println("[CMD] From remote");
    //get status
  }
  else {
    Serial.println("[CMD] Unknown origin address: ");
    Serial.print(data[0], HEX);
  }
#endif

  s->last_cmd_type = MSG_UNKNOWN;

  // get master and remote address from observed data
  // master is supposed to have address [0x00 .. 0x0F]
  // remote is supposed to have address [0x40 .. 0x4F]
  if (data[0] <= 0x0F) {
    s->observed_master=data[0];    
  } 

  // there can be multiple remotes, so we store them in an array
  if (data[0] >= 0x40 && data[0] < 0x50) {
      bool found = false;
      for (uint8_t i = 0; i < s->observed_remotes_count; i++) {
          if (s->observed_remotes[i] == data[0]) {
              found = true;
              break;
          }
      }
      if (!found && s->observed_remotes_count < MAX_OBSERVED_REMOTES) {
          s->observed_remotes[s->observed_remotes_count++] = data[0];
      }
  }

  /*
    Status
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17
    00 FE 1C 0D 80 81 8D AC 00 00 76 00 33 33 01 00 01 B9
    |  |     ||       |  |         |    |      |- save mode bit0
    |  |     ||       |  |         |    |  |- bit2 HEAT:1 COLD:0
    |  |-Dst |        |  |         |    |- bit2 HEAT:1 COLD:0
    |-Src    |        |  |         |- bit7..bit1  - 35 =Temp
             |        |  |-bit7..bit5 fan mode (auto:010 med:011 high:110 low:101 )
             |        |  |-bit2 ON:1 OFF:0
             |        |-bit7.bit5 (mode cool:010 fan:011 auto 101 heat:001 dry: 100)
             |        |-bit0 ON:1 OFF:0
             |-Byte count


    Extended status, similar to status but with 2 more bytes, indoor temp? and error code?
      0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19
    00 FE 58 0F 80 81 8D A8 00 00 7A 84 E9 00 33 33 01 00 01 9B
                            |     |  |  |-always E9 (error code?)
                            |     |  |-  1000 0100  1000010 66-35=31 (indoor return air temp?)
                            |     |-target temp 0111 1010 111101 61-35 = 26
                            |- bit 7 "filter alert", bit 1 "preheat"       
  */

  //status
  if ((data[2] == 0x1C && data[5] == 0x81)) { //data[2] == 0x1C and data[5] == 0x81
      s->last_cmd_type = MSG_STATUS;
      s->save = data[14] & 0b1;
      s->heat = (data[13] & 0b00000100) >> 2;
      s->cold = !s->heat;
      s->preheat = (data[8] & 0b00000010) >> 1;
      s->filter_alert = (data[8] & 0b10000000) >> 7;  // Extract bit 7 of byte 8

      s->fan  = (data[7] & 0b11100000) >> 5;
      switch (s->fan) {
        case FAN_AUTO:   strcpy(s->fan_str, "AUTO"); break;
        case FAN_MEDIUM: strcpy(s->fan_str, "MED");  break;
        case FAN_HIGH:   strcpy(s->fan_str, "HIGH"); break;
        case FAN_LOW:    strcpy(s->fan_str, "LOW");  break;
        default: strcpy(s->fan_str, "UNK");
      }
      s->mode = (data[6] & 0b11100000) >> 5;
      switch (s->mode) {
        case MODE_COOL: strcpy(s->mode_str, "COOL"); break;
        case MODE_FAN:  strcpy(s->mode_str, "FAN");  break;
        case MODE_AUTO: strcpy(s->mode_str, "AUTO"); break;
        case MODE_HEAT: strcpy(s->mode_str, "HEAT"); break;
        case MODE_DRY:  strcpy(s->mode_str, "DRY");  break;
        default: strcpy(s->mode_str, "UNK");
      }
      s->target_temp = ((data[10] & 0b11111110) >> 1) - 35;
      s->power = data[6] & 0b1;

      print_logf("[STATUS] Power: %d Mode: %s Fan: %s Temp: %d Heat: %d Cold: %d Save: %d Preheat: %d FilterAlert: %d\n",
                 s->power, s->mode_str, s->fan_str, s->target_temp, s->heat, s->cold, s->save, s->preheat, s->filter_alert);
  }
  
  //extended status data[2] == 0x58 and data[5] == 0x81
  if ((data[2] == 0x58) && (data[5] == 0x81)) {
      s->last_cmd_type = MSG_STATUS_EXT;
      /*
        Extended status, similar to status but with 2 more bytes, indoor temp? and error code?
         0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19
        00 FE 58 0F 80 81 8D A8 00 00 7A 84 E9 00 33 33 01 00 01 9B
                                |     |  |  |-always E9 (error code?)
                                |     |  |-  1000 0100  1000010 66-35=31 (indoor return air temp?)
                                |     |-target temp 0111 1010 111101 61-35 = 26
                                |- bit 7 "filter alert", bit 1 "preheat"
      */
      s->save = data[14 + 2] & 0b1;
      s->heat = (data[13 + 2] & 0b100) >> 2;
      s->cold = !s->heat;
      s->fan  = (data[7] & 0b11100000) >> 5;
      switch (s->fan) {
        case FAN_AUTO:   strcpy(s->fan_str, "AUTO"); break;
        case FAN_MEDIUM: strcpy(s->fan_str, "MED");  break;
        case FAN_HIGH:   strcpy(s->fan_str, "HIGH"); break;
        case FAN_LOW:    strcpy(s->fan_str, "LOW");  break;
        default:         strcpy(s->fan_str, "UNK");
      }

      s->mode = (data[6] & 0b11100000) >> 5;
      switch (s->mode) {
        case MODE_COOL: strcpy(s->mode_str, "COOL"); break; // 2
        case MODE_FAN:  strcpy(s->mode_str, "FAN");  break; // 3
        case MODE_AUTO: strcpy(s->mode_str, "AUTO"); break; // 
        case MODE_HEAT: strcpy(s->mode_str, "HEAT"); break; // 1
        case MODE_DRY:  strcpy(s->mode_str, "DRY");  break; // 4
        default:        strcpy(s->mode_str, "UNK");
      }

      s->target_temp = ((data[10] & 0b11111110) >> 1) - 35;
      s->power = data[6] & 0b1;

      s->preheat = (data[8] & 0b00000010) >> 1;
      s->filter_alert = (data[8] & 0b10000000) >> 7;  // Extract bit 7 of byte 8

      //WARNING: not sure
      //s->sensor_temp = (data[11] / 2.0 - 35); //TO CHECK

      print_logf("[EXT STATUS] Power: %d Mode: %s (%02X) Fan: %s Temp: %d Heat: %d Cold: %d Save: %d Preheat: %d FilterAlert: %d Temp?: %.1f Error: %02x\n",
                 s->power, s->mode_str, s->mode, s->fan_str, s->target_temp, s->heat, s->cold, s->save, s->preheat, s->filter_alert, (data[11] / 2.0 - 35), data[12]);
  }  else if ((data[2] == 0x55) && (data[5] == 0x81)) {
      //40 00 55 05 08 81 00 69 00 F0 remote sensor temp
      s->last_cmd_type = MSG_REMOTE_SENSOR_TEMP;
      s->remote_sensor_temp = data[7] / 2.0 - 35;
      print_logf("[REMOTE TEMP] Temp: %.1f ºC\n", s->remote_sensor_temp);
  } else if ((data[2] == 0x10) && (data[5] == 0x8A)) {
    // alive message
    //00 FE 10 02 80 8A E6
    s->last_cmd_type = MSG_MASTER_ALIVE;
    //print_log("[MASTER ALIVE]\n");

  } else if ((data[2] == 0x18) && (data[5] == 0xA1)) {
    // ACK after setting param
    //00 40 18 02 80 A1 7B
    s->last_cmd_type = MSG_MASTER_ACK;
    s->master_busy = false;
    //print_log("[MASTER ACK]\n");
  } else if ((data[2] == 0x11) && (data[5] == 0x86)) {
    // mode status
    //00 52 11 04 80 86 84 05 C0
    s->last_cmd_type = MSG_STATUS_MODE;
    s->mode = data[6] >> 5; //byte6 bit7-bit5

    print_logf("[STATUS MODE] Mode: %s (%d)\n", s->mode_str, s->mode);

    //s->power=data[7] & 1; //byte7 bit0
    //(data[7] >> 2) &0b1 //bit2??

    //00 52 11 04 80 86 84 01 C4   DRY,LOW      0001
    //00 52 11 04 80 86 64 01 24   FAN,LOW/HIGH/MED
    //00 52 11 04 80 86 44 01 04   COOL,LOW     0001       101
    //00 52 11 04 80 86 44 05 00   COOL,MED     0101   ->  100

    //00 52 11 04 80 86 24 00 65   heat low off
    //00 52 11 04 80 86 24 01 64   heat low on
    //00 52 11 04 80 86 24 05 60   heat low on
  } 
  
  //40 00 15 07 08 0c 81 00 00 48 00 9f remote ping
  if (data[2] == 0x15 && data[5] == 0x0c) {
    s->last_cmd_type = MSG_REMOTE_PING;
    //print_logf("[PING] from remote %02x\n", data[0]);
  }

  //00 40 18 08 80 0c 00 03 00 00 48 00 97 master pong
  if (data[2] == 0x18 && data[5] == 0x0c) {
    s->last_cmd_type = MSG_MASTER_PONG;
    //print_logf("[PONG] from master %02x\n", data[0]);
  }

  // 40 00 11 04 08 54 01 00 08   save off
  if (data[2] == 0x11 && data[5] == 0x54) {
    s->last_cmd_type = MSG_SAVE;
    print_logf("[SAVE] mode %d\n", (data[7] & 0b1));
  }
 
  // 40 00 11 09 08 0C 82 00 00 30 05 01 01 EB timer
  if (data[2] == 0x11 && data[5] == 0x0C) {
    s->last_cmd_type = MSG_TIMER;
    uint8_t timer_mode = data[10];
    uint8_t timer_value = data[11]; //or data[12]
    print_logf("[TIMER] Mode: %d Value: %d\n", timer_mode, timer_value);
  }

  //40 f0 15 02 00 08  af  model request
  if (data[2] == 0x15 && data[5] == 0x08) {
    s->last_cmd_type = MSG_MODEL_REQUEST;
    //print_logf("[MODEL REQUEST] from remote %02x\n", data[0]);
  }
  
  //40 00 15 02 08 0f 50 setpoint request
  if (data[2] == 0x15 && data[5] == 0x0f) {
    s->last_cmd_type = MSG_SETPOINT_REQUEST;
    //print_logf("[SETPOINT REQUEST] from remote %02x\n", data[0]);
  }

  //40 00 15 02 08 0a 55 temp limits request
  if (data[2] == 0x15 && data[5] == 0x0a) {
    s->last_cmd_type = MSG_TEMP_LIMITS_REQUEST;
    //print_logf("[TEMP LIMITS REQUEST] from remote %02x\n", data[0]);
  }

  //read model
  if (data[2] == 0x18 && data[5] == 0x08) {
    // Model string answer
    // 00 40 18 14 80 08 52 41 56 2d 53 4d 31 31 30 36 42 54 50 2d 45 20 03 37 8e
    //                   R  A  V  -  S  M  1  1  0  6  B  T  P  -  E  LF ETX %
    // Model string starts at byte 6 and continues for (data[3] - 4) bytes
    s->last_cmd_type = MSG_MODEL_ANSWER;
     
    // Extract model string (starts at byte 6)
    unsigned int model_len = data[3] - 2 - 3; // data[3] is byte count, subtract header bytes and last 3 bytes which are not part of the string
    if (model_len > 0 && model_len < sizeof(s->model_str)) {
      memset(s->model_str, 0, sizeof(s->model_str)); // Clear buffer
      for (unsigned int i = 0; i < model_len && i < sizeof(s->model_str) - 1; i++) {
        s->model_str[i] = (char)data[6 + i];
      }
      s->model_str[model_len] = '\0'; // Null terminate
      print_logf("[MODEL] %s\n", s->model_str);
    }
  }

  //temp limits
  // 00 40 18 10 80 0a 00 2f 0f 80 6a 80 6a 80 6a 80 6a 04 56 00 b0 
  if (data[2] == 0x18 && data[5] == 0x0A) {
    s->last_cmd_type = MSG_TEMP_LIMITS;

    s->temp_limit_max = (data[9]>>1) - 35;
    s->temp_limit_min  = (data[10]>>1) - 35;

    // Frost Protection / Special Mode
    bool frost_protection_enabled = (data[17] == 0x04); // Byte 13: 0x04 = Frost Protection enabled
    if (frost_protection_enabled)
      s->temp_frost_protection = (data[18] >> 1) - 35; // Byte 14: Temperature for frost protection (typically 8°C)
    else
      s->temp_frost_protection = -1; // Indicate frost protection is disabled
      
    print_logf("[LIMITS] Min: %d Max: %d Frost: %d \n", s->temp_limit_min, s->temp_limit_max, s->temp_frost_protection);
  }

  //setpoint status - default temperatures for each mode
  // 00 40 18 09 80 0f 7a 74 78 78 25 52 05 a2
  if (data[2] == 0x18 && data[5] == 0x0F) {
    s->last_cmd_type = MSG_SETPOINT;
    
    // Extract default temperatures for each mode
    // Data bytes 6-9 contain temperatures: (value/2) - 35
    s->temp_default_auto = (data[6] >> 1) - 35;  // 0x7A = 122 -> 61-35 = 26°C
    s->temp_default_heat = (data[7] >> 1) - 35;  // 0x74 = 116 -> 58-35 = 23°C  
    s->temp_default_dry  = (data[8] >> 1) - 35;  // 0x78 = 120 -> 60-35 = 25°C
    s->temp_default_cool = (data[9] >> 1) - 35;  // 0x78 = 120 -> 60-35 = 25°C
    
    print_logf("[SETPOINT] Auto: %d°C, Heat: %d°C, Dry: %d°C, Cool: %d°C\n", 
              s->temp_default_auto, s->temp_default_heat, 
              s->temp_default_dry, s->temp_default_cool);
  }

  //request fan mode capabilities
  //40 00 15 02 08 10 4f
  if (data[2] == 0x15 && data[5] == 0x10) {
    s->last_cmd_type = MSG_FAN_MODES_REQUEST;
    print_logf("[FAN MODES REQUEST]\n");
  } 
  
  //fan mode capabilities response
  //00 40 18 0e 80 10 00 35 33 35 33 35 33 35 33 00 00 00 c6 
  if (data[2] == 0x18 && data[5] == 0x10) {
    s->last_cmd_type = MSG_FAN_MODES_ANSWER;
    
    // Parse fan mode capabilities
    // Payload starts at byte 6, contains 8 bytes for different modes
    // Each byte: high nibble = fan speed count, low nibble = capability flags
    
    // uint8_t payload_start = 6;
    // uint8_t num_modes = data[3] - 2; // Byte count minus header bytes
    
    // // Example parsing for common modes (adjust based on actual protocol)
    // if (num_modes >= 8) {
    //   // Mode order typically: Auto, Cool, Heat, Dry, Fan, etc.
    //   for (int i = 0; i < 8 && i < num_modes; i++) {
    //     uint8_t mode_byte = data[payload_start + i];
    //     uint8_t fan_speeds = (mode_byte >> 4) & 0x0F;  // High nibble
    //     uint8_t capabilities = mode_byte & 0x0F;        // Low nibble
        
    //     print_logf("[FAN_MODES] Mode %d: Speeds=%d, Caps=0x%X\n", 
    //                i, fan_speeds, capabilities);
    //   }
    // }
   
    print_log("[FAN_MODES]\n");
  }

  //00 40 18 12 80 0d 08 00 fe fe fe fe fe fe fe fe fe fe fe fe fe fe cd  -> features
  //after this message, we can consider that remote is registered
  if (data[2] == 0x18 && data[5] == 0x0D) {
    s->last_cmd_type = MSG_FEATURES;
    
    // Update announcement state to LINKED
    if (s->announce_state == ANNOUNCE_BUSY || s->announce_state == ANNOUNCE_UNLINKED) {
      s->announce_state = ANNOUNCE_LINKED;
      print_log("[FEATURES] - Remote announcement COMPLETE - Now LINKED!\n");
    } else {
      print_logf("[FEATURES]\n");
    }
  }

  // 40 00 15 05 08 02 f5 00 01 ae       -> request code 00 with f5
  if (data[2] == 0x15 && data[5] == 0x02) {
    s->last_cmd_type = MSG_DN_CODE_REQUEST;
    s->query_dn_code = data[7]; //byte 7 is requested code
    print_logf("[DN_CODE_REQUEST] Code: 0x%02X\n", s->query_dn_code);
  }
  
  // //report remote sensor temp
  // //40 00 55 05 08 81 00 6b 00 f2
  // if (data[2] == 0x55 && data[5] == 0x81) {
  //   s->last_cmd_type = MSG_REMOTE_SENSOR_TEMP;
  //   s->remote_sensor_temp = (data[7] / 2.0) - 35;
  //   print_logf("[REMOTE SENSOR TEMP] Temp: %.1f\n", s->remote_sensor_temp);
  // }

  //DN code response
  // 40 00 15 05 08 02 f5 00 01 ae       -> request code 00 with f5
  // 00 40 18 07 80 02 01 02 05 00 00 db -> byte 6 next code 01, byte 7 value 02
  if (data[2] == 0x18 && data[5] == 0x02) {
    s->last_cmd_type = MSG_DN_CODE;
    
    //uint8_t next_code = data[6];
    uint8_t value = data[7];

    // Store in array
    if (s->dn_codes_count < MAX_DN_CODES) {
      s->dn_codes[s->dn_codes_count].code = s->query_dn_code;
      s->dn_codes[s->dn_codes_count].value = value;
      //s->dn_codes[s->dn_codes_count].next_code = next_code;
      s->dn_codes_count++;
    }
    
    print_logf("[DN_CODE] Value: 0x%02X\n", value);
  }

  //announcement message
  //40 f0 15 02 00 0d aa
  if (data[2] == 0x15 && data[5] == 0x0D) {
    s->last_cmd_type = MSG_ANNOUNCE;
    print_logf("[ANNOUNCE] from remote %02x\n", data[0]);
  }

  //master busy status
  //00 40 18 02 80 a3 79
  if (data[2] == 0x18 && data[5] == 0xA3) {
    s->last_cmd_type = MSG_MASTER_BUSY;
    s->master_busy = true;
  }

  //40 00 15 04 08 07 00 c2 9c -> request power save ratio
  if (data[2] == 0x15 && data[5] == 0x07) {
    s->last_cmd_type = MSG_SAVE_RATIO_REQUEST;
    print_logf("[SAVE RATIO]\n");
  }

  // 00 40 18 03 80 07 4b 97 -> received power save ratio value
  if (data[2] == 0x18 && data[5] == 0x07) {
    s->last_cmd_type = MSG_SAVE_RATIO_ANSWER;
    s->power_save_ratio = data[6];
    print_logf("[SAVE RATIO ANSWER] %d\n", data[6]);
  }

  //40 00 15 06 08 E8 00 01 00 9E 2C -> time counter?
  if (data[2] == 0x15 && data[5] == 0xE8) {
    s->last_cmd_type = MSG_TIME_COUNTER;
    print_logf("[TIME COUNTER]\n");
  }
  
  //00 40 18 07 80 e8 00 01 00 01 83 b4 -> time counter answer
  if (data[2] == 0x18 && data[5] == 0xE8) {
    s->last_cmd_type = MSG_TIME_COUNTER_ANSWER;
    //time counter value in byte 6 and 7
    uint16_t time_counter = (data[6] << 8) | data[7];
    print_logf("[TIME COUNTER ANSWER] %d\n", time_counter);
  }

  //41 00 11 03 08 41 03 19 -> power on
  if (data[2] == 0x11 && data[5] == 0x41) {
    s->last_cmd_type = MSG_POWER;
    print_logf("[POWER] %d\n", data[6]&0b1);
  }
 
  // 40 00 11 08 08 4C 0C 1D 7A 00 33 33 76 -> set temp
  if (data[2] == 0x11 && data[4] == 0x08 && data[5] == 0x4C) {
    s->last_cmd_type = MSG_SET_TEMP_FAN;
    print_logf("[SET TEMP] %d\n", (data[6] >> 1) - 35);
  }

  // 40 00 11 03 08 42 04 1C -> set mode
  if (data[2] == 0x11 && data[5] == 0x42) {
    s->last_cmd_type = MSG_SET_MODE;
    print_logf("[SET MODE] %d\n", data[6]);
  }

  //sensor reading
  if ((data[2] == 0x1A) && (data[5] == 0xEF)) {
    s->last_cmd_type = MSG_SENSOR_ANSWER;

    if (data[8] == 0xA2) {
      //00 40 1A 05 80 EF 80 00 A2 12   //undefined value
      s->sensor_val = -1;
    } else if (data[8] == 0x2C) {
      //00 40 1A 07 80 EF 80 00 2C 00 00 9E
      s->sensor_val = data[9] * 256 + data[10]; //answer does not report query id, so we should assign it to the last queried sensor
      //INDOOR_ROOM, INDOOR_TA, INDOOR_TCJ, INDOOR_TC, FILTER_TIME, OUTDOOR_TE, OUTDOOR_TO, OUTDOOR_TD, OUTDOOR_TS, OUTDOOR_THS, OUTDOOR_CURRENT, OUTDOOR_HOURS
      switch (s->sensor_id) {
        case INDOOR_ROOM: s->remote_sensor_temp = s->sensor_val; break;
        case INDOOR_TA:   s->indoor_ta = s->sensor_val;  break;
        case INDOOR_TCJ:  s->indoor_tcj = s->sensor_val;  break;
        case INDOOR_TC:   s->indoor_tc = s->sensor_val;  break;
        case INDOOR_FILTER_TIME: s->indoor_filter_time = s->sensor_val;  break;
        case OUTDOOR_TE:  s->outdoor_te = s->sensor_val;  break;
        case OUTDOOR_TO:  s->outdoor_to = s->sensor_val;  break;
        case OUTDOOR_TD:  s->outdoor_td = s->sensor_val;  break;
        case OUTDOOR_TS:  s->outdoor_ts = s->sensor_val;  break;
        case OUTDOOR_THS: s->outdoor_ths = s->sensor_val;  break;
        case OUTDOOR_CURRENT: s->outdoor_current = s->sensor_val;  break;
        case OUTDOOR_HOURS:   s->outdoor_cumhour = s->sensor_val;  break;

        case INDOOR_FAN_RUN_TIME: s->indoor_fan_run_time = s->sensor_val;  break; //0xF2 //Fan Run Time (x 100h)
        case INDOOR_FAN_SPEED:    s->indoor_fan_speed = s->sensor_val;  break; //

        case OUTDOOR_TL:  s->outdoor_tl = s->sensor_val;  break;
        case OUTDOOR_COMP_FREQ:  s->outdoor_comp_freq = s->sensor_val;  break;
        case OUTDOOR_LOWER_FAN_SPEED:  s->outdoor_lower_fan_speed = s->sensor_val;  break;
        case OUTDOOR_UPPER_FAN_SPEED:  s->outdoor_upper_fan_speed = s->sensor_val;  break;

        default:
          break;
      }
    }
    print_logf("[SENSOR] id: 0x%x val: %d\n", s->sensor_id, s->sensor_val);
  }

  //40 00 17 08 08 80 ef 00 2c 08 00 02 1e -> sensor query
  if ((data[2] == 0x17) && (data[5] == 0x80)) {
    s->last_cmd_type = MSG_SENSOR_QUERY;   
  }

  if ((data[2] == 0x18) && (data[5] == 0x27)) {
    s->last_cmd_type = MSG_SENSOR_ERROR;
    // answer 00 40 18 05 80 27 08 00 48 ba
    //                                |---------Type 0x4 Num error 0x8   E-08 in remote screen
    s->error_type = data[8] >> 4;
    s->error_val = data[8] & 0b00001111;
  }

  return s->last_cmd_type;

} // end air_decode_command

// Utility function to print buffer state (for debugging)
void print_buffer_state(air_status_t *air) {
    Serial.printf("[BUFFER] Read: %d, Write: %d, Available: %d\n", 
                  air->curr_r_idx, air->curr_w_idx,
                  (air->curr_w_idx - air->curr_r_idx + MAX_RX_BUFFER) % MAX_RX_BUFFER);
}


/*____________________________RING BUFFER HELPERS_______________________________ */

// Initialize ring buffer
void rb_init(ring_buffer_t *rb) {
  rb->write_idx = 0;
  rb->read_idx = 0;
  rb->count = 0;
}

// Get number of available bytes in buffer
inline int rb_available(ring_buffer_t *rb) {
  return rb->count;
}

// Write byte to ring buffer (returns false if buffer full)
inline bool rb_write(ring_buffer_t *rb, uint8_t byte) {
  if (rb->count >= MAX_RX_BUFFER) {
    return false; // Buffer full
  }
  
  rb->buffer[rb->write_idx] = byte;
  rb->write_idx = (rb->write_idx + 1) % MAX_RX_BUFFER;
  rb->count++;
  return true;
}

// Read byte from ring buffer (without removing it)
inline uint8_t rb_peek(ring_buffer_t *rb, int offset) {
  int idx = (rb->read_idx + offset) % MAX_RX_BUFFER;
  return rb->buffer[idx];
}

// Remove one byte from ring buffer
inline void rb_consume(ring_buffer_t *rb, int count) {
  if (count > rb->count) {
    count = rb->count;
  }
  rb->read_idx = (rb->read_idx + count) % MAX_RX_BUFFER;
  rb->count -= count;
}

/*____________________________MESSAGE RECOVERY_______________________________ */

// Attempt to recover corrupted packets by fixing common address errors
// Returns true if packet was successfully recovered
bool air_recover_message(air_status_t *air, byte *cmd, uint8_t packet_len) {
  // Common addresses to try
  uint8_t common_sources[] = {MASTER, REMOTE, BCAST, 
                               air->master, air->remote, 
                               air->observed_master};
  uint8_t common_dests[] = {MASTER, REMOTE, BCAST, 
                             air->master, air->remote,
                             air->observed_master};
  
  // Add observed remotes to common addresses
  for (int i = 0; i < air->observed_remotes_count && i < MAX_OBSERVED_REMOTES; i++) {
    if (air->observed_remotes[i] != 0xFF) {
      // These could be either source or destination
    }
  }
  
  byte original_src = cmd[FROM];
  byte original_dst = cmd[TO];
  
  // Strategy 1: Try fixing source address
  for (int i = 0; i < sizeof(common_sources) / sizeof(common_sources[0]); i++) {
    cmd[FROM] = common_sources[i];
    
    if (check_crc(cmd, packet_len)) {
      print_logf("[RECOVER] Fixed SRC: 0x%02X -> 0x%02X Len: %d\n", original_src, cmd[FROM], packet_len);
      return true;
    }
  }
  
  // Restore original source
  cmd[FROM] = original_src;
  
  // Strategy 2: Try fixing destination address
  for (int i = 0; i < sizeof(common_dests) / sizeof(common_dests[0]); i++) {
    cmd[TO] = common_dests[i];
    
    if (check_crc(cmd, packet_len)) {
      print_logf("[RECOVER] Fixed DST: 0x%02X -> 0x%02X Len: %d\n", original_dst, cmd[TO], packet_len);
      return true;
    }
  }
  
  // Restore original destination
  cmd[TO] = original_dst;
  
  // Strategy 3: Try fixing both source AND destination
  for (int i = 0; i < sizeof(common_sources) / sizeof(common_sources[0]); i++) {
    for (int j = 0; j < sizeof(common_dests) / sizeof(common_dests[0]); j++) {
      cmd[FROM] = common_sources[i];
      cmd[TO] = common_dests[j];
      
      if (check_crc(cmd, packet_len)) {
        print_logf("[RECOVER] Fixed SRC+DST: 0x%02X->0x%02X, 0x%02X->0x%02X Len: %d\n", 
                   original_src, cmd[FROM], original_dst, cmd[TO], packet_len);
        return true;
      }
    }
  }
  
  // Recovery failed - restore original values
  cmd[FROM] = original_src;
  cmd[TO] = original_dst;
  
  return false;
}

/*____________________________PARSE FUNCTION_______________________________ */

// New optimized parse function with ring buffer and recovery
int air_parse_serial(air_status_t *air) {
  SoftwareSerial *ss = &(air->serial);
  ring_buffer_t *rb = &(air->rb);
  bool decoded_any = false;
  byte cmd[MAX_CMD_BUFFER];
  
  // STEP 1: Read all available serial data into ring buffer
  while (ss->available()) {
    byte ch = (byte)ss->read();
    
    if (!rb_write(rb, ch)) {
      print_log("[WARN] RX buffer full, dropping byte!\n");
      break;
    }
  }
  
  // Debug output AFTER all data is safely in buffer
  int initial_available = rb_available(rb);
  #ifdef DEBUG
  if (initial_available > 0) {
    Serial.print("[RCV]");
    for (int k = 0; k < initial_available; k++) {
      byte b = rb_peek(rb, k);
      Serial.print(b < 0x10 ? " 0" : " ");
      Serial.print(b, HEX);
    }
    Serial.println();
  }
  #endif
  
  // STEP 2: Parse messages from buffer
  while (rb_available(rb) >= MIN_PACKET_SIZE) {
    
    // Declare variables BEFORE any goto labels
    uint8_t packet_len;
    bool valid_packet;
    bool recovered;
    
    // Check if we have a potential valid packet
    // Byte 3 contains the data length
    if (rb_available(rb) < 4) {
      break; // Need at least 4 bytes to read length
    }
    
    packet_len = rb_peek(rb, 3) + 5; // Length byte + header (4) + CRC (1)
    
    // Validate packet length
    if (packet_len < MIN_PACKET_SIZE || packet_len > MAX_CMD_BUFFER) {
      // Invalid length - discard first byte
      goto discard_byte;
    }
    
    // Check if we have enough data for complete packet
    if (rb_available(rb) < packet_len) {
      break; // Wait for more data
    }
    
    // Copy packet data for CRC check
    for (int k = 0; k < packet_len; k++) {
      cmd[k] = rb_peek(rb, k);
    }
    
    // Validate CRC - try recovery if it fails
    valid_packet = check_crc(cmd, packet_len);
    recovered = false;
    
    if (!valid_packet) {
      // Attempt message recovery
      recovered = air_recover_message(air, cmd, packet_len) && (air_decode_command(cmd, air) != MSG_UNKNOWN);
      valid_packet = recovered;      
    }
    
    if (valid_packet) {
      // Valid packet found!
      Serial.print("\n[CMD]");
      if (recovered) {
        Serial.print("[RECOVERED]");
        air->recovered_msgs++;

        //print info about recovered message, packet len
        print_logf("[RECOVERED MSG] Len: %d From: 0x%02X To: 0x%02X\n", 
                   packet_len, cmd[FROM], cmd[TO]);
      }
      
      // Build command string with recovery marker
      if (recovered) {
        air->buffer_cmd += "<REC>"; // Mark start of recovered message
      }
      
      for (int k = 0; k < packet_len; k++) {
        Serial.print(cmd[k] < 0x10 ? " 0" : " ");
        Serial.printf("%02X", cmd[k]);
        air->last_cmd[k] = cmd[k];
        air->buffer_cmd += ((cmd[k] < 0x10) ? "0" : "") + String(cmd[k], HEX) + " ";
        // Add to raw buffer with recovery marker
        if (recovered) {
          air->buffer_raw += "<REC>";
        }
        air->buffer_raw += ((cmd[k] < 0x10) ? "0" : "") + String(cmd[k], HEX) + " ";
        if (recovered) {
          air->buffer_raw += "</REC>";
        }
      }
      
      if (recovered) {
        air->buffer_cmd += "</REC>"; // Mark end of recovered message
       
      }
      
      Serial.println();
      
      // Decode the command
      air_decode_command(cmd, air);
      
      // Update statistics
      air->decoded_msgs++;
      air->decoded_bytes += packet_len;
      decoded_any = true;
      
      // Remove processed packet from buffer
      rb_consume(rb, packet_len);
      
      // Reset discard sequence flag
      air->in_discard_sequence = false;
      
      // Notify WebSocket
      notifyTXRXData();
      
      continue;
    }
    
discard_byte:
    // Invalid packet - discard first byte
    byte discarded = rb_peek(rb, 0);
    
#ifdef DEBUG
    Serial.print("\n[SKIP] ");
    Serial.println(discarded, HEX);
#endif
    
    // Mark discarded byte in raw buffer
    air->buffer_raw += "<ERR>" + String((discarded < 0x10) ? "0" : "") + String(discarded, HEX) + "</ERR> ";
    air->discarded_bytes++;
    
    // Count new discard sequence
    if (!air->in_discard_sequence) {
      air->discarded_msgs++;
      air->in_discard_sequence = true;
    }
    
    // Remove discarded byte
    rb_consume(rb, 1);
  }
  
  // Send any remaining buffered data
  if (air->buffer_raw.length() > 0 || air->buffer_cmd.length() > 0) {
    notifyTXRXData();
  }
  
  return decoded_any;
}



// reads serial and gets a valid command in air.rx_data
// calls decode command to fill air structure
// returns true if one or command commands are decoded, false otherwise
int air_parse_serialKK(air_status_t *air) {
  int i, j_init, k;
  uint8_t mylen = 0;
  byte ch;
  byte cmd[MAX_CMD_BUFFER];
  int i_start, segment_len;
  bool found = false;
  bool rbuffer = false;
  int retval = false;
  bool in_discard_sequence = false;  // Track if we're in a sequence of discarded bytes

  // circular buffer avoids loosing parts of messages but it is not working so rbuffer=false to avoid crashes
  if (!rbuffer)
    air->curr_w_idx = air->curr_r_idx = 0; //no circular buffer - with this some bytes will be lost but not a big problem

  i = air->curr_w_idx; i_start = air->curr_r_idx;

  //Serial.printf("curr_w %d curr_r %d (rbuff %d)\n", air->curr_w_idx, air->curr_r_idx, rbuffer);

  SoftwareSerial *ss;
  ss = &(air->serial);

  //STEP 1 producer reads from serial
  //Serial.print("Receiving data ");
  while (ss->available()) {
    ch = (byte)ss->read();
    air->rx_data[i] = ch;
    i = (i + 1) % MAX_RX_BUFFER;
    air->buffer_raw += ((ch < 0x10) ? "0" : "")  + String(ch, HEX) + " ";
  }

  if (i_start < i) Serial.print("[RCV] "); //print_log("[RCV] ");
  for (k = i_start; k < i; k++) {
    Serial.print(air->rx_data[k] < 0x10 ? " 0" : " ");
    Serial.print(air->rx_data[k], HEX);
  }

  //STEP 2 consumer parses data and fill air_status structure
  //try all combinations
  for (j_init = i_start; ((j_init < i) && !rbuffer) || ((j_init != i) && rbuffer); j_init = (j_init + 1) % MAX_RX_BUFFER) {

    segment_len = air->rx_data[(j_init + 3) % MAX_RX_BUFFER] + 5; //packet is byte size plus 5
    if ((segment_len <  MAX_CMD_BUFFER )) { //max size of cmd packets are 32 bytes and not exceed last written byte

#ifdef DEBUG_PLUS
      Serial.println("");
      Serial.print("Try: ");
      Serial.print("("); Serial.print(j_init); Serial.print(")");
#endif
      //get message according to length byte
      for (k = 0; (k < segment_len && k < MAX_CMD_BUFFER); k++) {
        cmd[k] = air->rx_data[(j_init + k) % MAX_RX_BUFFER];
#ifdef DEBUG_PLUS
        //Serial.printf("[%d] ",(j_init + k) % MAX_RX_BUFFER);
        if (((j_init + k) % MAX_RX_BUFFER) >= i) Serial.printf("** idx %d cur_w %d\n", (j_init + k) % MAX_RX_BUFFER , i);
        Serial.print(cmd[k] < 0x10 ? " 0" : " ");
        Serial.print(cmd[k], HEX);
#endif
      }


      //if valid crc, decode data
      if (check_crc(cmd, segment_len)) {
        mylen = cmd[3] + 5;
        Serial.println("");//print_logf("\n");
        Serial.print("[CMD] ");//print_log("[CMD] ");
        for (k = 0; k < mylen; k++) {
          Serial.print(cmd[k] < 0x10 ? " 0" : " ");//print_log(cmd[k] < 0x10 ? " 0" : " ");
          Serial.printf("%02X", cmd[k]);//print_logf("%02X", cmd[k]);
          air->last_cmd[k] = cmd[k]; //add cmd to last_cmd
          air->buffer_cmd += ((air->last_cmd[k] < 0x10) ? "0" : "")  + String(air->last_cmd[k], HEX) + " ";
        }
        //air->buffer_cmd += "<br>";

        Serial.println();//print_logf("\n");
        if (mylen > 4) { //min valid package is 5 bytes long with 0 data bytes
          air_decode_command(cmd, air);
          air->decoded_msgs = air->decoded_msgs + 1;
          air->decoded_bytes = air->decoded_bytes + mylen;
          retval = true;

          // Send debug data immediately via WebSocket
          notifyTXRXData();

        }
        i_start = (j_init + segment_len) % MAX_RX_BUFFER;
        j_init = (i_start - 1 + MAX_RX_BUFFER) % MAX_RX_BUFFER;
        //j_end = j_init;
        found = true;
        
        // End of discard sequence (if any)
        if (in_discard_sequence) {
          in_discard_sequence = false;
        }
      } //end if crc
    } //end if segment_len

    if (!found) {
#ifdef DEBUG
      Serial.println(""); Serial.print("[SKIP] ");
      Serial.print("("); Serial.print(j_init); Serial.print(")");
      Serial.println(air->rx_data[j_init % MAX_RX_BUFFER], HEX);
#endif

      // Add discarded byte to buffer with error marker
      byte discarded = air->rx_data[j_init % MAX_RX_BUFFER];
      air->buffer_raw += "<ERR>" + String((discarded < 0x10) ? "0" : "") + String(discarded, HEX) + "</ERR> ";
      
      i_start = j_init;
      //Serial.printf("NOT i_start %d, j_init %d, j_end %d\n", i_start, j_init, j_end);
      air->discarded_bytes = air->discarded_bytes + 1;
      
      // Count a new discarded message sequence when transitioning from valid to discarded
      if (!in_discard_sequence) {
        air->discarded_msgs = air->discarded_msgs + 1;
        in_discard_sequence = true;
      }
    }
    found = false;

  } //end for j_init

  //Serial.printf("END i_start %d, j_init %d, j_end %d\n", i_start, j_init, j_end);
  //circular buffer avoids loosing parts of messages but it is not working
  air->curr_w_idx = i;
  air->curr_r_idx = i_start;

  // FIX: Send any accumulated data (errors or raw bytes) that wasn't sent by a valid packet event
  if (air->buffer_raw.length() > 0 || air->buffer_cmd.length() > 0) {
      notifyTXRXData();
  }

  return retval;
}

void print_data(byte * data, int i, int j) {
  int k;
  Serial.print("Cmd:_");
  for (k = i; k < j; k++) {
    Serial.print(data[k] < 0x10 ? " 0" : " ");
    Serial.print(data[k], HEX);
  }
  Serial.println();
}

void air_send_data(air_status_t *air, byte * data, int len) {
  int i;

  SoftwareSerial *ss;
  ss = &(air->serial);

  ss->enableIntTx(true); //enable TX

#ifdef DEBUG
  String message = "[SEND] ";
  for (i = 0; i < len; i++) {
    message += (data[i] < 0x10 ? "0" : "") + String(data[i], HEX) + " ";
  }
  print_log(message);
  //print_logf("[SEND] (%d)\n", len); //Serial.printf("[SEND] (%d) ", len);
#endif
  
  for (i = 0; i < len; i++) {
    ss->write(data[i]);
  }

  ss->enableIntTx(false); //disable TX
  //copy cmd in last_cmd
  for (i = 0; i < len; i++) {
    air->last_cmd[i] = data[i];    
  }
}

void air_get_error(air_status_t *air, uint8_t id)  {
  //       byte    00    01    02    03    04    05    06    CRC
  //               40    f0    15    03    08    27    01    78
  byte data[] = {air->remote, air->master, 0x15, 0x03, 0x08, 0x27, 0x01, 0x78};

  data[6] = id;
  data[7] = XORChecksum8(data, sizeof(data) - 1);

  air->error_id = id;
  air->error_val = -1;
  air_send_data(air, data, sizeof(data));
  unsigned long time_now = millis();
  while (millis() - time_now < 70) { //avoid delay
  }
  air_parse_serial(air);
  air->error_id = 0xff; //dummy value: spurious values received  will be assigned to it

  //yield();
}

void air_query_sensor(air_status_t *air, uint8_t id)  {
  //       byte  00      01      02    03    04    05    06    07    08    09    10    11    CRC
  byte data[] = {air->remote, air->master, 0x17, 0x08, 0x08, 0x80, 0xEF, 0x00, 0x2C, 0x08, 0x00, 0x02, 0x1E};

  data[11] = id;
  data[12] = XORChecksum8(data, sizeof(data) - 1);

  air->sensor_id = id;
  air->sensor_val = -1;
  air_send_data(air, data, sizeof(data));

  unsigned long time_now = millis();
  while (millis() - time_now < 70) { //avoid delay
  }
  air_parse_serial(air); // get result
  air->sensor_id = 0xff; //dummy value: spurious values received  will be assigned to it

  yield();
}


/* byte ids[] = {//INDOOR_ROOM,
  INDOOR_FAN_SPEED,
  INDOOR_TA, INDOOR_TCJ, INDOOR_TC,
  //INDOOR_FILTER_TIME,
  // INDOOR_FAN_RUN_TIME,
  OUTDOOR_TE, OUTDOOR_TO,
  OUTDOOR_TD, OUTDOOR_TS, OUTDOOR_THS,
  OUTDOOR_CURRENT
  //OUTDOOR_HOURS, OUTDOOR_TL, OUTDOOR_COMP_FREQ,
  //OUTDOOR_LOWER_FAN_SPEED, OUTDOOR_UPPER_FAN_SPEED
}; */

void air_query_sensors(air_status_t *air, const byte *ids, size_t num_ids) {
    for (size_t i = 0; i < num_ids; i++) {
        air_query_sensor(air, ids[i]);
        air_parse_serial(air); // get result
        yield(); // allow other tasks to run
    }
}

//utility function to discover used sensors
void air_explore_all_sensors(air_status_t *air)  {
  int i = 0;
  String s;
  for (i = 0xa0; i <= 0xff; i++) {
    air_query_sensor(air, i);
    if (air->sensor_val != -1) {
      print_logf("%02x - %d\n", i, air->sensor_val);
      //s=s+air->sensor_id+ air->sensor_val
    }
  }
}

void reset_observed_remotes(air_status_t *air) {
    air->observed_remotes_count = 0;
    for (int i = 0; i < MAX_OBSERVED_REMOTES; i++) {
        air->observed_remotes[i] = 0xFF;
    }
}

void air_announce_remote(air_status_t *air){
  //       byte         00    01    02    03    04    05   CRC
  //                    XX    f0    15    02    00    0d   aa
  byte data[] = {air->remote, 0xf0, 0x15, 0x02, 0x00, 0x0d, 0xaa};

  data[6] = XORChecksum8(data, sizeof(data) - 1);
  air_send_data(air, data, sizeof(data));
}

// Start announcement process
void air_start_announcement(air_status_t *air) {
  air->announce_state = ANNOUNCE_UNLINKED;
  air->announce_last_time = 0;
  air->announce_retry_count = 0;
  print_log("[ANNOUNCE] Announcement process initialized to UNLINKED\n");
}

void air_handle_announcement(air_status_t *air) {
  unsigned long current_time = millis();
  
  switch (air->announce_state) {
    case ANNOUNCE_UNLINKED:
      // Send announcement message
      air_announce_remote(air);
      air_announce_remote(air);
      air->announce_state = ANNOUNCE_BUSY;
      air->announce_last_time = current_time;
      air->announce_retry_count = 0;
      print_log("[ANNOUNCE] Sent announcement - waiting for BUSY\n");
      break;
      
    case ANNOUNCE_BUSY:
      // Waiting for FEATURES response after receiving BUSY
      // If timeout, retry announcement
      if (current_time - air->announce_last_time > ANNOUNCE_RETRY_INTERVAL) {
        if (air->announce_retry_count < ANNOUNCE_MAX_RETRIES) {
          air_announce_remote(air);
          air->announce_last_time = current_time;
          air->announce_retry_count++;
          print_logf("[ANNOUNCE] Timeout waiting for response - Retry %d/%d\n", 
                     air->announce_retry_count, ANNOUNCE_MAX_RETRIES);
        } else {
          print_log("[ANNOUNCE] Max retries reached, resetting to UNLINKED\n");
          air->announce_state = ANNOUNCE_UNLINKED;
          air->announce_retry_count = 0;
        }
      }
      break;
      
    case ANNOUNCE_LINKED:
      // Successfully linked - do nothing
      // Could add periodic re-announcement here if needed
      break;
  }
}

// to unlink in hardware, power cycle the unit
// this function just resets the announce state
void air_unlink_remote(air_status_t *air) {
  air->announce_state = ANNOUNCE_UNLINKED;
  air->announce_last_time = 0;
  air->announce_retry_count = 0;
  print_log("[ANNOUNCE] Remote unlinked - state set to UNLINKED\n");
} 

// Check if remote is successfully announced/linked
bool air_is_announced(air_status_t *air) {
  return (air->announce_state == ANNOUNCE_LINKED);
}

// 40 00 15 02 08 0a 55 -> request temp limit
// 00 40 18 10 80 0a 00 2f 0f 80 6a 80 6a 80 6a 80
// 40 f0 15 02 00 08 af -> request model
// 00 40 18 14 80 08 52 41 56 2d 53 4d 31 31 30 36 42 54 50 2d 45 20 03 37 8e -> "RAV-SM1106BTP-E" from 52 to 45, the rest is non printable
// 40 00 15 04 08 07 00 c2 9c -> power save ratio
// 00 40 18 03 80 07 4b 97 -> answer 4b = 75%

void air_request_temp_limits(air_status_t *air) {
  //       byte         00    01    02    03    04    05    CRC
  //       cmd         src   dst   cmd   len  type    op 
  //                    40    00    15    02    08    0a    55
  byte data[] = {air->remote, 0x00, 0x15, 0x02, 0x08, 0x0a, 0x55};

  data[6] = XORChecksum8(data, sizeof(data) - 1);
  air_send_data(air, data, sizeof(data));
}

void air_request_model(air_status_t *air) {
  //       byte         00    01    02    03    04    05    CRC
  //                    40    f0    15    02    00    08    af
  byte data[] = {air->remote, 0xf0, 0x15, 0x02, 0x00, 0x08, 0xaf};

  data[6] = XORChecksum8(data, sizeof(data) - 1);
  air_send_data(air, data, sizeof(data));
}

void air_request_power_save_ratio(air_status_t *air) {
  //       byte         00    01    02    03    04    05    06   07   CRC
  //                    40    00    15    04    08    07    00   c2   9c
  byte data[] = {air->remote, 0x00, 0x15, 0x04, 0x08, 0x07, 0x00, 0xc2, 0x9c};

  data[8] = XORChecksum8(data, sizeof(data) - 1);
  air_send_data(air, data, sizeof(data));
}

void air_request_setpoint_defaults(air_status_t *air) {
  //       byte         00    01    02    03    04    05    CRC
  //                    40    00    15    02    08    0f    50
  byte data[] = {air->remote, 0x00, 0x15, 0x02, 0x08, 0x0f, 0x50};

  data[6] = XORChecksum8(data, sizeof(data) - 1);
  air_send_data(air, data, sizeof(data));
}

void request_master_info(air_status_t *air) {
  unsigned int i, retries=3;

  for(i=0; i< retries; i++){
    air_request_model(air);  
    air_parse_serial(air); // get result  
    yield(); // allow other tasks to run

    air_request_power_save_ratio(air);
    air_parse_serial(air);
    yield();

    air_request_setpoint_defaults(air);
    air_parse_serial(air);
    yield();

    air_request_temp_limits(air);
    air_parse_serial(air);
    yield();
  }
}

void air_get_time_counter(air_status_t *air){
  //       byte    00    01    02    03    04    05    06    07    08    09    CRC
  //               40    00    15    06    08    E8    00    01    00    9E    2C
  byte data[] = {air->remote, air->master, 0x15, 0x06, 0x08, 0xE8, 0x00, 0x01, 0x00, 0x9E, 0x2C};

  data[10] = XORChecksum8(data, sizeof(data) - 1);
  air_send_data(air, data, sizeof(data));
}

// Request current DN code value
void air_request_dn_code(air_status_t *air, uint8_t code, uint8_t mode, uint8_t offset) {
  //       byte         00    01    02    03    04    05    06    07    08    CRC
  //                    XX    00    15    05    08    02    f5    00    01    yy
  byte data[] = {air->remote, 0x00, 0x15, 0x05, 0x08, 0x02, mode, code, offset, 0xae};

  air->query_dn_code = code;

  data[9] = XORChecksum8(data, sizeof(data) - 1);
  air_send_data(air, data, sizeof(data));
}

// Request next DN code
void air_request_dn_next(air_status_t *air, uint8_t current_code) {
  //       byte         00    01    02    03    04    05    06    07    08    CRC
  //                    XX    00    15    05    08    02    f1    xx    01    yy  
  air_request_dn_code(air, current_code, 0xf1, 0x01);
}

void air_request_db_previous(air_status_t *air, uint8_t current_code) {
  //       byte         00    01    02    03    04    05    06    07    08    CRC
  //                    XX    00    15    05    08    02    f2    xx    01    yy  
  air_request_dn_code(air, current_code, 0xf2, 0x01);
}

// Simple non-blocking DN code request - webpage controls the scanning
void air_request_dn_code_current(air_status_t *air, uint8_t code) {
  air_request_dn_code(air, code, 0xf5, 0x00);
  
  print_logf("[DN_REQUEST] Requesting code 0x%02X\n", code);
}

void air_send_test_data(air_status_t *air) {
  unsigned int i;

  SoftwareSerial *ss;
  ss = &(air->serial);

  const unsigned char testdata[] = {
    0x40, 0x00, 0x11, 0x03, 0x08, 0x42, 0x05, 0x1D,
    0x00, 0x40, 0x18, 0x02, 0x80, 0xA1, 0x7B,
    0x00, 0xFE, 0x10, 0x02, 0x80, 0x8A, 0xE6,
    0x00, 0x40, 0x18, 0x02, 0x80, 0xA1, 0x7B,
    0x00,
    0x00, 0xFE, 0x1C, 0x0D, 0x80, 0x81, 0x8D, 0xAC, 0x00, 0x00, 0x76, 0x00, 0x33, 0x33, 0x01, 0x00, 0x01, 0xB9,
    0x00, 0xFE, 0x1C, 0x0D, 0x80, 0x81, 0xCD, 0x8C, 0x00, 0x00, 0x76, 0x00, 0x33, 0x33, 0x01, 0x00, 0x01, 0xD9,
    0x00, 0xFE, 0x58, 0x0F, 0x80, 0x81, 0x8C, 0xA8, 0x00, 0x00, 0x7A, 0x84, 0xE9, 0x00, 0x33, 0x33, 0x01, 0x00, 0x01, 0x90,
    0x00, 0xFE, 0x58, 0x0F, 0x80, 0x81, 0x8D, 0xAC, 0x00, 0x00, 0x7A, 0x7D, 0xE9, 0x00, 0x33, 0x33, 0x01, 0x00, 0x01, 0x67,
    0x40, 0x00, 0x11, 0x03, 0x08, 0x42, 0x05, 0x1D,
    0x00, 0x40, 0x18, 0x02, 0x80, 0xA1, 0x7B,
    0x00, 0xFE, 0x10, 0x02, 0x80, 0x8A, 0xE6,
    0x00, 0x40, 0x18, 0x02, 0x80, 0xA1, 0x7B,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xFE, 0x1C, 0x0D, 0x80, 0x81, 0x8D, 0xAC, 0x00, 0x00, 0x76, 0x00, 0x33, 0x33, 0x01, 0x00, 0x01, 0xB9,
    0x00, 0xFE, 0x1C, 0x0D, 0x80, 0x81, 0xCD, 0x8C, 0x00, 0x00, 0x76, 0x00, 0x33, 0x33, 0x01, 0x00, 0x01, 0xD9,
    0x00, 0xFE, 0x58, 0x0F, 0x80, 0x81, 0x8C, 0xA8, 0x00, 0x00, 0x7A, 0x84, 0xE9, 0x00, 0x33, 0x33, 0x01, 0x00, 0x01, 0x9b, //9b is correct crc
    0x00, 0xFE, 0x58, 0x0F, 0x80, 0x81, 0x8D, 0xAC, 0x00, 0x00, 0x7A, 0x7D, 0xE9, 0x00, 0x33, 0x33, 0x01, 0x00, 0x01, 0x67
  };

  ss->enableIntTx(true);

  Serial.println("");
  Serial.print("Sending test data (");
  Serial.print(sizeof(testdata));
  Serial.println(")");

  for (i = 0; i < sizeof(testdata); i++) {
    ss->write(testdata[i]);
    //    Serial.print(testdata[i] < 0x10 ? " 0" : " ");
    //    Serial.print(testdata[i], HEX);
  }
  Serial.println("");

  ss->enableIntTx(false);

}

/*
alive           00 fe 10 02 80 8a e6
params          00 52 11 04 80 86 44 01 04
params          00 52 11 04 80 86 44 05 00
alive           00 fe 10 02 80 8a e6
ping            40 00 15 07 08 0c 81 00 00 48 00 9f
pong(ping resp) 00 40 18 08 80 0c 00 03 00 00 48 00 97
alive           00 fe 10 02 80 8a e6
temperature   * 40 00 55 05 08 81 00 7b 00 e2
extend status   00 fe 58 0f 80 81 4d ac 00 00 7a 79 e9 00 33 33 01 00 01 a3
alive           00 fe 10 02 80 8a e6
*/
