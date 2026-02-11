/*
GNU GENERAL PUBLIC LICENSE

Version 2, June 1991

Copyright (C) 1989, 1991 Free Software Foundation, Inc.  
51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

Everyone is permitted to copy and distribute verbatim copies
of this license document, but changing it is not allowed.

*/

#include "display.h"

#ifdef USE_SCREEN

#include "print_log.h"

// Global display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// External variables from main file
extern air_status_t air_status;
extern float sensor_temperature[];
extern float sensor_humidity[];
extern float sensor_pressure[];
extern int temp_idx;
extern bool autonomous_mode;

#ifdef USE_MQTT
extern bool mqtt_enabled;
extern bool getMQTTStatus();
#endif

// 48x48px AC icon bitmap
const unsigned char ac_icon_bitmap[] PROGMEM = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfe, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x3f, 0xff, 
0xff, 0xff, 0xff, 0xc0, 0x07, 0xff, 0xff, 0xff, 0xff, 0x0c, 0xe1, 0xff, 0x80, 0x0f, 0xfe, 0x60, 
0x78, 0xff, 0x80, 0x0f, 0xfc, 0xc0, 0x3e, 0x7f, 0x80, 0x0f, 0xf9, 0x80, 0x3f, 0x3f, 0x80, 0x0f, 
0xf3, 0x80, 0x7f, 0x9f, 0x80, 0x0f, 0xe7, 0x80, 0x7f, 0x9f, 0x80, 0x0f, 0xe7, 0xc0, 0x7f, 0xcf, 
0x80, 0x0f, 0xef, 0xc1, 0xff, 0xcf, 0x80, 0x0f, 0xcf, 0xe4, 0x7f, 0xef, 0x80, 0x0f, 0xcf, 0xf8, 
0x3f, 0xe7, 0x80, 0x0f, 0xcf, 0xf0, 0x31, 0xe7, 0x80, 0x0f, 0xcf, 0xf0, 0x00, 0x67, 0x80, 0x0f, 
0xcf, 0xf0, 0x20, 0x27, 0x80, 0x0f, 0xcc, 0x08, 0x20, 0x27, 0x80, 0x0f, 0xc8, 0x04, 0xc0, 0x2f, 
0x80, 0x0f, 0xe8, 0x03, 0xc0, 0x4f, 0x80, 0x0f, 0xe0, 0x03, 0xe0, 0x4f, 0x80, 0x0f, 0xe4, 0x03, 
0xe0, 0x1f, 0x80, 0x0f, 0xf2, 0x07, 0xf0, 0x9f, 0x80, 0x0f, 0xf9, 0x07, 0xf9, 0x3f, 0x80, 0x0f, 
0xfc, 0x8f, 0xfe, 0x7f, 0x80, 0x0f, 0xfe, 0x7f, 0xf8, 0xff, 0x80, 0x0f, 0xff, 0x0f, 0xe1, 0xff, 
0x80, 0x0f, 0xff, 0x80, 0x07, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfe, 
0x3f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x01, 0xf0, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void startDisplay() {
  // Clear the buffer
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    print_log(F("SSD1306 allocation failed"));
  else {
    showSplashScreen();
  }
}

void showSplashScreen() {
  display.clearDisplay();

  // Draw the 48x48 bitmap centered on screen
  // 128 - 48 = 80/2 = 40 for proper horizontal centering
  display.drawBitmap(40, 0, ac_icon_bitmap, 48, 48, SSD1306_WHITE);
  
  // Add title text below or overlay
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(34, 50);
  display.println(F("Starting..."));
  
  display.display();
  delay(2000); // Show splash screen for 2 seconds
}

void showIPDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Connected!"));
  
  display.setCursor(0, 20);
  display.println(F("IP Address:"));
  display.setCursor(0, 35);
  display.println(air_status.ip);
  
  display.setCursor(0, 50);
  display.println(F("Ready to control"));
  
  display.display();
  delay(2000); // Show IP for 2 seconds before normal operation
}

