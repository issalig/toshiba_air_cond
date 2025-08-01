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

#define DEBUG 1

/*____________________________UTILS_______________________________ */

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
    //Serial.print(data[i], HEX); Serial.print(" ");
  }
  return value;
}

int check_crc(const byte *data, size_t len) {
  uint8_t crc = 1, my_crc = 2;
  unsigned int packet_len;
  
  if (len > 4) { //minimal packet len
    //byte count is byte 3
    packet_len = data[3] + 5; //all packet with CRC
    if (len == packet_len) {
      crc    = data[packet_len - 1]; //CRC is last byte
      my_crc = XORChecksum8(data, packet_len - 1); //CRC does not include own CRC
#if 0//def DEBUG      
      Serial.println("");
      Serial.print("CRC ");
      for (int k = 0; k < packet_len; k++) {
        Serial.print(data[k], HEX);
        Serial.print(" ");
      }
      Serial.print("- ");
      Serial.print(my_crc, HEX);
      Serial.print("- ");
      Serial.print(crc, HEX);
#endif
    }
  }

  return (my_crc == crc);
}


void air_print_status(air_status_t *s) {
  Serial.print("\n");
  Serial.print(" Power: "); Serial.print(s->power);
  Serial.print(" Mode: "); Serial.print(s->mode_str);
  Serial.print(" Fan: "); Serial.print(s->fan_str);
  Serial.print(" SensorTemp: "); Serial.print(s->remote_sensor_temp);
  Serial.print(" Temp: "); Serial.print(s->target_temp);
  Serial.print(" Cold: "); Serial.print(s->cold);
  Serial.print(" Heat: "); Serial.print(s->heat);
  Serial.print(" Save: "); Serial.print(s->save);
  Serial.print(" Errors: "); Serial.print(s->decode_errors);
  //Serial.print(" INDOOR_ROOM:  "); Serial.print(s->indoor_room_temp);
  Serial.print(" INDOOR_TA: "); Serial.print(s->indoor_ta);
  Serial.print(" INDOOR_TCJ:"); Serial.print(s->indoor_tcj);
  Serial.print(" INDOOR_TC: "); Serial.print(s->indoor_tc);
  //Serial.print(" INDOOR_FILTER_TIME:  "); Serial.print(s->indoor_filter_time);
  Serial.print(" OUTDOOR_TE:"); Serial.print(s->outdoor_te);
  Serial.print(" OUTDOOR_TO:"); Serial.print(s->outdoor_to);
  //Serial.print(" OUTDOOR_TD:   "); Serial.print(s->outdoor_td);
  //Serial.print(" OUTDOOR_TS:   "); Serial.print(s->outdoor_ts);
  //Serial.print(" OUTDOOR_THS:  "); Serial.print(s->outdoor_ths);
  Serial.print(" OUTDOOR_CURRENT:  "); Serial.print(s->outdoor_current);
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
  //             00      01      02    03    04    05    06    CRC
  byte data[] = {air->remote, air->master, 0x11, 0x03, 0x08, 0x41, 0x03, 0x18};
  air_send_data(air, data, sizeof(data));
}

void air_set_power_off(air_status_t *air)  {
  //             00      01      02    03    04    05    06    CRC
  byte data[] = {air->remote, air->master, 0x11, 0x03, 0x08, 0x41, 0x02, 0x19};
  air_send_data(air, data, sizeof(data));
}

void air_set_save_off(air_status_t *air) {
  //bit0 from 7th in remote message
  //             00      01      02    03    04    05    06    07    CRC
  byte data[] = {air->remote, air->master, 0x11, 0x04, 0x08, 0x54, 0x01, 0x00, 0x08};
  air_send_data(air, data, sizeof(data));
}

void air_set_save_on(air_status_t *air) {
  //bit0 from 7th in remote message
  //             00      01      02    03    04    05    06    07    CRC
  byte data[] = {air->remote, air->master, 0x11, 0x04, 0x08, 0x54, 0x01, 0x01, 0x09};
  air_send_data(air, data, sizeof(data));
}

void air_set_temp(air_status_t *air, uint8_t target_temp)  {
  //       byte  00      01      02    03    04    05    06    07    08    09    10    11    CRC
  byte data[] = {air->remote, air->master, 0x11, 0x08, 0x08, 0x4C, 0x0C, 0x1D, 0x7A, 0x00, 0x33, 0x33, 0x76};

  //set mode   0C is byte for dry 1100  -> 100, 0A is cool 1010 ->  10
  data[6] = air->mode | 0b1000;

  //get status
  data[7] = air->fan | 0b11000; //FAN set bit3 to 1 and bit4 to 1

  //compose new message
  data[8] = ((target_temp) + 35) << 1; //temp is bit7-bit1
  data[10] = data[11] = 0x01 + 0x04 * air->heat + 0x02 * air->cold;

  //compose new message
  //compute crc
  data[12] = XORChecksum8(data, sizeof(data) - 1);

  air_send_data(air, data, sizeof(data));
}

