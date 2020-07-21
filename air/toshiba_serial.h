#define MASTER  0x00
#define REMOTE  0x40
#define BCAST   0xFE

//position of bytes in packet
#define BFROM  0
#define BTO    1
#define BCOUNT 3


#define MAX_RX_BUFFER 256
#define MAX_CMD_BUFFER 32

#define FAN_AUTO   2
#define FAN_HIGH   3
#define FAN_MEDIUM 4
#define FAN_LOW    5

#define MODE_COOL 2
#define MODE_FAN  3
#define MODE_AUTO 5
#define MODE_HEAT 1
#define MODE_DRY  4

typedef struct {
  uint8_t save;
  uint8_t heat;
  uint8_t cold;
  uint8_t temp;
  float sensor_temp;
  uint8_t fan;
  char fan_str[5];
  uint8_t mode;
  char mode_str[5];
  uint8_t power;
  byte last_cmd[MAX_CMD_BUFFER];
  byte rx_data[MAX_RX_BUFFER];  //serial rx data
  int curr_w_idx = 0;
  int curr_r_idx = 0;
  bool timer_mode_req;
  uint8_t timer_time_req;
    
  SoftwareSerial serial;
} air_status_t;
