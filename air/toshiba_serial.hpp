#include "SoftwareSerial.h" //https://github.com/plerup/espsoftwareserial
#include "toshiba_serial.h"

#define DEBUG 1

void air_send_data(air_status_t *air, byte *data, int len);
uint8_t XORChecksum8(const byte *data, size_t len);
void air_print_status(air_status_t *s);

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
  int packet_len;
  int k;

  if (len > 4) { //minimal packet len
    //byte count is byte 3
    packet_len = data[3] + 5; //all packet with CRC
    if (len == packet_len) {
      crc    = data[packet_len - 1]; //CRC is last byte
      my_crc = XORChecksum8(data, packet_len - 1); //CRC does not include own CRC
#ifdef DEBUG
      Serial.println("");
      Serial.print("CRC ");
      for (k = 0; k < packet_len; k++) {
        Serial.print(data[k], HEX);
        Serial.print(" ");
      }
#endif
    }
  }

  return (my_crc == crc);
}


void air_print_status(air_status_t *s) {
  Serial.print(" Power: "); Serial.print(s->power);
  Serial.print(" Mode: "); Serial.print(s->mode_str);
  Serial.print(" Fan: "); Serial.print(s->fan_str);
  Serial.print(" SensorTemp: "); Serial.print(s->sensor_temp);
  Serial.print(" Temp: "); Serial.print(s->temp);
  Serial.print(" Cold: "); Serial.print(s->cold);
  Serial.print(" Heat: "); Serial.print(s->heat);
  Serial.print(" Save: "); Serial.print(s->save);
  Serial.println("");
}

/*
   Hard-coded byte streams
*/
int air_set_power_on(air_status_t *air) {
  byte data[] = {0x40, 0x00, 0x11, 0x03, 0x08, 0x41, 0x03, 0x18};
  air_send_data(air, data, sizeof(data));
}

int air_set_power_off(air_status_t *air)  {
  byte data[] = {0x40, 0x00, 0x11, 0x03, 0x08, 0x41, 0x02, 0x19};
  air_send_data(air, data, sizeof(data));
}

int air_set_mode_cool(air_status_t *air) {
  byte data[] = {0x40, 0x00, 0x11, 0x03, 0x08, 0x42, 0x02, 0x1A};
  air_send_data(air, data, sizeof(data));
}

int air_set_mode_dry(air_status_t *air) {
  byte data[] = {0x40, 0x00, 0x11, 0x03, 0x08, 0x42, 0x04, 0x1C};
  air_send_data(air, data, sizeof(data));
}

int air_set_save_off(air_status_t *air) {
  //bit0 from 7th in remote message
  byte data[] = {0x40, 0x00, 0x11, 0x04, 0x08, 0x54, 0x01, 0x00, 0x08};  
  air_send_data(air, data, sizeof(data));
}

int air_set_save_on(air_status_t *air) {
  //bit0 from 7th in remote message
  byte data[] = {0x40, 0x00, 0x11, 0x04, 0x08, 0x54, 0x01, 0x01, 0x09};  
  air_send_data(air, data, sizeof(data));
}


int air_set_temp_minus(air_status_t *air)  {
  //               00    01    02    03    04    05    06    07    08    09    10    11   CRC
  byte data[] = {0x40, 0x00, 0x11, 0x08, 0x08, 0x4C, 0x0C, 0x1D, 0x7A, 0x00, 0x33, 0x33, 0x76};
  byte heat, cold, temp, mode, fan;

  //get status
  //compose new message
#ifdef DEBUG
  Serial.println(""); Serial.print("Set temp "); Serial.print(air->temp - 1);  Serial.print(" ");  Serial.print(sizeof(data));
#endif
  data[8] = ((air->temp - 1) + 35) << 1; //temp is bit7-bit1
  data[10] = data[11] = air->heat ? 0x55 : 0x33;

  //compute crc
  data[12] = XORChecksum8(data, sizeof(data) - 1);

#ifdef DEBUG
  int k = 0;
  Serial.println(""); Serial.print("Send ");
  for (k = 0; k < sizeof(data); k++)
    Serial.print(data[k], HEX);
#endif

  air_send_data(air, data, sizeof(data));
}

