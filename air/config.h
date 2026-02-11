#ifndef CONFIG_H
#define CONFIG_H

#define AIR_VERSION "2.1"

//pins for wemos d1 mini
//D1: GPIO5/SCL,   D2: GPIO4/SDA,   D3: GPIO0/FLASH, D4: GPIO2, 
//D5: GPIO14/SCLK, D6: GPIO12/MISO, D7: GPIO13/MOSI, D8: GPIO15/CS

// enter into wifi configuration mode
#define RESET_MODE_PIN 2  //D4 GPIO2

//tx rx pins for software serial to communicate with AC unit
#define TX_PIN 15  //D8 GPIO15
#define RX_PIN 13  //D7 GPIO13

//i2c pins
#define I2C_SDA_PIN 4 //D2 GPIO4
#define I2C_SCL_PIN 5 //D1 GPIO5

#define MAX_LOG_DATA 72 //store up to 72 readings in a circular buffer

//#define USE_ASYNC // Use AsyncWebServer
#define USE_OTA
#define USE_SCREEN //if OLED screen installed
#define USE_MQTT // for Home Assistant integration

#define USE_AHT20     // Enable AHT20 temperature & humidity sensor
#define USE_BMP280    // Enable BMP280 temperature & pressure sensor (replaces BMP085)
#define USE_BME280    // Enable BME280 temperature, humidity & pressure sensor

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


//#define USE_TELEGRAM

#ifdef USE_TELEGRAM
// Telegram Bot configuration - Replace with your values
extern const char* telegram_bot_token;
extern const char* telegram_chat_id;
#endif


#endif //CONFIG_H
