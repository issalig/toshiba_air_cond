# toshiba_air_cond
This project implements functions to decode Toshiba AB protocol from indoor units to wired controllers and provides a hardware design for communication.

Gerbers are available but remember if you improve the design please share it, that's how open source works, if you do not want to share, this project is not for you.

In particular, this project has been tested with remote control unit RBC-AMT32E and central unit RAV-SM406BTP-E (http://www.toshiba-aircon.co.uk/assets/uploads/product_assets/20131115_IM_1115460101_Standard_Duct_RAV-SM_6BTP-E_EN.pdf)

You can find the service manual from central unit and wired controller here: http://www.toshibaclim.com/Portals/0/Documentation/Manuels%20produits/SM_Gainable_Std-Compact--DI_406566806110614061606_GB.pdf, https://rednux.com/mediafiles/Hersteller/toshiba/Toshiba-Bedienungsanleitung-RBC-AMT32E-Englisch.pdf

## Index

[Software installation](#Software-installation)

[Web Interface](#Web-Interface)

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
First things first. Compile it with VSCode and upload it to the board. If use are using Arduino IDE you will need to install the previous libraries and maybe some others. Once it is compiled it means you have all the dependencies installed.

Minimal hardware is ESP8266 and the pcb for adapting signal from serial to AB line. 

- ESP8266 (Wemos D1 Mini R2)
- Toshiba AC serial interface (SoftwareSerial D7=RX, D8=TX)
- Optional I2C sensors (shared bus D1=SCL, D2=SDA):
  - AHT20 (temperature + humidity)  [enabled with `#define USE_AHT20`]
  - BMP280 (temperature + pressure) [enabled with `#define USE_BMP280`]
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

### Addons
The project and the board support sensors for temperature, humidity and pressure (AHT20 and BMP280), these values are shown in a graph in the webpage. If you do not connect these sensors there is no problem. Graph will show indoor and outdoor temperature reported by the air conditioning.
In the bottom side of the pcb you can find the SPI connections.
![image](https://user-images.githubusercontent.com/7136948/148600587-4383e831-2e45-4c01-80d2-e20d8952b76c.png)

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

# Hardware installation
[Up](#toshiba_air_cond) [Previous](#Web-Interface) [Next](#Data-acquisition)


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


I have designed some circuits to read and write the signal

## Read
I will use an optocoupler because it simplifies things and also isolates microcontroller from the rest of the system.

- Air conditioning side:
Signal is around 15.6 volts when 1 and 14 when 0. Zener diode provides 13V reference, so signal is 1V .. 2.6V and after diode (0.7V drop) is 0.3V .. 1.9V, enough to activate photodiode (1.2V) when 1 and to not activate it when 0.

IR led from optocouple drops 1.2v, and from signal we have a difference of 15.6V-13V=2.6V, thus 2.6V-1.2V=1.4V/100ohm = 14mA which has a maximum CTR=140%
Ic=3.3, If=14
CTR=Ic/If

```
Type     VZnom  IZT  for  rzjT    rzjk  at  IZK    IR  at  VR
1N4743A  13     19        <10     <100      0.25   <5      9.9
```

Izt=19 mA -> 2.6/19=130ohm  P=VI 2.6*19 =52mW


- Microcontroller side: 1k resistor limits the current. ESP8266 max current is 12mA > 3.3/1k = 3.3 mA

```
                             1N4001  _______
  A -----------------100R ---->|----|       |-------------3v3
                |                   | PC817 |
               10k                  |       |
                |                   |       |
  B ------>|z-----------------------|_______|------------ OUT               
                                              |
         zener 13v                            1k
                                              |
                                             GND
             
```

## Write

Write circuit performs similarly to read circuit. When OUT signal is 1, transistor and pullup resistor are 0, thus optocoupler is OFF and voltage is 15.6 (HIGH). When OUT signal is 0, transistor is off and pullup resistor sends 1 and activates optocoupler and zener diode gives 13V (LOW).
Some systems recommend to set the Follower in the remote unit.

Here I attach specs for the components.
https://www.onsemi.com/pub/Collateral/P2N2222A-D.PDF 
https://learnabout-electronics.org/Downloads/PC817%20optocoupler.pdf


```

              3v3                                   1N4001
               |                             |--------<------- A               
               200                _______    |     |               
               ------------------|       |---|     ^ z13v
               |                 | PC817 |         |
 OUT --1k- ---|<  2N2222     |---|_______|---1k---|< 2N2222
               |             |                     |
               |             |                     |
              GND           GND                    ------------ B
  

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




Source (1 byte): 
|#|Desc|
|---|---|
|00 | master (central unit) |
|40 | remote controller |
|FE | broadcast |
|52 |??|

Dest (1 byte):
|#|Desc|
|---|---|
|00 | master (central unit) |
|40 | remote controller|
|FE | broadcast|
|52 |??|

Operation code 1 (1 byte) 
- From master (00)
  |Opc1|Desc|Example|
  |---|---|---|
  |10| ping|    00 FE 10 02 80 8A E6|
  |11| parameters (temp. power, mode, fan, save)| 00 52 11 04 80 86 84 01 C4| 
  |1A| sensor value| 00 40 1A 07 80 EF 80 00 2C 00 2B B5|
  |1C| status |00 FE 1C 0D 80 81 8D AC 00 00 76 00 33 33 01 00 01 B9|
  |58| extended status | |
  |18| pong, answer to remote ping |00 40 18 08 80 0C 00 03 00 00 48 00 97|
  |18| master ack after setting param  |00 40 18 02 80 A1 7B|
- From remote (40)
  |Opc1|Desc|Example|
  |---|---|---|
  |11|  set power |  40 00 11 03 08 41 03 18 |
  |11 | set mode |   40 00 11 03 08 42 01 19 (heat)|
  |11 | set temp | 40 00 11 08 08 4C 09 1D 6C 00 05 05 65|
  |11 | set save | |
  |15| Error history   | 40 00 15 07 08 0C 81 00 00 48 00 9F  |
  |17|   sensor query   | 40 00 17 08 08 80 EF 00 2C 08 00 02 1E |
  | 55        temperature|    40 00 55 05 08 81 00 69 00 F0 |
  

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
    | 81 | status | 00 FE 1C 0D 80 81 34 A8 00 00 6C 00 55 55 01 00 01 1E |
    | 81 | extended status | 00 FE 58 0F 80 81 35 AC 02 00 6E 6F E9 00 55 55 01 00 01 DB|
    | 8A | alive   |00 FE 10 02 80 8A E6|
    | A1 | ack after setting param  |00 40 18 02 80 A1 7B|
    | 86 | mode  |00 52 11 04 80 86 24 05 60| 
    | 0C | pong,answer to ping opc2 0C | 00 40 18 08 80 0c 00 03 00 00 48 00 97 | 

  - From remote (40)
  
  
    | Opcode2 | Desc | Example |
    |---|---|---|
    | 41 |power|40 00 11 03 08 41 02 19|
    | 42 |mode|40 00 11 03 08 42 02 1A|
    | 4C |temp, fan|40 00 11 08 08 4C 11 1A 6E 00 55 55 78|
    | 54 |save (opc1 11)|40 00 11 04 08 54 01 00 08 |
    | 80 |sensor query|40 00 17 08 08 80 EF 00 2C 08 00 02 1E|
    | 81 |sensor room temp |40 00 55 05 08 81 00 6A 00 F3|
    | 0C 81 | ping |40 00 15 07 08 0C 81 00 00 48 00 9F|
    | 0C 82 |timer |40 00 11 09 08 0C 82 00 00 30 05 01 01 EB|
    
    

CRC is computed as Checksum8 XOR of all the bytes (Compute it at https://www.scadacore.com/tools/programming-calculators/online-checksum-calculator/)

# Message types

[Up](#toshiba_air_cond) [Previous](#Data-format) [Next](#Other-info)

There are two different status messages sent from master: normal and extended. Opcode for both is 81
Extended has two extra bytes, one could be some temperature? and other is always E9 in my experiments.

Normal status
```
00 FE 1C 0D 80 81 8D AC 00 00 76 00 33 33 01 00 01 B9
|  |     ||       |  |         |    |      |- save mode bit0
|  |     ||       |  |         |    |  |- bit2 HEAT:1 COLD:0 
|  |-Dst |        |  |         |    |- bit2 HEAT:1 COLD:0
|-Src    |        |  |         |- bit7..bit1  - 35 =Temp
         |        |  |-bit7..bit5 fan level (auto:010 med:011 high:110 low:101 )
         |        |  |-bit2 ON:1 OFF:0
         |        |-bit7.bit5 (mode cool:010 fan:011 auto 101 heat:001 dry: 100)
         |        |-bit0 ON:1 OFF:0
         |-Byte count
         
remote, last byte bit0
master, status in two bits  byte bit0  byte bit2
```
Extended status 
```
                                 -- -- extra values in extended
00 FE 58 0F 80 81 8D A8 00 00 7A 84 E9 00 33 33 01 00 01 9B
                                    |-always E9
                                 |-  1000 0100  1000010 66-35=31 (real temp??)  
                              |-temp 0111 1010 111101 61-35 = 26    
                        |- 0x2 pre-heat
 
temperature reading also confirmed  in pg14 https://www.toshibaheatpumps.com/application/files/8914/8124/4818/Owners_Manual_-_Modbus_TCB-IFMB640TLE_E88909601.pdf                                                      
``` 

ALIVE message (sent every 5 seconds from master)
```
00 fe 10 02 80 8a e6 
|  |  |  |  |  |  |- CRC
|  | OPC1|  |  |- OPC2 
|  ALL   |  |-from master, info message??
Master   |- Length

From master (00) to all (fe)

```

ACK sent after setting parameters
```
00 40 18 02 80 a1 7b 
```

PING/PONG, remote pings and master pongs
```
40 00 15 07 08 0c 81 00 00 48 00 9f 
               |- opcode ping
               
00 40 18 08 80 0c 00 03 00 00 48 00 97
               |- opcode ping
```

Temp from remote to master.
```
40 00 55 05 08 81 00 68 00 f1 
      |           |  |- 0110 1000 -> 104/2 -> 52 - 35 = 17   (temp)
      |-opc1         |-bit0 ON:1 OFF:0                 
      
```

From master mode status Opcode 2 0x86
```
00 52 11 04 80 86 24 00 65  heat
      |-opc1      |  |- mode  bit7-bit5, power bit0, bit2 ???
                  |- 0010 0100 -> mode bit7-bit5  bit4-bit0 ???
                     ---                  
```

Setting commands

Power
```
40 00 11 03 08 41 03 18    ON
40 00 11 03 08 41 02 19   OFF
                  |- power >> 1 | 0b1
```                  
Set mode
```
40 00 11 03 08 42 01 19 //heat
                  |-(Mode is bit3-bit0 from last data 6th byte) cool:010 fan:011 auto 101 heat:001 dry: 100            
```

Set temperature
```
00 01 02 03 04 05 06 07 08 09 10 11 12  byte nr
40 00 11 08 08 4C 0C 1D 7A 00 33 33 76
                              |  |-0x33 for cold 0x55 for heat
                              |-0x33 for cold 0x55 for heat
                  |  |  |- ((target_temp) + 35) << 1
                  |  |- fan | 0b11000 
                  |- mode | 0b1000  cool:010 fan:011 auto 101 heat:001 dry: 100
```

Set fan (requires info about target temp, fan and heat/cold mode)

```
00 01 02 03 04 05 06 07 08 09 10 11 12 byte nr
40 00 11 08 08 4C 13 1D 7A 00 33 33 6E //LOW, FAN
                  |  |  |     |  |-0x33 for cold 0x55 for heat
                  |  |  |     |- 0x33 for cold 0x55 for heat
                  |  |  |- (target_temp) + 35) << 1
                  |  |- fan bit4=1 bit3-bit1  auto:0x010 med:011 high:110 low:101
                  |-  0x10 + mode  cool:010 fan:011 auto 101 heat:001 dry: 100
```

Set save
```
40 00 11 04 08 54 01 01 09  Save ON
40 00 11 04 08 54 01 00 08  Save OFF
                     |- bit 0 7th byte
```
TEST+SET for Error history
```
40 00 15 03 08 27 01 78
                  |-Error 1
00 40 18 05 80 27 08 00 48 ba
                        |-Type 0x4 Num error 0x8   E-08

```

TEST + CL for sensor query
```
40 00 17 08 08 80 EF 00 2C 08 00 F3 EF   
                                 |- Filter sign time
00 40 1A 07 80 EF 80 00 2C 03 1E 83  
                           |  |
                           |--|------0x03 0x1E->  798  2bytes

00 40 1A 05 80 EF 80 00 A2 12  answer for unknown sensor

```

Timer is decoded but remote takes care of it internally and ignores it if sent from external sources (our circuit)

```
40 00 11 09 08 0c 82 00 00 30 07 02 02 e9    1h for poweron
                                    |----- number of 30 minutes periods,  2 -> 1h
                                 |----- number of 30 minutes periods,  2 -> 1h
                              |------ 07 poweron   06 poweroff repeat 05 poweroff  00 cancel
                              

Sequence observed for 1h poweron

40 00 11 03 08 41 03 18    powers on
00 40 18 02 80 a1 7b       ack
40 00 11 09 08 0c 82 00 00 30 00 04 01 eb  cancels timer
00 40 18 02 80 a1 7b       ack

```
# Sensor addresses

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


# Notes from logs (these are my notes when I started to decode messages)

```
Op code from remote
4C 0C 1D  set temp     40 00 11 08 08 4C 0C 1D 78 00 33 33 74
0C 81     status      
41        power        40 00 11 03 08 41 03 18
42        mode         40 00 11 03 08 42 02 1A //cool  02 -> 0000 0010
4C 14     fan          40 00 11 08 08 4C 14 1D 7A 00 33 33 6E  //low
54        save         40 00 11 04 08 54 01 00 08
0C 82     timer
40 00 15 07 08 0c 81 00 00 48 00 9f 
00 40 18 08 80 0c 00 03 00 00 48 00 97 

Op code from master
81 status              00 FE 1C 0D 80 81 8D AC 00 00 7A 00 33 33 01 00 01 B5
8A ack (dest FE)       00 FE 10 02 80 8A E6 # every 5s
A1 ack (dest 40)       00 40 18 02 80 A1 7B
86 ?? (dest 52)        00 52 11 04 80 86 84 05 C0
                       00 52 11 04 80 86 84 01 C4   DRY,LOW
                       00 52 11 04 80 86 64 01 24   FAN,LOW
                       00 52 11 04 80 86 44 01 04   COOL,LOW
                       00 52 11 04 80 86 44 05 00   COOL,MED
                       
0C (ASNWER TO 0c)      00 40 18 08 80 0C 00 03 00 00 48 00 97 

UNK is
10 ack / ping??           00 FE 10 02 80 8A E6
1C status after request   00 FE 1C 0D 80 81 8D AC 00 00 7A 00 33 33 01 00 01 B5
58 ?? status when idle    00 FE 58 0F 80 81 8D AC 00 00 7A 84 E9 00 33 33 01 00 01 9E
18 ack (dest remote)      00 40 18 02 80 A1 7B 
11 ??                     40 00 11 03 08 41 03 18
                          00 52 11 04 80 86 84 05 C0
15 (from remote)          40 00 15 07 08 0C 81 00 00 48 00 9F
55 (from remote)          40 00 55 05 08 81 00 7E 00 E7                
                          40 00 55 05 08 81 00 7C 00 E5 (heat mode 24)

first part is 1,5
second part is 0,1,8,C


00 FE 1C 0D 80 81 CD 8C 00 00 76 00 33 33 01 00 01 D9    // CD 8C 00 -> 1100 1101 1000 1100
00 FE 10 02 80 8A E6                                                    ---         -

Status
00 FE 1C 0D 80 81 8D AC 00 00 76 00 33 33 01 00 01 B9
|  |     ||       |  |         |    |      |- save mode bit0
|  |     ||       |  |         |    |  |- bit2 HEAT:1 COLD:0 
|  |-Dst |        |  |         |    |- bit2 HEAT:1 COLD:0
|-Src    |        |  |         |- bit7..bit1  - 35 =Temp
         |        |  |-bit7..bit5 fan level (auto:010 med:011 high:110 low:101 )
         |        |  |-bit2 ON:1 OFF:0
         |        |-bit7.bit5 (mode cool:010 fan:011 auto 101 heat:001 dry: 100)
         |        |-bit0 ON:1 OFF:0
         |-Byte count
         
remote, last byte bit0
master, status in two bits  byte bit0  byte bit2

         
00 FE 1C 0D 80 81 8D AC 00 00 7A 00 33 33 01 00 01 B5         -> 8D AC  1000 1101 1010 1100
                   -  -                                                         -       -         
cool
00 FE 1C 0D 80 81 4D AC 00 00 76 00 33 33 01 00 01 79    // 4D AC 00 -> 0100 1101 1010 1100
                                                                        ---
00 FE 1C 0D 80 81 8D AC 00 00 7A 00 33 33 01 00 01 B5   -> A 1010
                     -                                       ---

Extended status
00 FE 58 0F 80 81 8D AC 00 00 7A 7A E9 00 33 33 01 00 01 60 
00 FE 58 0F 80 81 8D A8 00 00 7A 84 E9 00 33 33 01 00 01 9B
                                    |-always E9
                                 |-  1000 0100  1000010 66-35=31 (real temp??)  
                              |-temp 0111 1010 111101 61-35 = 26
                              
                              
Current temp from remote (off 27C and blow on thermistor until 30C and cold again)
3rd byte is 55
bit7..bit0 /2 - 35 =Temp
40 00 55 05 08 81 00 7C 00 E5  //7C  0111 1100   124    124/2-35 = 27
40 00 55 05 08 81 00 83 00 1A  //83  1000 0011   131    131/2-35= 30.5
40 00 55 05 08 81 00 7E 00 E7  //7E  0111 1110   126    126/2-35= 28
                     --                                       
```
```
40 00 11 08 08 4c 11 1c 6c 00 55 55 7c  //Set Fan Medium
00 40 18 02 80 a1 7b  //kind of ack
```

Communication while POWERED OFF
```
00 FE 10 02 80 8A E6 # 
00 FE 10 02 80 8A E6 # every 5s
40 00 15 07 08 0C 81 00 00 48 00 9F
00 40 18 08 80 0C 00 03 00 00 48 00 97 #inmediately answer
40 00 55 05 08 81 00 7E 00 E7 
00 FE 58 0F 80 81 8D A8 00 00 7A 84 E9 00 33 33 01 00 01 9B
00 FE 10 02 80 8A E6
00 FE 10 02 80 8A E6
```

When POWERED ON 
```
00 FE 10 02 80 8A E6 # typical answer, maybe confirmation
40 00 15 07 08 0C 81 00 00 48 00 9F
00 40 18 08 80 0C 00 03 00 00 48 00 97 #inmediate answer 
40 00 55 05 08 81 00 7E 00 E7 
00 FE 58 0F 80 81 8D AC 00 00 7A 84 E9 00 33 33 01 00 01 9E
00 FE 10 02 80 8A E6 # typical answer, maybe confirmation
00 52 11 04 80 86 84 01 C4
00 FE 10 02 80 8A E6 # typical answer, maybe confirmation
00 FE 10 02 80 8A E6 # typical answer, maybe confirmation (every 5s we see  the mesg)
00 FE 10 02 80 8A E6 # typical answer, maybe confirmation (every 5s we see  the mesg)
40 00 15 07 08 0C 81 00 00 48 00 9F
00 40 18 08 80 0C 00 03 00 00 48 00 97 #inmediate answer
40 00 55 05 08 81 00 7E 00 E7
00 FE 58 0F 80 81 8D AC 00 00 7A 84 E9 00 33 33 01 00 01 9E  #inmediate answer
```

Temp down
```
40 00 11 08 08 4C 0C 1D 78 00 33 33 74 # press temp down (current 26, 25 after pressing)
00 40 18 02 80 A1 7B # typical answer, maybe confirmation
00 FE 1C 0D 80 81 8D AC 00 00 78 00 33 33 01 00 01 B7 # looks like current status
00 52 11 04 80 86 84 01 C4
00 FE 10 02 80 8A E6 # typical answer, maybe confirmation
```

```
only messages from remote
method 1
40 00 11 08 08 4C 0C 1D 7C 00 33 33 70 //27     7C -> 0111 1100  111110  14     30    62
40 00 11 08 08 4C 0C 1D 7E 00 33 33 72 //28     7E -> 0111 1110  011111  15     31    63
40 00 11 08 08 4C 0C 1D 80 00 33 33 8C //29     80 -> 1000 0000  100000   0 ???       64
                                                      ---- ---
40 00 11 08 08 4C 0C 1D 7C 00 33 33 70 //27     7C -> 0111 1100  111110  14           62
40 00 11 08 08 4C 0C 1D 7A 00 33 33 76 //26     7A -> 0111 1010  111101  13           61
40 00 11 08 08 4C 0C 1D 6E 00 33 33 62 //20     6E -> 0110 1110  110111               55
40 00 11 08 08 4C 0C 1D 6A 00 33 33 66 //18     6A -> 0110 1010  110101               53  - 35 = 18

Full log
40 00 11 08 08 4C 0C 1D 78 00 33 33 74 //25     78 -> 0111 1000  111100               60
00 40 18 02 80 A1 7B # typical answer, maybe ack
00 FE 1C 0D 80 81 8D AC 00 00 78 00 33 33 01 00 01 B7  //78

00 52 11 04 80 86 84 05 C0 #this seq was in temp up ???

00 FE 1C 0D 80 81 8D AC 00 00 76 00 33 33 01 00 01 B9


```

Power
```
remote, last byte bit0
master, status in two bits  byte bit0  byte bit2

ON
40 00 11 03 08 41 03 18    ->03 0000 0011
                   -                    -
00 40 18 02 80 A1 7B # typical answer, maybe confirmation
00 FE 1C 0D 80 81 8D AC 00 00 7A 00 33 33 01 00 01 B5         -> 8D AC  1000 1101 1010 1100
                   -  -                                                         -       -
00 FE 10 02 80 8A E6 
00 52 11 04 80 86 84 05 C0   -> 5 0101
                      -            - -

OFF
40 00 11 03 08 41 02 19   ->02 0000 0010
                   -                   - 
00 40 18 02 80 A1 7B 
00 FE 1C 0D 80 81 8C A8 00 00 7A 00 33 33 01 00 01 B0         -> 8C A8  1000 1100 1010 1000
00 FE 10 02 80 8A E6                                                            -       -
00 52 11 04 80 86 84 00 C5   -> 0 0000
                      -            - -
```

Modes
```
From remote (Mode is bit3-bit0 from last data byte) cool:010 fan:011 auto 101 heat:001 dry: 100
40 00 11 03 08 42 02 1A //cool  02 -> 0000 0010
40 00 11 03 08 42 03 1B //fan   03 -> 0000 0011
40 00 11 03 08 42 05 1D //auto  05 -> 0000 0101
40 00 15 02 08 42 1D    //???
40 00 11 03 08 42 01 19 //heat  01 -> 0000 0001
40 00 11 03 08 42 04 1C //dry   04 -> 0000 0100


Full log for mode changing (Mode in master is bit3-bit1 from byte 6 (starting from 0)
cool
40 00 11 03 08 42 02 1A                                  //cool   02 -> 0000 0010
00 40 18 02 80 A1 7B                                                          ---
00 FE 1C 0D 80 81 4D AC 00 00 76 00 33 33 01 00 01 79    // 4D AC 00 -> 0100 1101 1010 1100
                                                                        ---
fan
40 00 11 03 08 42 03 1B                                  //fan    03 -> 0000 0011
00 40 18 02 80 A1 7B                                                          ---
00 52 11 04 80 86 64 01 24 
00 FE 1C 0D 80 81 6D AC 00 00 76 00 33 33 01 00 01 59    // 6D AC 00 -> 0110 1101 1010 1100
00 52 11 04 80 86 64 01 24                                              --- 
00 52 11 04 80 86 64 01 24                               // 64 -> 0110 0100       new value here is 3 
                                                                  ---
auto
40 00 11 03 08 42 05 1D                                  //auto  05 -> 0000 0101
00 40 18 02 80 A1 7B                                                         ---
40 00 15 02 08 42 1D                                     
00 40 18 04 80 42 05 06 9D                               //requested 05, assigned 06??
00 FE 1C 0D 80 81 CD 8C 00 00 76 00 33 33 01 00 01 D9    // CD 8C 00 -> 1100 1101 1000 1100
00 FE 10 02 80 8A E6                                                    ---         -
                                                                        mode      fan level bit3-bit1 -> 100 ???
00 52 11 04 80 86 C4 01 84                               // C4 -> 1100 0100       
                                                                  ---                                                                                                                       
heat
40 00 11 03 08 42 01 19                                  //heat  01 -> 0000 0001
00 40 18 02 80 A1 7B                                                         ---
00 FE 1C 0D 80 81 35 AC 02 00 76 00 55 55 01 00 01 03    // 35 AC 02 -> 0011 0101 1010 1100 0000 0010   // 55 55 vs 33 33 in other modes
                                                                        ---                        -       0101 0101    0011 0011
00 52 11 04 80 86 24 01 64                               //24 -> 0010 0100
                                                                 ---
dry
40 00 11 03 08 42 04 1C                                  //dry   04 -> 0000 0100
00 40 18 02 80 A1 7B                                                         ---
00 FE 1C 0D 80 81 8D AC 00 00 7A 00 33 33 01 00 01 B5    // 8D AC 00 -> 1000 1101 1010 1100
                                                                        ---
00 52 11 04 80 86 84 01 C4                               // 84 -> 1000 0100
00 FE 10 02 80 8A E6                                              ---


```

Save mode

```
bit0 from 14th in master message
bit0 from 7th in remote message

40 00 11 04 08 54 01 00 08 
                      -
00 40 18 02 80 A1 7B 
00 FE 1C 0D 80 81 8D AC 00 00 7A 00 33 33 00 00 01 B4 
                                           -
00 52 11 04 80 86 84 01 C4 

40 00 11 04 08 54 01 01 09 
00 40 18 02 80 A1 7B 
00 FE 1C 0D 80 81 8D AC 00 00 7A 00 33 33 01 00 01 B5 
                                           -
00 52 11 04 80 86 84 01 C4 
```

Fan level
```
From remote request 7th byte bit3-bit1  auto:0x010 med:011 high:110 low:101
From master status  7th byte bit7-bit5

                      *
 0  1  2  3  4  5  6  7  8  9 10 11 12
40 00 11 08 08 4C 14 1A 7A 00 33 33 69                 -> A 1010  auto
                      -                                      ---
00 FE 1C 0D 80 81 8D 4C 00 00 7A 00 33 33 01 00 01 55  -> 4 0100
                     -                                      ---
...
40 00 11 08 08 4C 14 1B 7A 00 33 33 68                 -> B 1011 medium
                      -                                      ---
00 FE 1C 0D 80 81 8D 6C 00 00 7A 00 33 33 01 00 01 75  -> 6 0110
                     -                                      ---
...
40 00 11 08 08 4C 14 1C 7A 00 33 33 6F                  -> A 1100 high
                      -                                       ---
00 FE 1C 0D 80 81 8D 8C 00 00 7A 00 33 33 01 00 01 95   -> 8 1000
                     -                                       ---
...

40 00 11 08 08 4C 14 1D 7A 00 33 33 6E                  -> D 1101   low
                      -                                       ---
00 FE 1C 0D 80 81 8D AC 00 00 7A 00 33 33 01 00 01 B5   -> A 1010
                     -                                       ---
...
??
40 00 15 07 08 0C 81 00 00 48 00 9F                     -> 
               
00 40 18 08 80 0C 00 03 00 00 48 00 97 

40 00 55 05 08 81 00 7C 00 E5 
00 FE 58 0F 80 81 8D AC 00 00 7A 7D E9 00 33 33 01 00 01 67

```


TEST, ON, HEAT, COOL, OFF, TEST
```
40 00 15 07 08 0C 81 00 00 48 00 9F
40 00 55 05 08 81 00 7C 00 E5
40 00 11 03 08 41 C0 DB  test on?    1100
40 00 11 03 08 41 03 18  power on
40 00 15 07 08 0C 81 00 00 48 00 9F
40 00 11 03 08 42 02 1A
40 00 55 05 08 81 00 7C 00 E5
40 00 11 03 08 41 02 19  power off
40 00 11 03 08 41 80 9B  test off?   1000
```

TEST + CL sensor inquiry
```
40 00 17 08 08 80 EF 00 2C 08 00 02 1E
                                 |----- sensor 2
00 40 1A 07 80 EF 80 00 2C 00 15 8B  
                              |--------- 0x15 -> 21

00 40 1A 05 80 EF 80 00 A2 12  answer for unknown sensor

40 00 17 08 08 80 EF 00 2C 08 00 F3 EF   F3 Filter sign time
00 40 1A 07 80 EF 80 00 2C 03 1E 83     0x03 0x1E->  798  2bytes
```

Timer
```
40 00 11 09 08 0c 82 00 00 30 05 01 01 eb   30m poweroff
40 00 11 09 08 0c 82 00 00 30 05 02 02 eb    2h poweroff
40 00 11 09 08 0c 82 00 00 30 00 04 01 eb   cancel timer                                  
40 00 11 09 08 0c 82 00 00 30 06 03 03 e8   1.5h poweroff repeat
40 00 11 09 08 0c 82 00 00 30 06 01 01 e8   30m  poweroff repeat
40 00 11 09 08 0c 82 00 00 30 07 30 30 e9   24h poweron
40 00 11 09 08 0c 82 00 00 30 07 04 04 e9    2h poweron

40 00 11 09 08 0c 82 00 00 30 07 02 02 e9    1h for poweron
                                    |----- number of 30 minutes
                                 |----- repeated
                              |------ 07 poweron   06 poweroff repeat 05 poweroff  00 cancel

```

Power on

```
40 00 11 03 08 41 03 18                                      Power on 
00 fe 1c 0d 80 81 35 ac 00 00 6c 00 55 55 01 00 01 1b        Normal status
00 52 11 04 80 86 24 01 64                                   Mode
00 fe 10 02 80 8a e6                                         Periodic ping
40 00 15 07 08 0c 81 00 00 48 00 9f                          ??
00 40 18 08 80 0c 00 03 00 00 48 00 97                       Answer to ??
40 00 55 05 08 81 00 65 00 fc                                Sensor temp
00 fe 58 0f 80 81 35 ac 00 00 6c 6f e9 00 55 55 01 00 01 db  Extended status
00 52 11 04 80 86 24 01 64

```
Power off

```
40 00 11 03 08 41 02 19                                      Power off
00 40 18 02 80 a1 7b                                         ACK after a command
00 fe 1c 0d 80 81 34 a8 00 00 6c 00 55 55 01 00 01 1e        Normal status
00 52 11 04 80 86 24 00 65                                   Mode
00 fe 10 02 80 8a e6                                         Periodic ping
40 00 15 07 08 0c 81 00 00 48 00 9f                          ??
00 40 18 08 80 0c 00 03 00 00 48 00 97                       Answer to ??
40 00 55 05 08 81 00 6a 00 f3                                Sensor temp
00 fe 58 0f 80 81 34 a8 00 00 6c 6c e9 00 55 55 01 00 01 dd  Extended status

```
TEST+SET for Error history
```
40 00 15 03 08 27 01 78           Error 1
00 40 18 05 80 27 08 00 48 ba     Type 0x4 Num error 0x8   E-08

40 00 15 03 08 27 02 7b           Error 2
00 40 18 05 80 27 08 00 43 b1     Type 0x4 Num error 0x3   E-03

40 00 15 03 08 27 03 7a           Error 3
00 40 18 05 80 27 00 00 00 fa     0x0 0x0   No error
```

```
00 40 18 02 80 A1 7B
00 40 18 08 80 0C 00 03 00 00 48 00 97
00 40 1A 07 80 EF 80 00 2C 00 00 9E
00 52 11 04 80 86 24 00 65
00 55 55 01 00 01
00 FE 10 02 80 8A E6
00 FE 1C 0D 80 81 34 A8 00 00 6C 00 55 55 01 00 01 1E
00 FE 58 0F 80 81 34 A8 00 00 6C 6D E9 00 55 55 01 00 01 DC
40 00 11 03 08 41 02 19
40 00 11 03 08 41 03 18
40 00 11 08 08 4C 09 1D 6C 00 05 05 65
40 00 11 09 08 0C 82 00 00 30 05 01 01 EB
40 00 15 07 08 0C 81 00 00 48 00 9F
40 00 17 08 08 80 EF 00 2C 08 00 02 1E
40 00 55 05 08 81 00 66 00 FF
```



# TO-DOS

- Improve PCB
- Fix PCB: route EN line and necessary stuff for ESP12X
- Fix PCB: jumper for Hardware or Software Serial

- Fix parsing to support round buffer and not to loose partial frames (not necessary)

- Redesign HTML page (WIP, fixed divs) 
https://mdbootstrap.com/snippets/jquery/ascensus/456902#html-tab-view   calculator
https://codepen.io/lalwanivikas/pen/eZxjqo  calculator
https://codepen.io/giana/pen/GJMBEv   calculator
https://codepen.io/CiTA/pen/OwowEB remote

- Simple decode example

- Clearer Protocol documentation

- MQTT and HA support

- Use DC buck/boost from A-B line to power ESP8266 (tried, but not working)

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