int air_set_temp_plus(air_status_t *air)  {
  //               00    01    02    03    04    05    06    07    08    09    10    11   CRC
  byte data[] = {0x40, 0x00, 0x11, 0x08, 0x08, 0x4C, 0x0C, 0x1D, 0x7A, 0x00, 0x33, 0x33, 0x76}; // press temp up   (current 25, 26 after pressing)
  byte heat, cold, temp, mode, fan;

  //get status
  //compose new message
#ifdef DEBUG
  Serial.println(""); Serial.print("Set temp "); Serial.print(air->temp + 1);  Serial.print(" ");  Serial.print(sizeof(data));
#endif
  data[8] = ((air->temp + 1) + 35) << 1; //temp is bit7-bit1
  data[10] = data[11] = air->heat ? 0x55 : 0x33;

  //compute crc
  data[12] = XORChecksum8(data, sizeof(data) - 1);

#ifdef DEBUG
  int k = 0;
  Serial.println(""); Serial.print("Send ");
  for (k = 0; k < sizeof(data); k++)
    Serial.print(data[k], HEX);
#endif

//byte data2[] = {0x40, 0x00, 0x11, 0x08, 0x08, 0x4C, 0x0C, 0x1D, 0x78, 0x00, 0x33, 0x33, 0x74};

  air_send_data(air, data, sizeof(data));
}

int air_set_temp(air_status_t *air, uint8_t target_temp)  {
  //               00    01    02    03    04    05    06    07    08    09    10    11   CRC
  byte data[] = {0x40, 0x00, 0x11, 0x08, 0x08, 0x4C, 0x0C, 0x1D, 0x7A, 0x00, 0x33, 0x33, 0x76}; // press temp up   (current 25, 26 after pressing)
  byte heat, cold, temp, mode, fan;

  //get status
  data[7] = air->fan | 0b11000; //FAN set bit3 to 1 and bit4 to 1

  //compose new message
  data[8] = ((target_temp) + 35) << 1; //temp is bit7-bit1
  data[10] = data[11] = 0x01 + 0x04 * air->heat + 0x02 * air->cold;

  //compose new message
  //compute crc
  data[12] = XORChecksum8(data, sizeof(data) - 1);

#ifdef DEBUG
  int k = 0;
  Serial.println(""); Serial.print("Send ");
  for (k = 0; k < sizeof(data); k++) {
    Serial.printf("%02x:", data[k]);
  }
#endif

//40 00 11 08 08 4C 0C 1D 78 00 33 33 74 

air_send_data(air, data, sizeof(data));

}

int air_set_mode(air_status_t *air, uint8_t value)  {
//From remote (Mode is bit3-bit0 from last data byte) cool:010 fan:011 auto 101 heat:001 dry: 100
    //               00    01    02    03    04    05    06   CRC
  byte data[] = {0x40, 0x00, 0x11, 0x03, 0x08, 0x42, 0x04, 0x1C}; // dry
  
  //set mode
  data[6] = value; 

  //compose new message
  //compute crc
  data[7] = XORChecksum8(data, sizeof(data) - 1);

#ifdef DEBUG
  int k = 0;
  Serial.println(""); Serial.print("Send ");
  for (k = 0; k < sizeof(data); k++) {
    Serial.printf("%02x:", data[k]);
  }
#endif

air_send_data(air, data, sizeof(data));

}

// To check this, specially if we have no information, for example cannot read from RX

int air_set_fan(air_status_t *air, uint8_t value)  {
     //Master 7th byte bit4=1 bit3-bit1  auto:0x010 med:011 high:110 low:101 
    //             00    01    02    03    04    05    06    07    08    09    10    11   CRC
  byte data[] = {0x40, 0x00, 0x11, 0x08, 0x08, 0x4C, 0x14, 0x1D, 0x7A, 0x00, 0x33, 0x33, 0x6E}; //LOW

  //set mode
  data[7] = 0b11000 + value; 

  //set 8 current temp
  data[8] = ((air->temp) + 35) << 1; //temp is bit7-bit1

  //set other 10, 11
  data[10] = data[11] = air->heat ? 0x55 : 0x33;
  
  //compose new message
  //compute crc
  data[12] = XORChecksum8(data, sizeof(data) - 1);

#ifdef DEBUG
  int k = 0;
  Serial.println(""); Serial.print("Send ");
  for (k = 0; k < sizeof(data); k++) {
    Serial.printf("%02x:", data[k]);
  }
#endif

air_send_data(air, data, sizeof(data));

}

/*____________________________CORE_______________________________ */

/*
  air_parse_serial
  air_decode_command

*/



void init_air_serial(air_status_t *air) {
  air->serial.begin(2400, SWSERIAL_8E1, D7, D8, false, 256);
  //begin(uint32_t baud, SoftwareSerialConfig config,       int8_t rxPin, int8_t txPin, bool invert, int bufCapacity = 64, int isrBufCapacity = 0);

  // high speed half duplex, turn off interrupts during tx
  air->serial.enableIntTx(false);

}