void air_set_mode(air_status_t *air, uint8_t value)  {
  //From remote (Mode is bit3-bit0 from last data b yte) cool:010 fan:011 auto 101 heat:001 dry: 100
  //             00      01      02    03    04    05    06    CRC
  byte data[] = {air->remote, air->master, 0x11, 0x03, 0x08, 0x42, 0x04, 0x1C}; // dry
  //40 00 11 03 08 42 01 19 //heat
  //set mode
  data[6] = value;

  //compose new message
  //compute crc
  data[7] = XORChecksum8(data, sizeof(data) - 1);

  air_send_data(air, data, sizeof(data));
}

// To check this, specially if we have no information, for example cannot read from RX

void air_set_fan(air_status_t *air, uint8_t value)  {
  //Master 7th byte bit4=1 bit3-bit1  auto:0x010 med:011 high:110 low:101
  //             00      01      02    03    04    05    06    07    08    09    10    11    CRC
  byte data[] = {air->remote, air->master, 0x11, 0x08, 0x08, 0x4C, 0x13, 0x1D, 0x7A, 0x00, 0x33, 0x33, 0x6E}; //LOW, FAN

  //set mode
  data[6] = 0x10 + air->mode;

  //set fan level
  data[7] = 0b11000 + value;

  //set 8 current temp  (not sure if we should fill this or leave 7A)
  data[8] = ((air->target_temp) + 35) << 1; //temp is bit7-bit1

  //set 10, 11
  data[10] = data[11] = air->heat ? 0x55 : 0x33;

  //compose new message
  //compute crc
  data[12] = XORChecksum8(data, sizeof(data) - 1);

  air_send_data(air, data, sizeof(data));
}

//send ping
void air_send_ping(air_status_t *air)  {
  //             00      01      02    03    04    05    06    07    08    09    10    CRC
  byte data[] = {air->remote, air->master, 0x15, 0x07, 0x08, 0x0c, 0x81, 0x00, 0x00, 0x48, 0x00, 0x9f};
  air_send_data(air, data, sizeof(data));
}

void air_send_remote_temp(air_status_t *air, uint8_t value)  {
  //             00      01      02    03    04    05    06    07    08    CRC
  byte data[] = {air->remote, air->master, 0x55, 0x05, 0x08, 0x81, 0x00, 0x7C, 0x00, 0xE5}; //dummy msg
  //data[2]==0x55 data[5]==0x81   opcodes for this message
  //sensor temp
  data[7] = int((value) + 35) << 1;
  //compute crc
  data[9] = XORChecksum8(data, sizeof(data) - 1);

  air_send_data(air, data, sizeof(data));
}

//does not work correctly, maybe wall unit sets some internal register
void air_set_timer(air_status_t *air, uint8_t timer_mode, uint8_t timer_value)  {
  //             00      01      02    03    04    05    06    07    08    09    10    11    12    CRC
  byte data[] = {air->remote, air->master, 0x11, 0x09, 0x08, 0x0c, 0x82, 0x00, 0x00, 0x30, 0x07, 0x02, 0x02, 0xe9};
  //                                                                                     |----- number of 30 minutes
  //                                                                               |----- repeated
  //                                                                         |------ 07 poweron,   06 poweroff repeat, 05 poweroff,  00 cancel

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

  air->serial.begin(2400, SWSERIAL_8E1, D7, D8, false);//, 256, 11*16);
  //begin(uint32_t baud, SoftwareSerialConfig config, int8_t rxPin, int8_t txPin, bool invert, int bufCapacity = 64, int isrBufCapacity = 0);

  // high speed half duplex, turn off interrupts during tx
  air->serial.enableIntTx(false);

}

