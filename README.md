# toshiba_air_cond
This project implements functions to decode Toshiba AB protocol from indoor units to wired controllers and provides a hardware design for communication.

Gerbers are available but remember if you improve the design please share it, that's how open source works, if you do not want to share, this project is not for you.

I strongly recommend to use [makusets' board](https://github.com/makusets/esphome-toshiba-ab/tree/main/hardware/D1%20mini) instead of my custom hardware.
My hardware is designed assuming a voltage level of the AB line around 15.6V when "1" and 14V when "0". Different voltage values will not work for writing while reading will work.

In particular, this project has been tested with remote control unit RBC-AMT32E and central unit RAV-SM406BTP-E (http://www.toshiba-aircon.co.uk/assets/uploads/product_assets/20131115_IM_1115460101_Standard_Duct_RAV-SM_6BTP-E_EN.pdf)

You can find the service manual from central unit and wired controller here: http://www.toshibaclim.com/Portals/0/Documentation/Manuels%20produits/SM_Gainable_Std-Compact--DI_406566806110614061606_GB.pdf, https://rednux.com/mediafiles/Hersteller/toshiba/Toshiba-Bedienungsanleitung-RBC-AMT32E-Englisch.pdf

## Index

[Software installation](#Software-installation)

[Web Interface](#Web-Interface)

[Home Assistant & MQTT](#Home-Assistant-&-MQTT)

[Hardware Installation](#Hardware-Installation)

[Custom hardware](#Custom-hardware)

[Data format](#Data-format)

[Message types](#Message-types)

[Other info](#Other-info)
 
# Software installation
[Up](#toshiba_air_cond) [Previous](#Index) [Next](#Hardware-Installation)

Code is developed in PlatformIO for ESP8266 and in particular for Wemos D1 mini board. It is basically a WebServer that serves a webpage and communicates with the client by means of WebSockets. It also offers Home Assistant integration and has nice features usch as OTA updates, file uploading, WifiManager and others.

### Dependencies

This project uses libraries and code from different authors, they are installed automatically from platformio.ini file

- [esp8266](https://github.com/esp8266/Arduino)
- [espsoftwareserial](https://github.com/plerup/espsoftwareserial) by Peter Lerup
- [WiFiManager](https://github.com/tzapu/WiFiManager) by tzapu
- [WebSockets](https://github.com/Links2004/arduinoWebSockets) by Links2004
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson) by Benoit Blanchon
- [Adafruit libraries](https://github.com/adafruit) Adafruit SSD1306, Adafruit GFX Library, adafruit/Adafruit AHTX0 and adafruit/Adafruit BMP280
- LittleFS
- [PubSubClient](https://github.com/knolleary/pubsubclient)
- NTPClient

### Compilation
First things first. Compile it with VSCode and upload it to the board with PlatformIO.

Minimal hardware is ESP8266 and the pcb for adapting signal from serial to AB line. 

- ESP8266 (Wemos D1 Mini R2)
- Toshiba AC serial interface (SoftwareSerial D7=RX, D8=TX)
- Optional I2C sensors (shared bus D1=SCL, D2=SDA):
  - AHT20 (temperature + humidity)  [enabled with `#define USE_AHT20`]
  - BMP280 (temperature + pressure) [enabled with `#define USE_BMP280`]
  - BME280 (temperature + humidity + pressure) [enabled with `#define USE_BME280`]
- Optional OLED 128x64 SSD1306 (I2C D1=SCL, D2=SDA)         [`#define USE_SCREEN`]
- Reset / Config button on D4 (WiFiManager AP)
  
To sum up, the following defines are available and features are disabled by commenting its `#define` in  [config.h](https://github.com/issalig/toshiba_air_cond/blob/master/air/config.h)

- `USE_OTA`      OTA updates
- `USE_SCREEN`   OLED status display
- `USE_MQTT`     MQTT + Home Assistant integration
- `USE_AHT20`    AHT20 sensor support
- `USE_BMP280`   BMP280 sensor support


### WiFi setup
Once you have the code uploaded it is time to configure your WiFi. This project makes use of the great WiFiManager library so there is no need to hardcode your WiFi settings.
- Plug you esp board. It will start blinking.
- Connect to **airAP** wifi network from your cellphone or PC.
- Open a browser and the WifiManager will appear.
- Select your WiFi network and the password.
- If everything is correct, airAP shuts down and board connects to your WiFi.
- Connect to your normal WiFi from computer/cellphone.

### Connect
Now the esp8266 is connected to your network and can be reached as http://air.local

### Upload files
Connect to http://air.local/filemanager, type ![index.html](https://github.com/issalig/toshiba_air_cond/blob/master/air/data/index.html) and upload it.

This will store index.html file and http://air.local should show the main page.
You can use this endpoint to modify the webpage or add more functionality. If you do it please share.

### Delete files
Similarly to upload page you can use http://air.local/filemanager to delete a file.

### OTA and file update
OTA updates are available, so you do not need to unplug the esp everytime you want to flash it. In the Arduino IDE just set Tools->Port->air at xxx.
If you are using PlatformIO it is done in platform.io
Default OTA password is esp8266

```
; OTA upload configuration
upload_protocol = espota
upload_port = air.local        ; Replace with your device's actual IP from OTAName in config.cpp
upload_flags = --auth=esp8266  ; Replace with your OTA password from OTAPassword varialble in config.cpp
```


If you just want to upload individual files you can use http://air.local/edit.html

# Web Interface
[Up](#toshiba_air_cond) [Previous](#Software-Installation) [Next](#Hardware-Installation)

This section describes the features available through the embedded web interface served by the ESP8266 air conditioning controller.

## Access
- URL via mDNS: `http://air.local` (See `mdnsName` in `config.h`)
- HTTP Port: 80
- WebSocket Port: 81

![web interface](https://github.com/user-attachments/assets/c799ae9e-48ad-44bb-9438-c98791343c15))

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
   - Info: IP, boot time and decoding errors.
   - Sensors: Internal / External AC sensors.
   - Address: Address configuration for master/remote in case your system uses different from default or you want to install different remotes.
   - Mode: 
    - Autonomous mode indicator: Use it if there is no remote connected (sends pings like remotes). It is necessary to have a temperature sensor to report room temperture.
    - Simulation mode indicator: Simulates a physical AC
   - Admin: Upload files. Use it to upload index.html and others.
5. Debug
    - Send RAW messages to AC, i.e., "00 FE 10 02 80 8A E6"
    - Debug output: Shows serial raw and decoded messages
    - Log

6. MQTT Configuration (if `USE_MQTT`)
   - Runtime modification of: host, port, username, password, device name
   - Persisted to `/mqtt_config.json` in LittleFS

# Home Assistant & MQTT
[Up](#toshiba_air_cond) [Previous](#Web-Interface) [Next](#Hardware-Installation)

If `USE_MQTT` is enabled, the device integrates with Home Assistant using MQTT Discovery.

### Discovery
The device automatically publishes configuration to:
`homeassistant/climate/toshiba_ac/config`

### Manual Configuration / Topics
If discovery is not used, here are the available topics:

| Topic Type | Topic Path | Payload / Description |
|---|---|---|
| **Status** | `homeassistant/ac/status/mode` | `cool`, `heat`, `auto`, `fan_only`, `dry`, `off` |
| **Status** | `homeassistant/ac/status/fan_mode` | `auto`, `high`, `medium`, `low` |
| **Status** | `homeassistant/ac/status/temperature` | Target Temperature (e.g. `22`) |
| **Status** | `homeassistant/ac/status/current_temperature` | Room Temperature (e.g. `21.5`) |
| **Command** | `homeassistant/ac/set/mode` | Set mode (same values as status + `on`/`off`) |
| **Command** | `homeassistant/ac/set/fan_mode` | Set fan speed |
| **Command** | `homeassistant/ac/set/temperature` | Set target temperature |

### Configuration
MQTT settings (Broker, User, Password) can be configured via the Web Interface and are saved to `/mqtt_config.json`. The device status (Power, Mode, Fan, Temperature) is periodically published and updated in real-time when changed via Web or Remote.

# Hardware installation
[Up](#toshiba_air_cond) [Previous](#Home-Assistant-&-MQTT) [Next](#Data-acquisition)

You will need an esp8266, a circuit for adapting signals to esp8266, a USB power supply, a a couple of dupont (female) wires.
- Take out the cover of your remote controller
- Loose the screws of AB terminals. **WARNING**: My PCB assumes A is positive and B is negative. If this is not your case you can damage the PCB. (https://github.com/issalig/toshiba_air_cond/discussions/40#discussioncomment-8149607)
- Pass the wires through the ventilation holes of the cover.
- Insert dupont wires on the terminals and screw them again
- Close the cover
- Connect dupont wires to the pcb (A,B)
- Plug the Wemos D1 mini into the pcb
- Power Wemos with usb cable and the led will start blinking (if you have already programmed it)

Just switch it on/off while you are in bed. If you like it just send me a beer and/or improve the project!

![image](https://github.com/issalig/toshiba_air_cond/blob/master/pcb/remote_back_pcb.jpg)
![image](https://github.com/issalig/toshiba_air_cond/blob/master/pcb/mounted_board.jpg)

# Data acquisition
[Up](#toshiba_air_cond) [Previous](#Hardware-Installation) [Next](#Custom-hardware)

This is how I managed to decode the information from the AB bus. First I plugged a multimeter to check the range of the signal and not fry anything. Then I used a DS0138 oscilloscope to monitor the signal and to guess voltages and baudrate (a resistor divider is suggested in order to lower the voltage). Later, an 8-channel USB logic analyzer (4-5 USD) can be used to capture data into the computer. **REMEMBER** to convert voltages to 0-3.3v before connecting it to logic analyzer or you will make magic smoke. You can use the read circuit below.

To capture data you can use pulseview with uart decoder 2400 bps, 8bits, start, stop, EVEN parity

In case you need it you can install the following packages
```
sudo apt install sigrok-cli
sudo apt install sigrok-firmware-fx2lafw
```

When the data has been validated visually you can use the following command line that reads RX data annotations and print one message per line according to 4th byte (message size).

```
sigrok-cli -P uart:rx=D0:baudrate=2400:parity_type=even -A uart=rx_data -i  YOURFILE  | awk '{pad =" "; b[len%4]=$2; if(len==3) {bytes="0x"b[len];  printf("%s%s%s%s%s%s%s%s",b[0],pad,b[1],pad,b[2],pad,b[3],pad)} if(len>3) {printf("%s%s",$2,pad);} len=len+1; if(len==4+bytes+1) {print "";len=0;bytes=0}}'
```

```
sigrok-cli -d fx2lafw -c samplerate=250000 -t D0=r -P uart:rx=D0:baudrate=2400:parity_type=even  -A uart=rx_data --continuous
```

# Custom hardware
[Up](#toshiba_air_cond) [Previous](#Data-acquisition) [Next](#Data-format)

Note: I strongly recommend to use [makusets board](https://github.com/makusets/esphome-toshiba-ab/tree/main/hardware/D1%20mini) instead of my custom hardware.
My hardware is designed assuming a voltage level of the AB line around 15.6V when "1" and 14V when "0". Different voltage values will not work for writing while reading will work.

I have designed some circuits to read and write the signal

## Read
I will use an optocoupler because it simplifies things and also isolates microcontroller from the rest of the system.

- Air conditioning side:
Voltage is around 15.6 volts when "1" and 14 when "0". A zener diode in reversed position provides 13V reference given that voltage is in the range [14, 15.6] and is > 13V. Thus, the signal is now in the range [1V, 2.6V]. Then, after diode 1N4001 (0.7V drop) voltage is 0.3V .. 1.9V, enough to activate the photodiode (1.2V) when "1" and to not activate it when "0".

IR led from optocouple drops 1.2v, and from signal we have a difference of 15.6V-13V=2.6V, thus 2.6V-1.2V=1.4V/100ohm = 14mA which has a maximum CTR=140%
Ic=3.3, If=14
CTR=Ic/If

```
Type     VZnom  IZT  for  rzjT    rzjk  at  IZK    IR  at  VR
1N4743A  13     19        <10     <100      0.25   <5      9.9
```

Now, let's calculate the resistor value
Izt=19 mA -> 2.6/19=130ohm  P=VI 2.6*19 =52mW


- Microcontroller side: 1k resistor limits the current. ESP8266 max current is 12mA > 3.3/1k = 3.3 mA

```
Air                                         Microcontroller
Conditioner

                          1N4001  _______
  A ---------+----100R ---->|----|       |-------3v3
             |                   | PC817 |
            10k                  |       |
             |                   |       |
  B --->|----+-------------------|_______|------ OUT               
                                             |
     zener 13v                              1k
      1N4743A                                |
                                            GND
             
```

## Write

Write circuit performs similarly to read circuit. 
- When OUT signal is "1", transistor and pullup resistor are "0", thus optocoupler is OFF and voltage is 15.6 (HIGH). 
- When OUT signal is 0, transistor is off and pullup resistor sends 1 and activates optocoupler and zener diode gives 13V (LOW).
Some systems recommend to set the Follower in the remote unit.

Here I attach the datasheets of the components.
https://www.onsemi.com/pub/Collateral/P2N2222A-D.PDF 
https://learnabout-electronics.org/Downloads/PC817%20optocoupler.pdf


```

              3v3                                    1N4001
               |                             |-----+---|<------- A               
              200                 _______    |     |               
               ------------------|       |---|     ^ zener 13v
               /                 | PC817 |         /
 OUT --1k- ---|   2N2222     |---|_______|---1k---| 2N2222
               \             |                     \
               |             |                     |
              GND           GND                    ------------- B
  

```

## Plan B. (In fact it was plan A but then I managed to decode AB protocol, yeah!)

To solder wires to button pads on the remote controller and close circuit to simulate pressing them (with and optocoupler).

``` 
                       _________
    uc OUT --- 200R----| PC817 |------- PAD+
                 GND---|_______|---4k7--PAD-
                                  
```

ESP8266 high level is 3v3 and the maximum current per pin is 12mA (but we will go safer with 10mA). Thus, the resistor for the IR diode of the optocoupler is 3.3-1.2/10=210 -> 200 ohm. 4k7 is a safe value since we just want continuity.

Following traces from button pads end in 2k resistors that we will use to solder wires. As R46 is common, we can think is button GND
ON button is connected to R25 and R46
Temp down button is connected to R23 adn R46
Temp up button is connected to R24 and R46


# Data format
[Up](#toshiba_air_cond) [Previous](#Custom-hardware) [Next](#Message-types)

Data packages have the following format:

|Source | Dest | Opcode 1  | Data Length | Data | CRC |
|---|---|---|---|---|---|

And Data field is composed of the following parts:

| R/W mode | Opcode 2 | Payload |
|---|---|---|


Source/Dest (1 byte): 
|#|Description|
|---|---|
|00 | master (central unit) |
|40 | remote controller in the range [0x40..] |
|FE | broadcast |
|F0 | Group
|52 |??|

Operation code 1 (1 byte) 
- From master (00)
  |Opc1|Desc|Example|
  |---|---|---|
  |10| ping | 00 FE **10** 02 80 8A E6|
  |11| parameters (temp. power, mode, fan, save)| 00 52 **11** 04 80 86 84 01 C4| 
  |1A| sensor value | 00 40 **1A** 07 80 EF 80 00 2C 00 2B B5|
  |1C| status |00 FE **1C** 0D 80 81 8D AC 00 00 76 00 33 33 01 00 01 B9|
  |58| extended status | 00 FE **58** 0F 80 81 34 A8 00 00 6C 6D E9 00 55 55 01 00 01 DC|
  |18| pong, answer to remote control ping |00 40 **18** 08 80 0C 00 03 00 00 48 00 97|
  |18| master ack after setting param  |00 40 **18** 02 80 A1 7B|
- From remote (40)
  |Opc1|Desc|Example|
  |---|---|---|
  |11| set power | 40 00 **11** 03 08 41 03 18 |
  |11| set mode | 40 00 **11** 03 08 42 01 19 (heat)|
  |11| set temp | 40 00 **11** 08 08 4C 09 1D 6C 00 05 05 65|
  |11| set save | 40 00 **11** 04 08 54 01 01 09|
  |15| ping | 40 00 **15** 07 08 0C 81 00 00 48 00 9F  |
  |17| sensor query | 40 00 **17** 08 08 80 EF 00 2C 08 00 02 1E |
  |55| temperature | 40 00 **55** 05 08 81 00 69 00 F0 |
  
Data length (1 byte)
- Length of data field

Data (composed of the following parts)

R/W mode (1 byte)
|Mode|Desc|
|---|---|
|08 |for Write mode (from remote 40)|
|80 |for Read mode (from master 00)|

Opcode2

  - From master (00)
  
    | Opcode2 | Desc | Example |
    |---|---|---|
    | 81 | status | 00 FE 1C 0D 80 **81** 34 A8 00 00 6C 00 55 55 01 00 01 1E |
    | 81 | extended status | 00 FE 58 0F 80 **81** 35 AC 02 00 6E 6F E9 00 55 55 01 00 01 DB|
    | 8A | alive   |00 FE 10 02 80 **8A** E6|
    | A1 | ack after setting param  |00 40 18 02 80 **A1** 7B|
    | 86 | mode  |00 52 11 04 80 **86** 24 05 60| 
    | 0C | pong,answer to ping opc2 0C | 00 40 18 08 80 **0C** 00 03 00 00 48 00 97 | 

  - From remote (40)
    
    | Opcode2 | Desc | Example |
    |---|---|---|
    | 41 |power|40 00 11 03 08 **41** 02 19|
    | 42 |mode|40 00 11 03 08 **42** 02 1A|
    | 4C |temp, fan, mode|40 00 11 08 08 **4C** 11 1A 6E 00 55 55 78|
    | 54 |save (opc1 11)|40 00 11 04 08 **54** 01 00 08 |
    | 80 |sensor query|40 00 17 08 08 **80** EF 00 2C 08 00 02 1E|
    | 81 |sensor room temp |40 00 55 05 08 **81** 00 6A 00 F3|
    | 0C 81 | ping |40 00 15 07 08 **0C 81** 00 00 48 00 9F|
    | 0C 82 |timer |40 00 11 09 08 **0C 82** 00 00 30 05 01 01 EB|
    
    

CRC is computed as Checksum8 XOR of all the bytes (Compute it at https://www.scadacore.com/tools/programming-calculators/online-checksum-calculator/)

# Message types

[Up](#toshiba_air_cond) [Previous](#Data-format) [Next](#Other-info)

The protocol relies on a command/response structure. Most control commands are sent by the Remote (Source 0x40), and the Master (Source 0x00) responds with status updates or acknowledgments.

## 1. Control Commands (Remote -> Master)
These commands control the state of the AC unit.

| Command | OpCode 1 | Data Structure (Example) | Description |
|---|---|---|---|
| **Power** | `11` | `40 00 11 03 08 41 [Val] [CRC]` | `Val`: `03`=ON, `02`=OFF |
| **Mode** | `11` | `40 00 11 03 08 42 [Mode] [CRC]` | `Mode`: `01`=Heat, `02`=Cool, `03`=Fan, `04`=Dry, `05`=Auto |
| **Enc. Data**| `11` | `40 00 11 08 08 4C [Mode+] [Fan+] [Temp+] ...` | Combined Setpoint (Temp, Fan, Mode) |
| **Save** | `11` | `40 00 11 04 08 54 01 [Val] [CRC]` | `Val`: `01`=ON, `00`=OFF |
| **Ping** | `15` | `40 00 15 07 08 0C 81 ...` | Keep-alive / Status Request |

**Encoded Data Packet (OpCode 4C) Breakdown:**
The `4C` packet is used when changing Temperature or Fan Speed. It contains multiple state variables.
`40 00 11 08 08 4C [Byte6] [Byte7] [Byte8] 00 [ModeCheck] [ModeCheck] [CRC]`
*   **Byte 6 (Mode)**: `0x10` + Mode (`02`=Cool, `01`=Heat, etc.)
*   **Byte 7 (Fan)**: `0x18` + Fan (`0`=Auto, `1`=High, `2`=Med, `5`=Low) -> *Note: bitmasks vary*
*   **Byte 8 (Temp)**: `((TargetTemp + 35) << 1)`
*   **ModeCheck**: `0x33` for Cool/Dry/Fan/Auto, `0x55` for Heat.

## 2. Status Reports (Master -> Remote)
The Master unit periodically broadcasts its status or responds to specific queries.

| Type | OpCode 1 | OpCode 2 | Description |
|---|---|---|---|
| **Status** | `1C` | `81` | Standard periodic status (Power, Mode, Fan, RoomTemp) |
| **Ext. Status**| `58` | `81` | Extended status (Filter, Preheat, Errors, Extra Temps) |
| **Alive** | `10` | `8A` | Frequent Keep-Alive broadcast from Master |
| **ACK** | `18` | `A1` | Acknowledgment after a valid setting change |
| **Mode Stat**| `11` | `86` | Mode confirmation |
| **Pong** | `18` | `0C` | Response to Remote Ping |

**Status Packet Parsing (OpCode 1C):**
`00 FE 1C 0D 80 81 [D1] [D2] 00 00 [Temp] 00 [Chk] [Chk] [Sv] 00 [Pwr] [CRC]`
*   **D1 (Mode)**: Mode is in bits 7-5. `0x80` mask often seen.
*   **D2 (Fan)**: Fan speed in bits 7-5.
*   **Temp**: `((Value >> 1) - 35)` = Room Temperature.
*   **Sv (Save)**: Bit 0 indicates Save Mode.
*   **Pwr**: Bit 0 indicates Power (1=ON).

## 3. Configuration & Initialization
These messages occur during startup or when "exploring" the system.

| Command | OpCode 1 | OpCode 2 | Description |
|---|---|---|---|
| **Announce** | `15` | `0D` | Remote announces presence (`40 F0...`) |
| **Link** | `18` | `0D` | Master confirms link/features (`00 40...`) |
| **Model** | `15`/`18`| `08` | Request/Response AC Model String (ASCII) |
| **Limits** | `15`/`18`| `0A` | Request/Response Temp Limits (Min/Max/Frost) |
| **DN Code** | `15`/`18`| `02` | Request/Response Configuration Codes (Settings) |

**DN Codes**: Used to configure deep settings of the AC (e.g., jumper settings, addresses).
*   Request: `40 00 15 05 08 02 F5 00 [Code] [CRC]`
*   Response: `00 40 18 07 80 02 01 [Val] [Next] 00 [CRC]`

## 4. Sensor & Maintenance
Messages for reading specific sensor values or error history.

| Command | OpCode 1 | OpCode 2 | Description |
|---|---|---|---|
| **Query** | `17` | `80` | Request specific sensor ID (see Sensor Addresses) |
| **Answer** | `1A` | `EF` | Sensor Value Response |
| **Errors** | `15`/`18`| `27` | Request/Response Error History |

**Sensor Answer Format:**
`00 40 1A 07 80 EF 80 00 2C [ValH] [ValL] [CRC]`
*   Value is typically signed int16. For temperatures, often `Value / 2 - 35` or raw.

# Sensor addresses
These are the sensor addresses for sensor query.

| No.  | Desc  | Example value  |
|---|---|---|
| 00 | Room Temp (Control Temp) (°C) | Obtained from master status frames 00 FE 1C ...|
| 01 | Room temperature (remote controller) | Obtained from controller messages 40 00 55 ... |
| 02 | Indoor unit intake air temperature (TA) | 23 |
| 03 | Indoor unit heat exchanger (coil) temperature (TCJ) Liquid | 19 |
| 04 | Indoor unit heat exchanger (coil) temperature (TC) Vapor | 19 |
| 07 | Indoor Fan Speed|  0 |
| 60 | Outdoor unit heat exchanger (coil) temperature (TE) | 18 |
| 61 | Outside air temperature (TO)| 19 |
| 62 | Compressor discharge temperature (TD) | 33 |
| 63 | Compressor suction temperature (TS) | 26 |
| 65 | Heatsink temperature (THS) | 55 |
| 6a | Operating current (x1/10) | 0 |
| 6d | TL Liquid Temp (°C) | 22 |
| 70 | Compressor Frequency (rps)| 0 |
| 72 | Fan Speed (Lower) (rpm) | 0 |
| 73 | Fan Speed (Upper) (rpm) | defined in manual, not working  |
| 74 | ? | 43, is this fan speed upper? |
| 75 | ? | 0 |
| 76 | ? | 0 |
| 77 | ? | 0 |
| 78 | ? | 0 |
| 79 | ? | 0 |
| f0 | ? | 204 |
| f1 | Compressor cumulative operating hours (x100 h) | 7 |
| f2 | Fan Run Time (x 100h) | 8 |
| f3 | Filter sign time x 1h | 37 |
| f8 | Indoor Discharge Temperature | - |

# TO-DOS
[Up](#toshiba_air_cond) [Previous](#Message-types) [Next](#Other-info)

- Improve PCB

- Fix PCB: jumper for Hardware or Software Serial

- Fix parsing to support round buffer and not to loose partial frames (not necessary)

- Clearer Protocol documentation

- Check other circuits as:
  - https://easyeda.com/marcegli/door-opener
  - https://frog32.ch/smart-intercom.html 
  - https://electronics.stackexchange.com/questions/458996/logic-level-converter-for-nodemcu-esp8266-input-24v-16v-hi-lo-500-baud 
  - https://sudonull.com/post/18480-We-pump-the-intercom-with-the-MQTT-protocol-to-control-from-the-phone
  - https://hackaday.com/2019/01/07/building-an-esp8266-doorbell-on-hard-mode/
  - https://daeconsulting.co.za/2018/12/17/theres-someone-at-the-door/

- Announce project in other similar ones
  - https://github.com/ToniA/arduino-heatpumpir/
  - https://github.com/openenergymonitor
  - https://github.com/roarfred/AmsToMqttBridge
  - https://github.com/dgoodlad/esp8266-mitsubishi-aircon
  - https://github.com/H4jen/webasto_sniffer

# Other info
[Up](#toshiba_air_cond) [Previous](#Message-types)

If you want to know about error codes and sensor addresses you can check the following links.
http://www.toshiba-aircon.co.uk/assets/uploads/pdf/sales_tools/Technical_Handbook_ver._13.1.pdf
https://www.cdlweb.info/wp-content/uploads/2020/10/1-CDL-Toshiba-R32-Technical-Handbook-V10-2020.pdf
https://www.toshibaclim.com/Portals/0/Documentation/Manuels%20produits/SM_CassetteUTP_DI-SDI-111416-E_GB.pdf


I found this projects interesting even that it is not the same protocol https://github.com/H4jen/webasto_sniffer
https://echonet.jp/wp/wp-content/uploads/pdf/General/Standard/Release/Release_F_en/SpecAppendixF_e.pdf

Info about commercial gateways but no info about protocol :(

Connections https://www.toshibaclim.com/Portals/0/Documentation/Manuels%20produits/SM_CassetteUTP_DI-SDI-111416-E_GB.pdf
Sensor addresses (pg 52) http://www.toshiba-aircon.co.uk/assets/uploads/pdf/sales_tools/New_Technical_Handbook_version_14_1_3.pdf
Sensor addresses (pg 73) https://www.toshibaclim.com/Portals/0/Documentation/Manuels%20produits/SM_CassetteUTP_DI-SDI-111416-E_GB.pdf
Sensor addresses (pg 42) http://www.toshiba-aircon.co.uk/assets/uploads/pdf/sales_tools/Technical_Handbook_ver._13.1.pdf

Error codes from Toshiba (pg 38) https://cdn.shopify.com/s/files/1/1144/2302/files/BP-STD_Toshiba_v1_08.pdf
Temperature formula TCS-Net https://www.toshibaheatpumps.com/application/files/8914/8124/4818/Owners_Manual_-_Modbus_TCB-IFMB640TLE_E88909601.pdf
https://www.toshibaheatpumps.com/customer-support/owner-manuals
https://www.intesisbox.com/intesis/product/media/intesisbox_to-ac-knx-16-64_user_manual_en.pdf?v=2.2

The unit where I have tested the project is model RAV-SM406BTP-E  which stands for

```
RAV-SM406BTP-E 
|     | |    |- CE marking
|     | ||-Duct
|     | |-gen
|     |-duty 4.0 kW 
|   |-Digital inverter
|- Light comercial
```