void air_decode_command(byte * data, air_status_t *s) {
  if (data[0] == MASTER) {
    Serial.println("Master sends: ");
    //get status
  }
  else if (data[0] == REMOTE) {
    Serial.println("Remote sends: ");
    //get status
  }
  else {
    Serial.println("Unknown origin address: ");
    Serial.print(data[0], HEX);
  }
  /*
    Status
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17
    00 FE 1C 0D 80 81 8D AC 00 00 76 00 33 33 01 00 01 B9
    |  |     ||       |  |         |    |      |- save mode bit0
    |  |     ||       |  |         |    |  |- bit2 HEAT:1 COLD:0
    |  |-Dst |        |  |         |    |- bit2 HEAT:1 COLD:0
    |-Src    |        |  |         |- bit7..bit1  - 35 =Temp
             |        |  |-bit3..bit1 fan mode (auto:010 med:011 high:110 low:101 ) !!bit7-bit5
             |        |  |-bit2 ON:1 OFF:0
             |        |-bit7.bit5 (mode cool:010 fan:011 auto 101 heat:001 dry: 100)
             |        |-bit0 ON:1 OFF:0
             |-Byte count


    Extended status
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19
    00 FE 58 0F 80 81 8D A8 00 00 7A 84 E9 00 33 33 01 00 01 9B
                                        |-always E9
                                     |-  1000 0100  1000010 66-35=31 (real temp??)
                                   |-temp 0111 1010 111101 61-35 = 26
  */

  //status
  if (data[5] == 0x81) {
    if (data[2] == 0x1C) { //status data[5] == 0x81 and data[2] == 0x1C
      s->save = data[14] & 0b1;
      s->heat = (data[13] & 0b00000100) >> 2;
      s->cold = !s->heat;
      s->fan  = (data[7] & 0b11100000) >> 5;
      Serial.println(s->fan);
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
      s->temp = ((data[10] & 0b11111110) >> 1) - 35;
      s->power = data[6] & 0b1;

      //extended status data[5] == 0x81 and data[2] == 0x58
    } else if (data[2] == 0x58) {

      /*
        0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19
        00 FE 58 0F 80 81 8D A8 00 00 7A 84 E9 00 33 33 01 00 01 9B
                                          |-always E9
                                       |-  1000 0100  1000010 66-35=31 (real temp??)
                                    |-temp 0111 1010 111101 61-35 = 26
      */
      s->save = data[14 + 2] & 0b1;
      s->heat = (data[13 + 2] & 0b100) >> 2;
      s->cold = !s->heat;
      s->fan  = (data[7] & 0b11100000) >> 5;
      switch (s->fan) {
        case FAN_AUTO: strcpy(s->fan_str, "AUTO"); break;
        case FAN_MEDIUM: strcpy(s->fan_str, "MED"); break;
        case FAN_HIGH: strcpy(s->fan_str, "HIGH"); break;
        case FAN_LOW: strcpy(s->fan_str, "LOW"); break;
        default: strcpy(s->fan_str, "UNK");
      }

      s->mode = (data[6] & 0b11100000) >> 5;
      switch (s->mode) {
        case MODE_COOL: strcpy(s->mode_str, "COOL"); break;
        case MODE_FAN: strcpy(s->mode_str, "FAN"); break;
        case MODE_AUTO: strcpy(s->mode_str, "AUTO"); break;
        case MODE_HEAT: strcpy(s->mode_str, "HEAT"); break;
        case MODE_DRY: strcpy(s->mode_str, "DRY"); break;
        default: strcpy(s->mode_str, "UNK");
      }

      s->temp = ((data[10] & 0b11111110) >> 1) - 35;
      s->power = data[6] & 0b1;

      //s->sensor_temp = - (data[11] / 2.0 - 35); //TO CHECK
      
    }  else if (data[2] == 0x55) {
      //data[2]==0x55 data[5]==0x81
      s->sensor_temp = data[7] / 2.0 - 35;
    }
  } else if (data[5] == 0x8A) {

  } else if (data[5] == 0xA1) {

  } else if (data[5] == 0x86) {    
    s->mode=data[6] >> 5; //byte  6 bit7-bit5
    s->power=data[7] & 1; //byte7 bit0
    //bit2??

  } else if (data[5] == 0x55) {
  }

}

