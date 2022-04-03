#pragma once
#include <Arduino.h>

String formatBytes(size_t bytes) { // convert sizes in bytes to KB and MB
  String val;
  if (bytes < 1024) {
    val = String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    val = String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    val = String(bytes / 1024.0 / 1024.0) + "MB";
  }

  return val;
}

String getContentType(String filename) { // determine the filetype of a given
                                         // filename, based on the extension
  if (filename.endsWith(".html"))
    return "text/html";
  else if (filename.endsWith(".css"))
    return "text/css";
  else if (filename.endsWith(".js"))
    return "application/javascript";
  else if (filename.endsWith(".ico"))
    return "image/x-icon";
  else if (filename.endsWith(".gz"))
    return "application/x-gzip";
  return "text/plain";
}

float round_f(float var) { // round float to two decimal places
  float value = (int)(var * 100 + .5);
  return (float)value / 100;
}