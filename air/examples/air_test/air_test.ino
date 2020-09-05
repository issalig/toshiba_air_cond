// simple program to test serial communication
// connect D7 to D8 or change pins inside init_air_serial()

#include "toshiba_serial.hpp"
air_status_t air_status;

void setup() {
  Serial.begin(115200);
  Serial.println("READ AB Bus");
  init_air_serial(&air_status);
}

void loop() {
  air_send_test_data_partial(&air_status);
  air_parse_serial(&air_status);
  air_send_test_data_partial2(&air_status);
  air_parse_serial(&air_status);
  //air_status.serial.flush();
  delay(500);
}
