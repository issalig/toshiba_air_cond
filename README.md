# toshiba_air_cond
Decode Toshiba A-B protocol (aka TCC Link)

TO-DOS

-Reconnect WebScokets when connection closes (WIP, parsing does not crash now)

-Redesign HTML page (WIP, fixed divs)

-Simple decode example

-Protocol documentation

-Use DC buck/boost from A-B line to power ESP8266 (tried, but not working)

-Guess timer opcodes if any (not sure if remote takes care of it internally)

-Check other circuits as https://easyeda.com/marcegli/door-opener https://frog32.ch/smart-intercom.html https://electronics.stackexchange.com/questions/458996/logic-level-converter-for-nodemcu-esp8266-input-24v-16v-hi-lo-500-baud https://sudonull.com/post/18480-We-pump-the-intercom-with-the-MQTT-protocol-to-control-from-the-phone https://hackaday.com/2019/01/07/building-an-esp8266-doorbell-on-hard-mode/
https://daeconsulting.co.za/2018/12/17/theres-someone-at-the-door/

```
Signal is around 15.6 volts when 1 and 14 when 0. Zener diode provides 13V reference, so signal is 1 .. 2.6 and after diode (0.7 drop) is 0.3 .. 1.9, enough to activate photodiode (1.2V) when 1 and to not activate it when 0.

Type     VZnom  IZT  for  rzjT    rzjk  at  IZK    IR  at  VR
1N4743A  13     19        <10     <100      0.25   <5      9.9

Izt=19 mA -> 2.6/19=130ohm  P=VI 2.6*19 =52mW


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

DS0138 analyzer can be used to monitor the signal and a logic analyzer to capture data in to computer.

To capture data pulseview with uart decoder 2400 bps, 8bits, start, stop, even parity

When validated visually you can use the following command line that reads RX data annotations and print one message per line according to 4th byte (message size).
```
sigrok-cli -P uart:rx=D0:baudrate=2400:parity_type=even -A uart=rx_data -i  YOURFILE  | awk '{pad =" "; b[len%4]=$2; if(len==3) {bytes="0x"b[len];  printf("%s%s%s%s%s%s%s%s",b[0],pad,b[1],pad,b[2],pad,b[3],pad)} if(len>3) {printf("%s%s",$2,pad);} len=len+1; if(len==4+bytes+1) {print "";len=0;bytes=0}}'
```

Data format seems  to be

|Source | Dest | UNK  | Bytes | Data | CRC |

Data 

| R/W | Operation Code | Params |

08  W?, 80 would be R

CRC is computed as Checksum8 XOR (Compute it at https://www.scadacore.com/tools/programming-calculators/online-checksum-calculator/)

Dest 00 is master, 40 is remote, FE is broadcast, 52 is ???

```
Op code from remote
4C 0C 1D  temp         40 00 11 08 08 4C 0C 1D 78 00 33 33 74
0C 81     status      
41        power        40 00 11 03 08 41 03 18
42        mode         40 00 11 03 08 42 02 1A //cool  02 -> 0000 0010
4C 14     fan          40 00 11 08 08 4C 14 1D 7A 00 33 33 6E  //low
54        save         40 00 11 04 08 54 01 00 08
0C 82     timer

Op code from master
81 status              00 FE 1C 0D 80 81 8D AC 00 00 7A 00 33 33 01 00 01 B5
8A ack (dest FE)       00 FE 10 02 80 8A E6 # every 5s
A1 ack (dest 40)       00 40 18 02 80 A1 7B
86 ?? (dest 52)        00 52 11 04 80 86 84 05 C0

UNK is
10 ack / ping??           00 FE 10 02 80 8A E6
1C status after request   00 FE 1C 0D 80 81 8D AC 00 00 7A 00 33 33 01 00 01 B5
58 ?? status when idle    00 FE 58 0F 80 81 8D AC 00 00 7A 84 E9 00 33 33 01 00 01 9E
18 ack (dest remote)      00 40 18 02 80 A1 7B 
11 ??                     40 00 11 03 08 41 03 18
                          00 52 11 04 80 86 84 05 C0
15 (from remote)          40 00 15 07 08 0C 81 00 00 48 00 9F
55 (from remote)          40 00 55 05 08 81 00 7E 00 E7 
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

When POWERED OFF
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
00 40 18 08 80 0C 00 03 00 00 48 00 97 #inmediately answer
40 00 55 05 08 81 00 7E 00 E7 
00 FE 58 0F 80 81 8D AC 00 00 7A 84 E9 00 33 33 01 00 01 9E
00 FE 10 02 80 8A E6 # typical answer, maybe confirmation
00 52 11 04 80 86 84 01 C4
00 FE 10 02 80 8A E6 # typical answer, maybe confirmation
00 FE 10 02 80 8A E6 # typical answer, maybe confirmation (every 5s we see  the mesg)
00 FE 10 02 80 8A E6 # typical answer, maybe confirmation (every 5s we see  the mesg)
40 00 15 07 08 0C 81 00 00 48 00 9F
00 40 18 08 80 0C 00 03 00 00 48 00 97 #inmediately answer
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