void showDisplay(void) {
  static unsigned long lastDisplayUpdate = 0;
  static unsigned long lastInfoRotation = 0;
  static int infoIndex = 0;
  const unsigned long INFO_ROTATION_INTERVAL = 3000; // 3 seconds per info

  if (millis() - lastDisplayUpdate < 1000) return; // Limit display updates
  lastDisplayUpdate = millis();

  // Rotate information every 3 seconds
  if (millis() - lastInfoRotation > INFO_ROTATION_INTERVAL) {
    infoIndex = (infoIndex + 1) % 4; // 4 different info screens
    lastInfoRotation = millis();
  }

  //Turn off display when AC is OFF
  int show_always = 1; // 0 - show only when AC is ON, 1 - always show
  if (show_always == 0 && air_status.power == 0) {
    display.clearDisplay();
    display.display();
    return;
  }

  display.clearDisplay();

  // Top section - AC Status (lines 0-31)
  if (air_status.power != 0) {
    // Large target temperature
    display.setTextSize(3);
    display.setCursor(3, 3);
    display.setTextColor(SSD1306_WHITE);
    display.printf("%d", air_status.target_temp);
    
    // Mode indicator
    display.setTextSize(2);
    display.setCursor(60, 0);
    display.printf("%s", air_status.mode_str);
    
    // Fan speed
    display.setTextSize(1);
    display.setCursor(60, 16);
    display.printf("%s", air_status.fan_str);
    
    // Current AC sensor temperature
    display.setCursor(90, 16);
    display.printf("%.1f", air_status.remote_sensor_temp);
  } 

  // Separator line
  display.drawLine(0, 31, 127, 31, SSD1306_WHITE);

  // Middle section - Environmental data (lines 32-50)
  int idx = (temp_idx > 0) ? temp_idx - 1 : 0;

  // Room temperature and humidity from centralized arrays
  display.setTextSize(1);
  display.setCursor(0, 34);
  if (sensor_temperature[idx] > -999 && sensor_humidity[idx] > -999) {
    display.printf("Room: %.1f C %.0f%%", sensor_temperature[idx], sensor_humidity[idx]);
  } else if (sensor_temperature[idx] > -999) {
    display.printf("Room: %.1f C --%% ", sensor_temperature[idx]);
  } else {
    display.printf("Room: -- C --%% (No sensor)");
  }

  // Outdoor temperature
  display.setTextSize(1);
  display.setCursor(0, 44);
  if (air_status.outdoor_te > -999) {
    display.printf("Out: %d C", air_status.outdoor_te);
  } else {
    display.printf("Out: -- C");
  }

  // Pressure from centralized array
  display.setCursor(70, 44);
  if (sensor_pressure[idx] > -999) {
    display.printf("%.0fmb", sensor_pressure[idx]);
  } else {
    display.printf("--mb");
  }

  // Bottom rotating information line (line 57)
  display.setTextSize(1);
  display.setCursor(0, 57);
  
  switch (infoIndex) {
    case 0: // Connection status
      if (WiFi.status() == WL_CONNECTED) {
        display.print("WiFi");
      } else {
        display.print("No WiFi");
      }
      
      #ifdef USE_MQTT
      if (getMQTTStatus()) {
        display.print(" MQTT");
      } else {
        display.print(" No MQTT");
      }
      #endif
      break;
      
    case 1: 
      if (!autonomous_mode) {
        //Indoor sensors, ta, tcj, tc
        //outdoor sensors, te, to, td, ts, ths
        display.printf("TA: %d C TCJ: %d C TC: %d C",
                     air_status.indoor_ta, air_status.indoor_tcj, air_status.indoor_tc);
      }
      // Show outdoor temperatures
      else {
        display.printf("TE: %d C TO: %d C TD: %d C TS: %d C THS: %d C",
                     air_status.outdoor_te, air_status.outdoor_to,
                     air_status.outdoor_td, air_status.outdoor_ts,
                     air_status.outdoor_ths);
        }
      break;
      
    case 2: // Timer info or IP
      if (air_status.timer_enabled) {
        display.printf("Timer %d %s", air_status.timer_time_req, 
                       air_status.timer_mode_req == TIMER_SW_OFF ? "OFF" : "ON");
      } else {
        display.print("IP: ");
        display.print(air_status.ip);
      }
      break;
      
    case 3: // Autonomous mode or outdoor current
      if (autonomous_mode) {
        display.printf("Master %x02 Remote %x02", air_status.master, air_status.remote);
      } else {
        // Show current and RPM
        display.printf("Current: %d A RPM: %d", air_status.outdoor_current, air_status.indoor_fan_speed);
      }
      break;
  }

  display.display();
  yield();
}

#endif // USE_SCREEN
