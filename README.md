# toshiba_air_cond
Decode toshiba 2wire ab  signal

Signal is around 15.6 volts when 1 and 14 when 0. Zener provides 13 volt reference, so signal is 2.6 .. 1 and after diode (0.7) 1.9 0.3, enough to activate photodiode (1.2) when 1 and to not activate it when 0.


'''
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
             

'''

DS0138 analyzer can be used to monitor the signal and a logic analyzer to capture data in to computer.

Use pulseview uart decoder 2400 bps, 8bits, start, stop, even parity

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


78 0111 1000 //set target temp to 25 11001 ???
7A 0111 1010 //set target temp to 26 11010

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

Format seems this
Source Dest XX Bytes Data CRC
CRC is computed as Checksum8 XOR
Data starts with 08 if Source is 40 and 80 if source is 00

uint8_t XORChecksum8(const byte *data, size_t dataLength)
{
  uint8_t value = 0;
  for (size_t i = 0; i < dataLength; i++)
  {
    value ^= (uint8_t)data[i];
  }
  return ~value;
}