void air_decode_command(byte * data, air_status_t *s) {

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


    Extended status
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19
    00 FE 58 0F 80 81 8D A8 00 00 7A 84 E9 00 33 33 01 00 01 9B
                            |     |  |  |-always E9
                            |     |  |-  1000 0100  1000010 66-35=31 (real temp??)
                            |     |- temp 0111 1010 111101 61-35 = 26
                            |- bit 7 "filter alert", bit 1 "preheat"          
  */

  // get master and remote address from observed data
  // master is supposed to have address [0x00, 0x0F]
  // remote is supposed to have address [0x40, 0x4F]
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

  s->last_cmd_type = MSG_UNK;

  //status
  if (data[5] == 0x81) {
    if (data[2] == 0x1C) { //status data[5] == 0x81 and data[2] == 0x1C
      s->last_cmd_type = MSG_STATUS;
      s->save = data[14] & 0b1;
      s->heat = (data[13] & 0b00000100) >> 2;
      s->cold = !s->heat;
      s->preheat = (data[8] & 0b00000010) >> 1;
      s->filter_alert = (data[8] & 0b10000000) >> 7;  // Extract bit 7 of byte 8

      s->fan  = (data[7] & 0b11100000) >> 5;
      switch (s->fan) {
        case FAN_AUTO:   strcpy(s->fan_str, "AUTO"); break;
        case FAN_MEDIUM: strcpy(s->fan_str, "MED"); break;
        case FAN_HIGH:   strcpy(s->fan_str, "HIGH"); break;
        case FAN_LOW:    strcpy(s->fan_str, "LOW"); break;
        default: strcpy(s->fan_str, "UNK");
      }
      s->mode = (data[6] & 0b11100000) >> 5;
      switch (s->mode) {
        case MODE_COOL: strcpy(s->mode_str, "COOL"); break;
        case MODE_FAN:  strcpy(s->mode_str, "FAN"); break;
        case MODE_AUTO: strcpy(s->mode_str, "AUTO"); break;
        case MODE_HEAT: strcpy(s->mode_str, "HEAT"); break;
        case MODE_DRY:  strcpy(s->mode_str, "DRY"); break;
        default: strcpy(s->mode_str, "UNK");
      }
      s->target_temp = ((data[10] & 0b11111110) >> 1) - 35;
      s->power = data[6] & 0b1;

      //extended status data[5] == 0x81 and data[2] == 0x58
    } else if (data[2] == 0x58) {
      s->last_cmd_type = MSG_STATUS_EXT;
      /*
         0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19
        00 FE 58 0F 80 81 8D A8 00 00 7A 84 E9 00 33 33 01 00 01 9B
                                            |-always E9
                                         |-  1000 0100  1000010 66-35=31 (real temp??)
                                      |-target temp 0111 1010 111101 61-35 = 26
                                |- bit 7 "filter alert", bit 1 "preheat"
      */
      s->save = data[14 + 2] & 0b1;
      s->heat = (data[13 + 2] & 0b100) >> 2;
      s->cold = !s->heat;
      s->fan  = (data[7] & 0b11100000) >> 5;
      switch (s->fan) {
        case FAN_AUTO:   strcpy(s->fan_str, "AUTO"); break;
        case FAN_MEDIUM: strcpy(s->fan_str, "MED"); break;
        case FAN_HIGH:   strcpy(s->fan_str, "HIGH"); break;
        case FAN_LOW:    strcpy(s->fan_str, "LOW"); break;
        default:         strcpy(s->fan_str, "UNK");
      }

      s->mode = (data[6] & 0b11100000) >> 5;
      switch (s->mode) {
        case MODE_COOL: strcpy(s->mode_str, "COOL"); break;
        case MODE_FAN:  strcpy(s->mode_str, "FAN"); break;
        case MODE_AUTO: strcpy(s->mode_str, "AUTO"); break;
        case MODE_HEAT: strcpy(s->mode_str, "HEAT"); break;
        case MODE_DRY:  strcpy(s->mode_str, "DRY"); break;
        default:        strcpy(s->mode_str, "UNK");
      }

      s->target_temp = ((data[10] & 0b11111110) >> 1) - 35;
      s->power = data[6] & 0b1;

      s->preheat = (data[8] & 0b00000010) >> 1;
      s->filter_alert = (data[8] & 0b10000000) >> 7;  // Extract bit 7 of byte 8

      //WARNING: not sure
      //s->sensor_temp = (data[11] / 2.0 - 35); //TO CHECK

    }  else if (data[2] == 0x55) {
      //data[2]==0x55 data[5]==0x81
      s->remote_sensor_temp = data[7] / 2.0 - 35;
    }
  } else if (data[5] == 0x8A) {
    // alive message
    //00 FE 10 02 80 8A E6
    s->last_cmd_type = MSG_MASTER_ALIVE;
  } else if (data[5] == 0xA1) {
    // ACK after setting param
    //00 40 18 02 80 A1 7B
    s->last_cmd_type = MSG_MASTER_ACK;
  } else if (data[5] == 0x86) {
    // mode status
    //00 52 11 04 80 86 84 05 C0

    s->last_cmd_type = MSG_STATUS_MODE;
    s->mode = data[6] >> 5; //byte6 bit7-bit5

    //s->power=data[7] & 1; //byte7 bit0
    //(data[7] >> 2) &0b1 //bit2??

    //00 52 11 04 80 86 84 05 C0   dry       1000 >> 1 = 100 -> MODE_DRY;   101
    //00 52 11 04 80 86 44 05 00   cool      0100                       ;   101
    //00 52 11 04 80 86 64 01 24   fan       0110                       ;   001
    //00 52 11 04 80 86 44 01 04   cool auto 0100                       ;   001
    //00 52 11 04 80 86 24 05 60   heat low  0010
    //00 52 11 04 80 86 24 00 65   heat low  0010  OFF
    //-not related to air flow


    //00 52 11 04 80 86 84 01 C4   DRY,LOW      0001
    //00 52 11 04 80 86 64 01 24   FAN,LOW/HIGH/MED
    //00 52 11 04 80 86 44 01 04   COOL,LOW     0001       101
    //00 52 11 04 80 86 44 05 00   COOL,MED     0101   ->  100

    //00 52 11 04 80 86 24 00 65   heat low off
    //00 52 11 04 80 86 24 01 64   heat low on
    //00 52 11 04 80 86 24 05 60   heat low on
  } else if (data[5] == 0x55) {
  }

  //sensor reading
  if (data[2] == 0x1A) {
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
    print_logf("Read sensor id: 0x%x val: %d\n", s->sensor_id, s->sensor_val);
  }

  if (data[2] == 0x18) {
    s->last_cmd_type = MSG_SENSOR_ERROR;
    // answer 00 40 18 05 80 27 08 00 48 ba
    //                                |---------Type 0x4 Num error 0x8   E-08
    s->error_type = data[8] >> 4;
    s->error_val = data[8] & 0b00001111;
  }
}