Temp up
```
40 00 11 08 08 4C 0C 1D 7A 00 33 33 76 # press temp up   (current 25, 26 after pressing)
00 40 18 02 80 A1 7B # typical answer, maybe confirmation
00 FE 1C 0D 80 81 8D AC 00 00 7A 00 33 33 01 00 01 B5 # looks like current status
00 52 11 04 80 86 84 05 C0 #
00 FE 10 02 80 8A E6 # typical answer, maybe confirmation
```

Temp down from 26 to 18
```
40 00 11 08 08 4C 0C 1D 78 00 33 33 74 # press temp down (current 26, 25 after pressing)
00 40 18 02 80 A1 7B # typical answer, maybe confirmation
00 FE 1C 0D 80 81 8D AC 00 00 78 00 33 33 01 00 01 B7 # looks like current status
00 52 11 04 80 86 84 05 C0 #this seq was in temp up ???

40 00 11 08 08 4C 0C 1D 76 00 33 33 7A # press temp down (current 25, 24 after pressing)
00 40 18 02 80 A1 7B
00 FE 1C 0D 80 81 8D AC 00 00 76 00 33 33 01 00 01 B9
00 52 11 04 80 86 84 05 C0 

40 00 11 08 08 4C 0C 1D 74 00 33 33 78 # press temp down 22 
00 40 18 02 80 A1 7B
00 FE 1C 0D 80 81 8D AC 00 00 74 00 33 33 01 00 01 BB
00 52 11 04 80 86 84 05 C0

40 00 11 08 08 4C 0C 1D 72 00 33 33 7E # press temp down 21
00 40 18 02 80 A1 7B
00 FE 1C 0D 80 81 8D AC 00 00 72 00 33 33 01 00 01 BD
00 52 11 04 80 86 84 05 C0

40 00 11 08 08 4C 0C 1D 70 00 33 33 7C # press temp down 20
...
40 00 11 08 08 4C 0C 1D 6E 00 33 33 62 # press temp down 19
...
40 00 11 08 08 4C 0C 1D 6C 00 33 33 60 # press temp down 18
...
40 00 15 07 08 0C 81 00 00 48 00 9F # press temp up 19  !!!! 
00 40 18 08 80 0C 00 03 00 00 48 00 97 

40 00 11 08 08 4C 0C 1D 6C 00 33 33 60 # press temp up 20
00 40 18 02 80 A1 7B
00 52 11 04 80 86 84 05 C0
00 FE 1C 0D 80 81 8D AC 00 00 6A 00 33 33 01 00 01 A5
00 FE 10 02 80 8A E6

40 00 55 05 08 81 00 7C 00 E5 # press temp up 21
00 FE 58 0F 80 81 AC 00 00 6A 7F E9 00 33 33 01 00 01 75
00 52 11 04 80 86 84 05 C0

40 00 11 08 08 4C 0C 1D 6C 00 33 33 60 # press temp up 22
...
40 00 11 08 08 4C 0C 1D 6E 00 33 33 62 # press temp up 23
...
40 00 11 08 08 4C 0C 1D 70 00 33 33 7C # press temp up 24
...
40 00 11 08 08 4C 0C 1D 72 00 33 33 7E # press temp up 25
...
40 00 11 08 08 4C 0C 1D 70 00 33 33 7C # press temp up 26


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

from pg70 http://cyme.com.mx/equipos/sistema_vrf/carrier/Aplication%20Controls%20Manual-A04-007.pdf 
0x00=unfix,0x 01= heat,0x 02= cool,0x 03= dry 0x 04= fan,0x05 auto 

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

Timer off
```
40 00 11 09 08 0C 82 00 00 30 05 01 01 EB 
```

Format seems this
Source Dest XX Bytes Data CRC
CRC is computed as Checksum8 XOR
Data starts with 08 if Source is 40 and 80 if source is 00
```
uint8_t XORChecksum8(const byte *data, size_t dataLength)
{
  uint8_t value = 0;
  for (size_t i = 0; i < dataLength; i++)
  {
    value ^= (uint8_t)data[i];
  }
  return ~value;
}
```

Info about commercial gateways but no info about protocol :(

Connections https://www.toshibaclim.com/Portals/0/Documentation/Manuels%20produits/SM_CassetteUTP_DI-SDI-111416-E_GB.pdf
Some info o features  pg52 http://www.toshiba-aircon.co.uk/assets/uploads/pdf/sales_tools/New_Technical_Handbook_version_14_1_3.pdf


Error codes from Toshiba (pg 38) https://cdn.shopify.com/s/files/1/1144/2302/files/BP-STD_Toshiba_v1_08.pdf
TCS-Net https://www.toshibaheatpumps.com/application/files/8914/8124/4818/Owners_Manual_-_Modbus_TCB-IFMB640TLE_E88909601.pdf
https://www.intesisbox.com/intesis/product/media/intesisbox_to-ac-knx-16-64_user_manual_en.pdf?v=2.2

Plan B (in fact it was plan A but then I managed to decode AB protocol)

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




