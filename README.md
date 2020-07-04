# toshiba_air_cond
Decode toshiba 2wire ab  signal

Signal is around 15.6 volts when 1 and 14 when 0. Zener provides 13 volt reference, so signal is 2.6 .. 1 and after diode (0.7) 1.9 0.3, enough to activate photodiode (1.2) when 1 and to not activate it when 0.


```
                              diode  
  A --------------------200R ->|--|
               10k                |-|-------|-------------3v3
                |                   | PC817 |
                --------------------|-------|--|--------- OUT               
                |                              |
               / \ zener 13v                  1k
                |                              |
                |                             GND
  B -------------
             

```

DS0138 analyzer can be used to monitor the signal and a logic analyzer to capture data in to computer.

Use pulseview uart decoder 2400 bps, 8bits, start, stop, even parity

When validated visually you can use the following command line that reads RX data annotations and print one message per line according to 4th byte (message size).
```
sigrok-cli -P uart:rx=D0:baudrate=2400:parity_type=even -A uart=rx_data -i  YOURFILE  | awk '{pad =" "; b[len%4]=$2; if(len==3) {bytes="0x"b[len];  printf("%s%s%s%s%s%s%s%s",b[0],pad,b[1],pad,b[2],pad,b[3],pad)} if(len>3) {printf("%s%s",$2,pad);} len=len+1; if(len==4+bytes+1) {print "";len=0;bytes=0}}'
```

Format seems  to be
|Source | Dest | XX  | Bytes | Data | CRC |

CRC is computes as Checksum8 XOR (Compute it at https://www.scadacore.com/tools/programming-calculators/online-checksum-calculator/)

Dest 00 is master, 40 is remote, FE is broadcast, 52 is ???

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
40 00 11 08 08 4C 0C 1D 6E 00 33 33 62 //20
40 00 11 08 08 4C 0C 1D 6C 00 33 33 60 //19
40 00 15 07 08 0C 81 00 00 48 00 9F    //unexpected
40 00 11 08 08 4C 0C 1D 6A 00 33 33 66 //18
40 00 55 05 08 81 00 7C 00 E5          //unexpected
40 00 11 08 08 4C 0C 1D 6C 00 33 33 60 //19
40 00 11 08 08 4C 0C 1D 6E 00 33 33 62 //20

7A 011-1 101-0 //set target temp to 26 -> 1101 -> 13
78 011-1 100-0 //set target temp to 25 -> 1100 -> 12
70 011-1 000-0 //set target temp to 21 -> 1000 ->  8
6E 011-0 111-0 //set target temp to 20 -> 0111 ->  7
6C 011-0 110-0 //set target temp to 19 -> 0110 ->  6
6A 011-0 101-0 //set target temp to 18 -> 0101 ->  5

Thus temp is value + 13

```

Power ON
```
40 00 11 03 08 41 03 18  # press ON
00 40 18 02 80 A1 7B # typical answer, maybe confirmation
00 FE 1C 0D 80 81 8D AC 00 00 7A 00 33 33 01 00 01 B5 # looks like current status
00 FE 10 02 80 8A E6 # typical answer, maybe confirmation
00 52 11 04 80 86 84 05 C0 #
```

Power OFF
```
40 00 11 03 08 41 02 19  # press OFF
00 40 18 02 80 A1 7B # typical answer, maybe confirmation
00 FE 1C 0D 80 81 8C A8 00 00 7A 00 33 33 01 00 01 B0 # looks like current status
00 FE 10 02 80 8A E6 # typical answer, maybe confirmation
00 52 11 04 80 86 84 00 C5 #
```

Modes
```
From remote (Mode is bit3-bit0 from last data byte)
40 00 11 03 08 42 02 1A //cool  02 -> 0000 0010
40 00 11 03 08 42 03 1B //fan   03 -> 0000 0011
40 00 11 03 08 42 05 1D //auto  05 -> 0000 0101
40 00 15 02 08 42 1D    //???
40 00 11 03 08 42 01 19 //heat  01 -> 0000 0001
40 00 11 03 08 42 04 1C //dry   04 -> 0000 0100


Full log (Mode in master is bit3-bit1 from byte 6 (starting from 0)
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
00 52 11 04 80 86 64 01 24                               // 64 -> 0110 0100

auto
40 00 11 03 08 42 05 1D                                  //auto  05 -> 0000 0101
00 40 18 02 80 A1 7B                                                         ---
40 00 15 02 08 42 1D 
00 40 18 04 80 42 05 06 9D                               //requested 05, assigned 06??
00 FE 1C 0D 80 81 CD 8C 00 00 76 00 33 33 01 00 01 D9    // CD 8C 00 -> 1100 1101 1000 1100
00 FE 10 02 80 8A E6                                                    ---         -
00 52 11 04 80 86 C4 01 84                               // C4 -> 1100 0100
                                                                  ---
heat
40 00 11 03 08 42 01 19                                  //heat  01 -> 0000 0001
00 40 18 02 80 A1 7B                                                         ---
00 FE 1C 0D 80 81 35 AC 02 00 76 00 55 55 01 00 01 03    // 35 AC 02 -> 0011 0101 1010 1100 0000 0010   // 55 55 vs 33 33 in other modes
                                                                        ---                        -       -- -- 
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

Fan mode
```
Master 7th byte bit3-bit1

40 00 11 08 08 4C 14 1A 7A 00 33 33 69                 -> A 1010
                      -                                      ---
00 FE 1C 0D 80 81 8D 4C 00 00 7A 00 33 33 01 00 01 55  -> 4 0100
                     -                                      ---
---
40 00 11 08 08 4C 14 1B 7A 00 33 33 68                 -> B 1011
                      -                                      ---
00 FE 1C 0D 80 81 8D 6C 00 00 7A 00 33 33 01 00 01 75  -> 6 0110
                     -                                      ---
---
40 00 11 08 08 4C 14 1C 7A 00 33 33 6F                  -> A 1100
                      -                                       ---
00 FE 1C 0D 80 81 8D 8C 00 00 7A 00 33 33 01 00 01 95   -> 8 1000
                     -                                       ---
--

40 00 11 08 08 4C 14 1D 7A 00 33 33 6E                  -> D 1101   level 1
                      -                                       ---
00 FE 1C 0D 80 81 8D AC 00 00 7A 00 33 33 01 00 01 B5   -> A 1010
                     -                                       ---
--
??
40 00 15 07 08 0C 81 00 00 48 00 9F                     -> 
               
00 40 18 08 80 0C 00 03 00 00 48 00 97 

40 00 55 05 08 81 00 7C 00 E5 
00 FE 58 0F 80 81 8D AC 00 00 7A 7D E9 00 33 33 01 00 01 67

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