//reads serial and gets a valid command in air.rx_data
//calls decode command to fill air structure
//returns true if one or command commands are decoded, false otherwise
int air_parse_serial(air_status_t *air) {
  int i, j_init, k;
  uint8_t mylen = 0;
  byte ch;
  byte cmd[MAX_CMD_BUFFER];
  int i_start, segment_len;
  bool found = false;
  bool rbuffer = false;
  int retval = false;

  //circular buffer avoids loosing parts of messages but it is not working so rbuffer=false to avoid resets
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
    //Serial.print(ch < 0x10 ? "__ 0" : "__ ");
    //Serial.print(ch, HEX);
    air->rx_data[i] = ch;
    //air->rx2_data[air->rx_data_count] = ch;
    //air->rx_data_count=air->rx_data_count+1;
    i = (i + 1) % MAX_RX_BUFFER;
    air->buffer_rx += ((ch < 0x10) ? "0" : "")  + String(ch, HEX) + " ";
  }

  if (i_start < i) Serial.print("[RCV] "); //print_log("[RCV] ");
  for (k = i_start; k < i; k++) {
    //print_log(air->rx_data[k] < 0x10 ? " 0" : " ");
    //print_logf("%02X", air->rx_data[k]);
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
        air->buffer_cmd += "<br>";

        Serial.println();//print_logf("\n");
        if (mylen > 4) { //min valid package is 5 bytes long with 0 data bytes
          //if (cmd[5] == 0x81) { //decode only 0x81 -> status
          air_decode_command(cmd, air);
          retval = true;
          //air_print_status(air);
          //}
        }
        i_start = (j_init + segment_len) % MAX_RX_BUFFER;
        j_init = (i_start - 1 + MAX_RX_BUFFER) % MAX_RX_BUFFER;
        //j_end = j_init;
        found = true;
      } //end if crc
    } //end if segment_len

    if (!found) {
#ifdef DEBUG
      Serial.println(""); Serial.print("[SKIP] ");
      Serial.print("("); Serial.print(j_init); Serial.print(")");
      Serial.println(air->rx_data[j_init % MAX_RX_BUFFER], HEX);
#endif
      i_start = j_init;
      //Serial.printf("NOT i_start %d, j_init %d, j_end %d\n", i_start, j_init, j_end);
      air->decode_errors = air->decode_errors + 1;
    }
    found = false;

  } //end for j_init

  //Serial.printf("END i_start %d, j_init %d, j_end %d\n", i_start, j_init, j_end);
  //circular buffer avoids loosing parts of messages but it is not working
  air->curr_w_idx = i;
  air->curr_r_idx = i_start;

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