//reads serial and gets a valid command in air.rx_data
//calls decode command to fill air structure
void air_parse_serial(air_status_t *air) {
  int i, j_init, j_end, k;
  uint8_t mylen = 0;
  byte ch;
  byte cmd[32];
  int i_start, i_end, segment_len;
  bool found = false;

  //circular buffer avoids loosing parts of messages but it is not working
  //thus is disabled
  //i = air->curr_w_idx;
  //i_start = air->curr_r_idx;

  i = i_start = 0; //no circular buffer - with this some bytes will be lost but not a big problem

  SoftwareSerial *ss;
  ss = &(air->serial);

  //producer
  Serial.print("Receiving data ");
  while (ss->available()) {
    ch = (byte)ss->read();
    Serial.print(ch < 0x10 ? " 0" : " ");
    Serial.print(ch, HEX);
    air->rx_data[i] = ch;
    i = (i + 1) % MAX_RX_BUFFER;
  }

  //consumer
  Serial.println("");
  Serial.print("Parsing data ");
  //try all combinations
  for (j_init = i_start; j_init != i; j_init = (j_init + 1) % MAX_RX_BUFFER) {
    segment_len = air->rx_data[(j_init + 3) % MAX_RX_BUFFER] + 5; //packet should be
    if (segment_len <= 32) {
#ifdef DEBUG
      Serial.println("");
      Serial.print("Try: ");
      Serial.print("("); Serial.print(j_init); Serial.print(")");
#endif
      for (k = 0; (k < segment_len && k < 32); k++) {
        cmd[k] = air->rx_data[(j_init + k) % MAX_RX_BUFFER];
#ifdef DEBUG
        Serial.print(cmd[k] < 0x10 ? " 0" : " ");
        Serial.print(cmd[k], HEX);
#endif
      }

      if (check_crc(cmd, segment_len)) { //check crc for segment_len
        mylen = cmd[3] + 5;
        Serial.println("");
        Serial.print("Cmd: ");
        for (k = 0; k < mylen; k++) {
          Serial.print(cmd[k] < 0x10 ? " 0" : " ");
          Serial.print(cmd[k], HEX);
          air->last_cmd[k] = cmd[k];
        }
        //air->last_cmd[(mylen<32)?mylen:31]='\0';

        Serial.println("");
        //decode
        if (mylen > 4) {
          if (cmd[5] == 0x81) { //status
            air_decode_command(cmd, air);
            air_print_status(air);
          }
        }
        i_start = (j_init + segment_len) % MAX_RX_BUFFER;
        j_init = i_start - 1;
        j_end = j_init;
        Serial.print(air->rx_data[i_start], HEX);
        found = true;
      } //end if crc
    } //end if segment_len

    if (!found) {
      Serial.println(""); Serial.print("Skipping ");
      Serial.print(air->rx_data[j_init], HEX);
      i_start = j_init;
    }
    found = false;

  } //end for j_init

  //circular buffer avoids loosing parts of messages but it is not working
  //air->curr_w_idx = i;
  //air->curr_r_idx = i_start;
}


void air_send_data(air_status_t *air, byte *data, int len) {
  int i;

  SoftwareSerial *ss;
  ss = &(air->serial);

  ss->enableIntTx(true); //enable TX

#ifdef DEBUG
  Serial.println("");
  Serial.print("Sending data");
  Serial.printf("(%d)\n", len);
#endif
  for (i = 0; i < len; i++) {
    ss->write(data[i]);
#ifdef DEBUG
    Serial.print(data[i] < 0x10 ? " 0" : " ");
    Serial.print(data[i], HEX);
#endif
  }
#ifdef DEBUG
  Serial.println("");
#endif

  ss->enableIntTx(false); //disable TX

}

void air_send_test_data(air_status_t *air) {
  int i;

  SoftwareSerial *ss;
  ss = &(air->serial);

  const unsigned char testdata[] = {
    0x40, 0x00, 0x11, 0x03, 0x08, 0x42, 0x05, 0x1D,                                  //auto  05 -> 0000 0101
    0x00, 0x40, 0x18, 0x02, 0x80, 0xA1, 0x7B,
    0x00, 0xFE, 0x10, 0x02, 0x80, 0x8A, 0xE6,
    0x00, 0x40, 0x18, 0x02, 0x80, 0xA1, 0x7B,
    0x00,
    0x00, 0xFE, 0x1C, 0x0D, 0x80, 0x81, 0x8D, 0xAC, 0x00, 0x00, 0x76, 0x00, 0x33, 0x33, 0x01, 0x00, 0x01, 0xB9,
    0x00, 0xFE, 0x1C, 0x0D, 0x80, 0x81, 0xCD, 0x8C, 0x00, 0x00, 0x76, 0x00, 0x33, 0x33, 0x01, 0x00, 0x01, 0xD9,
    0x00 , 0xFE , 0x58 , 0x0F , 0x80 , 0x81 , 0x8C, 0xA8, 0x00, 0x00, 0x7A , 0x84, 0xE9, 0x00, 0x33, 0x33, 0x01 , 0x00, 0x01, 0x90, //9b is correct crc
    0x00 , 0xFE , 0x58 , 0x0F , 0x80 , 0x81 , 0x8D , 0xAC , 0x00 , 0x00 , 0x7A , 0x7D, 0xE9, 0x00, 0x33, 0x33, 0x01 , 0x00, 0x01, 0x67
  };

  ss->enableIntTx(true);

  Serial.println("");
  Serial.println("Sending data");
  for (i = 0; i < sizeof(testdata); i++) {
    ss->write(testdata[i]);
    Serial.print(testdata[i] < 0x10 ? " 0" : " ");
    Serial.print(testdata[i], HEX);
  }
  Serial.println("");

  ss->enableIntTx(false);

}
