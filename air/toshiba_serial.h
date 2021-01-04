#define MASTER  0x00
#define REMOTE  0x40
#define BCAST   0xFE

//position of bytes in packet
#define FROM  0
#define TO    1
#define COUNT 3


#define MAX_RX_BUFFER 128//256 //maximum rx buffer size
#define MAX_CMD_BUFFER 32 //maximum message size

#define FAN_AUTO   2
#define FAN_HIGH   3
#define FAN_MEDIUM 4
#define FAN_LOW    5

#define MODE_COOL 2
#define MODE_FAN  3
#define MODE_AUTO 5
#define MODE_HEAT 1
#define MODE_DRY  4

#define TIMER_POWER_OFF 0
#define TIMER_POWER_ON  1


typedef struct {
  byte data[1];//MAX_RX_BUFFER];
  int idx_r=0;
  int idx_w=0;
  //idx_r is always BEFORE idx_w
  //idx_w
} rb_t;

typedef struct {
  uint8_t save;
  uint8_t heat;
  uint8_t cold;
  uint8_t target_temp;
  float sensor_temp;
  uint8_t fan;
  char fan_str[5];
  uint8_t mode;
  char mode_str[5];
  uint8_t preheat;
  uint8_t power;
  byte last_cmd[MAX_CMD_BUFFER];
  byte rx_data[MAX_RX_BUFFER];  //serial rx data
  //byte tx_data[MAX_CMD_BUFFER]; //serial tx data
  int curr_w_idx = 0;
  int curr_r_idx = 0;
  uint8_t timer_mode_req;
  uint8_t timer_time_req;
  bool timer_enabled;
  int decode_errors=0;
  rb_t rb;
  
  SoftwareSerial serial;
} air_status_t;
