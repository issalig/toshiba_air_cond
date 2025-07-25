#ifndef CONFIG_H
#define CONFIG_H

#define MAX_LOG_DATA 72 //store up to 72 readings in a circular buffer

#define USE_SCREEN //if OLED screen installed
#define USE_MQTT // for Home Assistant integration

#define USE_AHT20     // Enable AHT20 temperature & humidity sensor
#define USE_BMP280    // Enable BMP280 temperature & pressure sensor (replaces BMP085)

//wifi credentials, no needed because now we use WiFiManager
extern const char* w_ssid;
extern const char* w_passwd;

//Over the air (OTA) credentials
extern const char* OTAName;
extern const char* OTAPassword;

// Domain name for the mDNS responder. Just connect to air.local
extern const char* mdnsName;

// Compile date and time
extern const char compile_date[];

// screen
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64//32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C// See datasheet for address if you are using a different one

#endif //CONFIG_H
