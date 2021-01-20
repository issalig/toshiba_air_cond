# toshiba_air_cond
Decode Toshiba AB protocol (TCC Link??) for air conditioners with wired controllers.

Tested with remote control unit RBC-AMT32E and central unit RAV-SM406BTP-E (http://www.toshiba-aircon.co.uk/assets/uploads/product_assets/20131115_IM_1115460101_Standard_Duct_RAV-SM_6BTP-E_EN.pdf)

Service manual http://www.toshibaclim.com/Portals/0/Documentation/Manuels%20produits/SM_Gainable_Std-Compact--DI_406566806110614061606_GB.pdf

```
RAV-SM406BTP-E 
|     | |    |- CE marking
|     | ||-Duct
|     | |-gen
|     |-duty 4.0 kW 
|   |-Digital inverter
|- Light comercial
```

https://rednux.com/mediafiles/Hersteller/toshiba/Toshiba-Bedienungsanleitung-RBC-AMT32E-Englisch.pdf

Error codes and sensor addresses
http://www.toshiba-aircon.co.uk/assets/uploads/pdf/sales_tools/Technical_Handbook_ver._13.1.pdf
https://www.cdlweb.info/wp-content/uploads/2020/10/1-CDL-Toshiba-R32-Technical-Handbook-V10-2020.pdf
https://www.toshibaclim.com/Portals/0/Documentation/Manuels%20produits/SM_CassetteUTP_DI-SDI-111416-E_GB.pdf


Interesting project with similar protocol for heat equipment https://github.com/H4jen/webasto_sniffer

https://echonet.jp/wp/wp-content/uploads/pdf/General/Standard/Release/Release_F_en/SpecAppendixF_e.pdf


# Status

-Operational.


# TO-DOS

- Fix PCB: mirror transistor footprint, route EN line and necessary stuff for ESP12X
- Fix PCB: jumper for Hardware or Software Serial

- Fix parsing to support round buffer and not to loose partial frames (not necessary)

- Redesign HTML page (WIP, fixed divs)

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

# Data acquisition

DS0138 oscilloscope can be used to monitor the signal (voltage differs around 0.7V but it is usable) and guess voltages and bps. Later, an 8-channel USB logic analyzer (4-5 USD) can be used to capture data into the computer. REMEMBER to convert voltages to 0-3.3v before connecting it to logic analyzer or you will fry it. You can use the read circuit below.

To capture data you can use pulseview with uart decoder 2400 bps, 8bits, start, stop, EVEN parity

When the data has been validated visually you can use the following command line that reads RX data annotations and print one message per line according to 4th byte (message size).

In case you need it you can install the following packages
```
sudo apt install sigrok-cli
sudo apt install sigrok-firmware-fx2lafw
```

```
sigrok-cli -P uart:rx=D0:baudrate=2400:parity_type=even -A uart=rx_data -i  YOURFILE  | awk '{pad =" "; b[len%4]=$2; if(len==3) {bytes="0x"b[len];  printf("%s%s%s%s%s%s%s%s",b[0],pad,b[1],pad,b[2],pad,b[3],pad)} if(len>3) {printf("%s%s",$2,pad);} len=len+1; if(len==4+bytes+1) {print "";len=0;bytes=0}}'
```

sigrok-cli -d fx2lafw -c samplerate=250000 -t D0=r -P uart:rx=D0:baudrate=2400:parity_type=even  -A uart=rx_data --continuous

# Custom hardware
https://learnabout-electronics.org/Semiconductors/opto_52.php

Circuits have been designed to read and write the signal

```
Air conditioning side:
Signal is around 15.6 volts when 1 and 14 when 0. Zener diode provides 13V reference, so signal is 1V .. 2.6V and after diode (0.7V drop) is 0.3V .. 1.9V, enough to activate photodiode (1.2V) when 1 and to not activate it when 0.

Led drops 1.2v, and from signal we have a difference of 15.6-13=2.6, thus 2.6-1.2=1.4/100= 14mA which has a maximum CTR=140%
Ic=3.3, If=14
CTR=Ic/If

Type     VZnom  IZT  for  rzjT    rzjk  at  IZK    IR  at  VR
1N4743A  13     19        <10     <100      0.25   <5      9.9

Izt=19 mA -> 2.6/19=130ohm  P=VI 2.6*19 =52mW


Microcontroller side: 1k resistor limits the current. ESP8266 max current is 12mA > 3.3/1k = 3.3 mA

Read schematic
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
             
Write circuit performs similarly to read circuit. When OUT signal is 1, transistor and pullup resistor are 0, thus optocoupler is OFF and voltage is 15.6 (HIGH). When OUT signal is 0, transistor is off and pullup resistor sends 1 and activates optocoupler and zener diode gives 13V (LOW).
Some systems recommend to set the Follower in the remote unit.


Write schematic (use under your own risk)
https://www.onsemi.com/pub/Collateral/P2N2222A-D.PDF 
https://learnabout-electronics.org/Downloads/PC817%20optocoupler.pdf


              3v3                                   1N4001
               |                             |--------<------- A               
               200                _______    |     |               
               ------------------|       |---|     ^ z13v
               |                 | PC817 |         |
 OUT --1k- ---|<  2N2222     |---|_______|---1k---|< 2N2222
               |             |                     |
               |             |                     |
              GND           GND                    ------------ B
  
  
                                      |



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

Data packages have the following format:

|Source | Dest | Opcode1  | Data Length | Data | CRC |
|---|---|---|---|---|---|


Source (1 byte): 
- 00 is master
- 40 is remote controller
- FE is broadcast
- 52 is ??

Dest (1 byte)
- Same as source

Operation code 1 (1 byte) 
- From master (00)
  |Opc1|Desc|Example|
  |---|---|---|
  |10| ping|    00 FE 10 02 80 8A E6|
  |11| parameters (temp. power, mode, fan, save)| 00 52 11 04 80 86 84 01 C4| 
  |1A| sensor value| 00 40 1A 07 80 EF 80 00 2C 00 2B B5|
  |1C| status |00 FE 1C 0D 80 81 8D AC 00 00 76 00 33 33 01 00 01 B9|
  |58| extended status | |
  |18| ack |00 40 18 08 80 0C 00 03 00 00 48 00 97|
  |18| ping |00 40 18 02 80 A1 7B|
- From remote (40)
  |Opc1|Desc|Example|
  |---|---|---|
  |11|  set parameters (temp. power, mode, fan, save) |  40 00 11 03 08 41 03 18 |
  ||  set parameters      | 40 00 11 08 08 4C 09 1D 6C 00 05 05 65|
  |15| Error history   | 40 00 15 07 08 0C 81 00 00 48 00 9F  |
  |17|   sensor query   | 40 00 17 08 08 80 EF 00 2C 08 00 02 1E |
  | 55        temperature|    40 00 55 05 08 81 00 69 00 F0 |
  

Data length (1 byte)
- Length of data field

Data (composed of the following parts)


| R/W mode | Operation Code 2 | Payload |
|---|---|---|

R/W mode (1 byte)
|Mode|Desc|
|---|---|
|08 |for Write mode (from remote 40)|
|80 |for Read mode (from master 00)|

Opcode2

  - From master (00)
  
    | Opcode2 | Desc | Example |
    |---|---|---|
    | 81 | status (opc1 55 | 00 FE 1C 0D 80 81 34 A8 00 00 6C 00 55 55 01 00 01 1E
    | 81 | extended status (opc1 58) | 00 FE 58 0F 80 81 35 AC 02 00 6E 6F E9 00 55 55 01 00 01 DB|
    | 8A | ack  (Dest FE) |00 FE 10 02 80 8A E6|
    | A1 | ping (Dest 40) |00 40 18 02 80 A1 7B|
    | 86 | mode (Dest 52) |00 52 11 04 80 86 24 05 60| 
    | 0C | 00 (answer to 0C from master) | 00 40 18 08 80 0c 00 03 00 00 48 00 97 | 

  - From remote (40)
  
  
    | Opcode2 | Desc | Example |
    |---|---|---|
    | 41 |power|40 00 11 03 08 41 02 19|
    | 42 |mode|40 00 11 03 08 42 02 1A|
    | 4C |temp, fan|40 00 11 08 08 4C 11 1A 6E 00 55 55 78|
    | 54 |save (opc1 11)|40 00 11 04 08 54 01 00 08 |
    | 80 |sensor query|40 00 17 08 08 80 EF 00 2C 08 00 02 1E|
    | 81 |sesnor temp |40 00 55 05 08 81 00 6A 00 F3|
    | 0C 00 ||40 00 18 08 80 0C 00 03 00 00 48 00 97|
    | 0C 81 |status/ping |40 00 15 07 08 0C 81 00 00 48 00 9F|
    | 0C 82 |timer |40 00 11 09 08 0C 82 00 00 30 05 01 01 EB|
    
    

CRC is computed as Checksum8 XOR of all the bytes (Compute it at https://www.scadacore.com/tools/programming-calculators/online-checksum-calculator/)

# Message types

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

Ping message (sent every 5 seconds)
```
00 fe 10 02 80 8a e6 
|  |  |  |  |  |  |- CRC
|  | OPC1|  |  |- OPC2 
|  ALL   |  |-from master, info message??
Master   |- Length

From master (00) to all (fe)

```

Temp from remote to master
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

Timer (it is not working, there should miss some other commands)

```
40 00 11 09 08 0c 82 00 00 30 07 02 02 e9    1h for poweron
                                    |----- number of 30 minutes
                                 |----- repeated
                              |------ 07 poweron   06 poweroff repeat 05 poweroff  00 cancel
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


# Notes from logs (used for the above info)

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


# Other info

Info about commercial gateways but no info about protocol :(

Connections https://www.toshibaclim.com/Portals/0/Documentation/Manuels%20produits/SM_CassetteUTP_DI-SDI-111416-E_GB.pdf
Sensor addresses (pg 52) http://www.toshiba-aircon.co.uk/assets/uploads/pdf/sales_tools/New_Technical_Handbook_version_14_1_3.pdf
Sensor addresses (pg 73) https://www.toshibaclim.com/Portals/0/Documentation/Manuels%20produits/SM_CassetteUTP_DI-SDI-111416-E_GB.pdf
Sensor addresses (pg 42) http://www.toshiba-aircon.co.uk/assets/uploads/pdf/sales_tools/Technical_Handbook_ver._13.1.pdf

Error codes from Toshiba (pg 38) https://cdn.shopify.com/s/files/1/1144/2302/files/BP-STD_Toshiba_v1_08.pdf
Temperature formula TCS-Net https://www.toshibaheatpumps.com/application/files/8914/8124/4818/Owners_Manual_-_Modbus_TCB-IFMB640TLE_E88909601.pdf
https://www.toshibaheatpumps.com/customer-support/owner-manuals
https://www.intesisbox.com/intesis/product/media/intesisbox_to-ac-knx-16-64_user_manual_en.pdf?v=2.2


