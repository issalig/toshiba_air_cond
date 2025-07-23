#include "config.h"

// not really needed because we use WiFiManager
const char* w_ssid = "your_wifi";
const char* w_passwd = "your_password";

//Over the air (OTA) credentials
const char* OTAName = "air";
const char* OTAPassword = "esp8266";

// Domain name for the mDNS responder. Just connect to air.local
const char* mdnsName = "air";

const char compile_date[] = __DATE__ " " __TIME__;