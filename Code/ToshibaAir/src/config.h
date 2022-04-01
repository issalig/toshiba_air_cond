#ifndef CONFIG
#define CONFIG

#define MAX_LOG_DATA 72 //store up to 72 readings in a circular buffer

#define USE_BMP //if BMP sensor installed
#define USE_DHT //in DHT sensor installed

//#define USE_ASYNC

//wifi credentials, no needed because now we use WiFiManager
const char *w_ssid = "YOURWIFI";
const char *w_passwd = "YOURPASSPWD";

//Over the air credentials
const char *OTAName = "air";           // A name and a password for the OTA service
const char *OTAPassword = "esp8266";

const char* mdnsName = "air"; // Domain name for the mDNS responder. Just connect to air.local

#endif
