# Web Interface

This document describes the features available through the embedded web interface served by the ESP8266 air conditioning controller.

## Access
- URL via mDNS: `http://air.local` (See `mdnsName` in `config.h`)
- HTTP Port: 80
- WebSocket Port: 81

| ![Main screen](images/air_main.png)

## Main Features
1. Control Functions
   - Power ON / OFF
   - Set target temperature (bounded 16–30 °C; internally enforced 18–30 in MQTT handler)
   - Change mode: `cool`, `heat`, `dry`, `fan_only`, `auto`, `off`
   - Change fan speed: `auto`, `low`, `medium`, `high`
   - Save mode

2. Timer
   - ON/OFF Timer. Software based relying only in esp8266

3. Chart
   - Shows current temperature, external temperature and pressure/humidity if sensors are available.

4. System
   - Info: 
     - IP, boot time, memory usage.
     - **Serial Statistics**: Detailed breakdown of decoded messages, discarded bytes/messages, and *recovered messages* (packets fixed by CRC repair).
     - **Announce State**: Current link status with the AC unit (UNLINKED, BUSY, LINKED).
   - Sensors: Internal / External AC sensors.
   - Address: 
     - **Sync**: Automatically detect and adopt addresses from observed traffic.
     - **Manual Set**: Configure specific Master/Remote addresses.
     - **Reset**: Restore default addresses (Master: 0x00, Remote: 0x40).
   - Modes: 
     - **Autonomous Mode**: Control loop to act as a remote (every 8s). Can be Start/Stopped.
     - **Simulation Mode**: Simulates a physical AC for testing UI/Integrations without hardware.
     - **Listen Mode**: Prioritizes serial RX (passive monitoring).
   - Settings:
     - **Persist**: Save/Load current configuration (Pins, etc.) to LittleFS.
     - **Hardware Config**: Runtime configuration of Serial (RX/TX/Buf) and I2C pins.
   - Admin: Upload files. Use it to upload `index.html` and others.
5. Debug
    - **Send RAW**: Send hex strings directly to the AC, i.e., "00 FE 10 02 80 8A E6".
    - **Explore Sensors**: Query arbitrary sensor IDs (0x00-0xFF) to discover undocumented values.
    - **Explore DN Codes**: Query configuration (DN) codes from the AC unit.
    - **Traffic Monitor**: Real-time view of `last_cmd` (sent) and `rx_data` (received), including recovered packets marker `<REC>`.
    - Log: System event log.

6. MQTT Configuration (if `USE_MQTT`)
   - **Runtime Config**: Modify host, port, user, password, device name without recompiling.
   - **Status**: View current connection status and test connection.
   - **Discovery**: Send/Reset Home Assistant discovery messages.
   - Persisted to `/mqtt_config.json` in LittleFS.

## Techincal details

1. File Management / Content Update
   - `upload.html` page (served directly from program memory via `upload_html.h`) allows file upload to LittleFS when there are no files.
   - Success redirect to `success.html`

2. Data & Persistence
   - Current status periodically serialized to `/status.json`
   - Hardware & Runtime settings persisted to `/settings.json`
   - Historical circular buffers in RAM for temperatures, humidity, pressure, timestamps (not persisted by default)
   - Remote access to raw files if exact path is known (no directory listing provided)

3. WebSocket Live Updates
   - Bi‑directional JSON messaging (see `processRequest()` logic)
   - Push notifications on state changes (e.g. after MQTT command processed, `notifyWebSocketClients()`)
   - Real‑time reflection of simulation adjustments and sensor readings

4. OTA & Maintenance (if `USE_OTA`)
   - Firmware updated via Arduino OTA (password in `config.h`)
   - Web assets updated through file upload (no firmware flashing required for UI tweaks)

## Message & Command Flow
| Action | Transport | Handler |
|--------|-----------|---------|
| UI control (temp/mode/fan/power) | WebSocket JSON | `processRequest()` → AC protocol / simulation
| Periodic sensor & status updates | Timers → WebSocket | `handleStatus()`, `handleTemperature()`
| MQTT incoming setpoints | MQTT subscribe topics | `mqttCallback()` → AC / simulation + WebSocket notify
| File upload | HTTP POST `/edit.html` or `/upload.html` | `handleFileUpload()`

## MQTT Topics (Summary)
Published (state):
- `homeassistant/ac/status/mode`
- `homeassistant/ac/status/fan_mode`
- `homeassistant/ac/status/temperature` (target)
- `homeassistant/ac/status/current_temperature`

Subscribed (commands):
- `homeassistant/ac/set/mode`
- `homeassistant/ac/set/fan_mode`
- `homeassistant/ac/set/temperature`

Home Assistant discovery config retained at:
- `homeassistant/climate/toshiba_ac/config`

## File System Notes
- Compression: Prefer uploading `.gz` versions of large assets (served automatically)
- Config files:
  - `/mqtt_config.json`
  - `/settings.json` (hardware & runtime config)
  - `/status.json` (periodic snapshot)
- Space is limited; remove unused large assets before OTA updates

## Security Considerations
- No HTTP authentication by default (add if exposing beyond LAN)
- OTA protected by password (see `config.h`)
- MQTT credentials stored in plaintext in LittleFS; restrict physical and network access

## Extending the UI
1. Add or modify HTML/JS/CSS in `data/` and upload (or create gzipped versions)
2. Add new WebSocket commands: implement in `processRequest.cpp` and update front‑end JS

## Build‑Time Feature Flags
| Flag | Effect |
|------|--------|
| `USE_MQTT` | Enables MQTT client, discovery & topics |
| `USE_SCREEN` | Enables OLED display updates |
| `USE_AHT20` / `USE_BMP280` | Enable corresponding sensors |
| `USE_OTA` | Activates OTA service |

## Quick Troubleshooting
| Symptom | Check |
|---------|-------|
| No web page | WiFi connection, mDNS name, LittleFS upload success |
| Stale data | WebSocket connection, browser console, `webSocket.loop()` activity |
| MQTT offline | Broker reachability, `/mqtt_config.json`, WiFi status |
| Commands ignored (simulation) | Ensure `simulation_mode` state matches expectations |
| OTA fails | Correct password, same network, firewall rules |

---
This file documents only the web interface & related runtime services. See the main `README.md` for full architectural details.
