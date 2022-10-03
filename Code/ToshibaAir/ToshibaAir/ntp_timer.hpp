#pragma once
#include <Arduino.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "config.h"

namespace NTPTimer {
// Define NTP Client to get time
WiFiUDP ntpUDP;
const long utcOffsetInSeconds = 3600; // 1 hour for Europe/Brussels
int timeOffset = 0;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 0); // do not apply utc here

long int boot_time;

void initialize(unsigned long &as_btime) {

  timeClient.begin();          // Get NTP time
  //timeClient.update();
  for (int i = 0; i < 5; i++) {
    timeClient.update();                        // update time
    boot_time = timeClient.getEpochTime();
    if (boot_time > 3600) break;                  // check we are not still in 1970
  }
  as_btime = boot_time;
}


} // namespace NTPTimer
