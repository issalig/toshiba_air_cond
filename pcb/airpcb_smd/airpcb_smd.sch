EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Toshiba AB Wifi Controller"
Date "2021-03-16"
Rev "2"
Comp "issalig"
Comment1 "github.com/issalig/toshiba_air_cond"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Device:R R2
U 1 1 5F560E22
P 1880 1750
F 0 "R2" H 1950 1796 50  0000 L CNN
F 1 "10k" H 1950 1705 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 1810 1750 50  0001 C CNN
F 3 "~" H 1880 1750 50  0001 C CNN
	1    1880 1750
	1    0    0    -1  
$EndComp
$Comp
L Device:R R3
U 1 1 5F561166
P 2030 1600
F 0 "R3" V 1823 1600 50  0000 C CNN
F 1 "100" V 1914 1600 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 1960 1600 50  0001 C CNN
F 3 "~" H 2030 1600 50  0001 C CNN
	1    2030 1600
	0    1    1    0   
$EndComp
$Comp
L Diode:1N4001 D2
U 1 1 5F563589
P 2330 1600
F 0 "D2" H 2330 1817 50  0000 C CNN
F 1 "1N4001" H 2330 1726 50  0000 C CNN
F 2 "Diode_SMD:D_SMA" H 2330 1425 50  0001 C CNN
F 3 "http://www.vishay.com/docs/88503/1n4001.pdf" H 2330 1600 50  0001 C CNN
	1    2330 1600
	-1   0    0    -1  
$EndComp
$Comp
L Isolator:PC817 U1
U 1 1 5F566455
P 2780 1700
F 0 "U1" H 2780 2025 50  0000 C CNN
F 1 "PC817" H 2780 1934 50  0000 C CNN
F 2 "Package_DIP:DIP-4_W7.62mm" H 2580 1500 50  0001 L CIN
F 3 "http://www.soselectronic.cz/a_info/resource/d/pc817.pdf" H 2780 1700 50  0001 L CNN
	1    2780 1700
	1    0    0    -1  
$EndComp
$Comp
L Device:R R5
U 1 1 5F567057
P 3080 1950
F 0 "R5" H 3150 1996 50  0000 L CNN
F 1 "1k" H 3150 1905 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 3010 1950 50  0001 C CNN
F 3 "~" H 3080 1950 50  0001 C CNN
	1    3080 1950
	1    0    0    -1  
$EndComp
$Comp
L Device:D_Zener D1
U 1 1 5F5679E8
P 1730 1900
F 0 "D1" H 1730 2117 50  0000 C CNN
F 1 "13" H 1730 2026 50  0000 C CNN
F 2 "Diode_SMD:D_MiniMELF_Handsoldering" H 1730 1900 50  0001 C CNN
F 3 "~" H 1730 1900 50  0001 C CNN
	1    1730 1900
	-1   0    0    -1  
$EndComp
Wire Wire Line
	2480 1800 2480 1900
Wire Wire Line
	2480 1900 1880 1900
Connection ~ 1880 1900
$Comp
L power:GND #PWR03
U 1 1 5F569069
P 3080 2100
F 0 "#PWR03" H 3080 1850 50  0001 C CNN
F 1 "GND" H 3085 1927 50  0000 C CNN
F 2 "" H 3080 2100 50  0001 C CNN
F 3 "" H 3080 2100 50  0001 C CNN
	1    3080 2100
	1    0    0    -1  
$EndComp
Wire Wire Line
	1880 1600 1430 1600
Connection ~ 1880 1600
Wire Wire Line
	1430 1700 1430 1900
Wire Wire Line
	1430 1900 1580 1900
Text GLabel 3330 1600 2    50   Input ~ 0
3V3
Wire Wire Line
	3330 1600 3080 1600
Text GLabel 1430 1700 0    50   Input ~ 0
B
Text GLabel 1430 1600 0    50   Input ~ 0
A
Wire Wire Line
	3080 1800 3330 1800
Connection ~ 3080 1800
Text GLabel 3330 1800 2    50   Input ~ 0
D7
Text Notes 1250 1050 0    50   ~ 0
Read circuit
Text Notes 1300 3050 0    50   ~ 0
Write circuit
Wire Notes Line
	1000 750  1000 2450
Wire Notes Line
	4040 2445 4040 745 
Wire Notes Line
	1050 2750 4050 2750
Wire Notes Line
	4050 2750 4050 4650
Wire Notes Line
	4050 4650 1050 4650
Wire Notes Line
	1050 4650 1050 2750
$Comp
L MCU_Module:WeMos_D1_mini U3
U 1 1 5F5DC37A
P 9755 2320
F 0 "U3" H 9755 1220 50  0000 C CNN
F 1 "WeMos_D1_mini" H 9755 1120 50  0000 C CNN
F 2 "Module:WEMOS_D1_mini_light" H 9755 1170 50  0001 C CNN
F 3 "https://wiki.wemos.cc/products:d1:d1_mini#documentation" H 7905 1170 50  0001 C CNN
	1    9755 2320
	1    0    0    -1  
$EndComp
Text GLabel 9855 1520 1    50   Input ~ 0
3V3
Text GLabel 10155 2220 2    50   Input ~ 0
D3
Text GLabel 10155 2720 2    50   Input ~ 0
D8
$Comp
L power:GND #PWR011
U 1 1 5F5DE558
P 9755 3120
F 0 "#PWR011" H 9755 2870 50  0001 C CNN
F 1 "GND" H 9760 2947 50  0000 C CNN
F 2 "" H 9755 3120 50  0001 C CNN
F 3 "" H 9755 3120 50  0001 C CNN
	1    9755 3120
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J4
U 1 1 5F5E144A
P 5795 955
F 0 "J4" H 5875 947 50  0000 L CNN
F 1 "AB" H 5875 856 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x02_P2.54mm_Vertical" H 5795 955 50  0001 C CNN
F 3 "~" H 5795 955 50  0001 C CNN
	1    5795 955 
	1    0    0    -1  
$EndComp
Text GLabel 5595 955  0    50   Input ~ 0
A
Text GLabel 5595 1055 0    50   Input ~ 0
B
Text GLabel 9655 1520 1    50   Input ~ 0
5V
$Comp
L Connector_Generic:Conn_01x04 J2
U 1 1 5F5E8FD1
P 5105 2750
F 0 "J2" H 5185 2742 50  0000 L CNN
F 1 "DHT" H 5185 2651 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x04_P2.54mm_Vertical" H 5105 2750 50  0001 C CNN
F 3 "~" H 5105 2750 50  0001 C CNN
	1    5105 2750
	1    0    0    -1  
$EndComp
Text GLabel 4455 2650 0    50   Input ~ 0
3V3
Text GLabel 4455 2850 0    50   Input ~ 0
D3
NoConn ~ 4905 2850
$Comp
L power:GND #PWR05
U 1 1 5F5EE6BC
P 4905 2950
F 0 "#PWR05" H 4905 2700 50  0001 C CNN
F 1 "GND" H 4910 2777 50  0000 C CNN
F 2 "" H 4905 2950 50  0001 C CNN
F 3 "" H 4905 2950 50  0001 C CNN
	1    4905 2950
	1    0    0    -1  
$EndComp
$Comp
L Device:R R7
U 1 1 5F5F1CC3
P 4605 2750
F 0 "R7" V 4398 2750 50  0000 C CNN
F 1 "10k" V 4489 2750 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 4535 2750 50  0001 C CNN
F 3 "~" H 4605 2750 50  0001 C CNN
	1    4605 2750
	0    1    1    0   
$EndComp
Wire Wire Line
	4905 2650 4455 2650
Wire Wire Line
	4455 2650 4455 2750
Wire Wire Line
	4755 2750 4905 2750
Wire Wire Line
	4455 2850 4755 2850
Wire Wire Line
	4755 2850 4755 2750
Connection ~ 4755 2750
$Comp
L Connector_Generic:Conn_01x04 J1
U 1 1 5F5F93C3
P 4805 3650
F 0 "J1" H 4885 3642 50  0000 L CNN
F 1 "SPI" H 4885 3551 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x04_P2.54mm_Vertical" H 4805 3650 50  0001 C CNN
F 3 "~" H 4805 3650 50  0001 C CNN
	1    4805 3650
	1    0    0    -1  
$EndComp
Text GLabel 4605 3550 0    50   Input ~ 0
3V3
$Comp
L power:GND #PWR04
U 1 1 5F5F9DA7
P 4605 3650
F 0 "#PWR04" H 4605 3400 50  0001 C CNN
F 1 "GND" V 4610 3522 50  0000 R CNN
F 2 "" H 4605 3650 50  0001 C CNN
F 3 "" H 4605 3650 50  0001 C CNN
	1    4605 3650
	0    1    1    0   
$EndComp
Text GLabel 4605 3750 0    50   Input ~ 0
SCL
Text GLabel 4605 3850 0    50   Input ~ 0
SDA
Text GLabel 10155 2020 2    50   Input ~ 0
SCL
Text GLabel 10155 2120 2    50   Input ~ 0
SDA
Wire Notes Line
	4205 750  6255 750 
Wire Notes Line
	6255 750  6255 4050
Wire Notes Line
	6255 4050 4205 4050
Wire Notes Line
	4205 4050 4205 750 
Text Notes 4350 950  0    50   ~ 0
Connectors
Text GLabel 9355 1920 0    50   Input ~ 0
RST
Text GLabel 10155 1820 2    50   Input ~ 0
A0
Text GLabel 10155 1920 2    50   Input ~ 0
D0
Text GLabel 10155 2420 2    50   Input ~ 0
D5
Text GLabel 10155 2520 2    50   Input ~ 0
D6
Text GLabel 10155 2320 2    50   Input ~ 0
D4
$Comp
L RF_Module:ESP-12E U4
U 1 1 5F622C8F
P 9855 5265
F 0 "U4" H 9855 6246 50  0000 C CNN
F 1 "ESP-12E" H 9855 6155 50  0000 C CNN
F 2 "RF_Module:ESP-12E" H 9855 5265 50  0001 C CNN
F 3 "http://wiki.ai-thinker.com/_media/esp8266/esp8266_series_modules_user_manual_v1.1.pdf" H 9505 5365 50  0001 C CNN
	1    9855 5265
	1    0    0    -1  
$EndComp
Text GLabel 9255 4665 0    50   Input ~ 0
RST
Text GLabel 9355 2320 0    50   Input ~ 0
ESP_TX
Text GLabel 9355 2220 0    50   Input ~ 0
ESP_RX
Text GLabel 10455 4765 2    50   Input ~ 0
ESP_TX
Text GLabel 10455 4965 2    50   Input ~ 0
ESP_RX
Text GLabel 9855 4465 2    50   Input ~ 0
3V3
Text GLabel 10455 5165 2    50   Input ~ 0
SCL
Text GLabel 10455 5065 2    50   Input ~ 0
SDA
Text GLabel 10455 4665 2    50   Input ~ 0
D3
Text GLabel 10455 4865 2    50   Input ~ 0
D4
Text GLabel 10455 5665 2    50   Input ~ 0
D0
Text GLabel 10455 5265 2    50   Input ~ 0
D6
Text GLabel 10455 5365 2    50   Input ~ 0
D7
Text GLabel 10455 5465 2    50   Input ~ 0
D5
Text GLabel 10455 5565 2    50   Input ~ 0
D8
Text GLabel 9255 5065 0    50   Input ~ 0
A0
$Comp
L Connector_Generic:Conn_01x03 J7
U 1 1 5F6377E8
P 5755 3700
F 0 "J7" H 5835 3742 50  0000 L CNN
F 1 "PWR" H 5835 3651 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x03_P2.54mm_Vertical" H 5755 3700 50  0001 C CNN
F 3 "~" H 5755 3700 50  0001 C CNN
	1    5755 3700
	1    0    0    -1  
$EndComp
Text GLabel 5555 3600 0    50   Input ~ 0
5V
Text GLabel 5555 3700 0    50   Input ~ 0
3V3
$Comp
L power:GND #PWR07
U 1 1 5F638CFB
P 5555 3800
F 0 "#PWR07" H 5555 3550 50  0001 C CNN
F 1 "GND" V 5560 3672 50  0000 R CNN
F 2 "" H 5555 3800 50  0001 C CNN
F 3 "" H 5555 3800 50  0001 C CNN
	1    5555 3800
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR012
U 1 1 5F63CC7E
P 9855 5965
F 0 "#PWR012" H 9855 5715 50  0001 C CNN
F 1 "GND" H 9860 5792 50  0000 C CNN
F 2 "" H 9855 5965 50  0001 C CNN
F 3 "" H 9855 5965 50  0001 C CNN
	1    9855 5965
	1    0    0    -1  
$EndComp
NoConn ~ 9255 5465
NoConn ~ 9255 5565
NoConn ~ 9255 5765
NoConn ~ 9255 4865
$Comp
L power:VCC #PWR010
U 1 1 5F6466FB
P 7625 3510
F 0 "#PWR010" H 7625 3360 50  0001 C CNN
F 1 "VCC" V 7640 3637 50  0000 L CNN
F 2 "" H 7625 3510 50  0001 C CNN
F 3 "" H 7625 3510 50  0001 C CNN
	1    7625 3510
	0    -1   -1   0   
$EndComp
Text GLabel 7625 3510 2    50   Input ~ 0
5V
NoConn ~ 9255 5265
NoConn ~ 9255 5365
NoConn ~ 9255 5665
$Comp
L power:GND #PWR01
U 1 1 5F64154F
P 2050 4100
F 0 "#PWR01" H 2050 3850 50  0001 C CNN
F 1 "GND" H 2055 3927 50  0000 C CNN
F 2 "" H 2050 4100 50  0001 C CNN
F 3 "" H 2050 4100 50  0001 C CNN
	1    2050 4100
	1    0    0    -1  
$EndComp
Text GLabel 1450 3900 0    50   Input ~ 0
D8
Connection ~ 2050 3700
Text GLabel 2050 3400 1    50   Input ~ 0
3V3
$Comp
L Transistor_BJT:PN2222A Q1
U 1 1 5F584BBB
P 1950 3900
F 0 "Q1" H 2140 3946 50  0000 L CNN
F 1 "PN2222A" H 2140 3855 50  0000 L CNN
F 2 "airpcb_smd:SOT-23_Reverse_Handsoldering" H 2150 3825 50  0001 L CIN
F 3 "http://www.fairchildsemi.com/ds/PN/PN2222A.pdf" H 1950 3900 50  0001 L CNN
	1    1950 3900
	1    0    0    -1  
$EndComp
Wire Wire Line
	2050 3700 2600 3700
$Comp
L power:GND #PWR02
U 1 1 5F581A59
P 2600 3900
F 0 "#PWR02" H 2600 3650 50  0001 C CNN
F 1 "GND" H 2605 3727 50  0000 C CNN
F 2 "" H 2600 3900 50  0001 C CNN
F 3 "" H 2600 3900 50  0001 C CNN
	1    2600 3900
	1    0    0    -1  
$EndComp
Text GLabel 3800 4400 2    50   Input ~ 0
B
Wire Wire Line
	3500 4400 3800 4400
Text GLabel 3800 3700 2    50   Input ~ 0
A
Connection ~ 3500 3700
Wire Wire Line
	3500 3700 3200 3700
$Comp
L Diode:1N4001 D4
U 1 1 5F57281A
P 3650 3700
F 0 "D4" H 3650 3917 50  0000 C CNN
F 1 "1N4001" H 3650 3826 50  0000 C CNN
F 2 "Diode_SMD:D_SMA" H 3650 3525 50  0001 C CNN
F 3 "http://www.vishay.com/docs/88503/1n4001.pdf" H 3650 3700 50  0001 C CNN
	1    3650 3700
	1    0    0    -1  
$EndComp
$Comp
L Transistor_BJT:PN2222A Q2
U 1 1 5F571ED4
P 3400 4200
F 0 "Q2" H 3590 4246 50  0000 L CNN
F 1 "PN2222A" H 3590 4155 50  0000 L CNN
F 2 "airpcb_smd:SOT-23_Reverse_Handsoldering" H 3600 4125 50  0001 L CIN
F 3 "http://www.fairchildsemi.com/ds/PN/PN2222A.pdf" H 3400 4200 50  0001 L CNN
	1    3400 4200
	1    0    0    -1  
$EndComp
$Comp
L Device:R R4
U 1 1 5F5717C2
P 2050 3550
F 0 "R4" H 1980 3504 50  0000 R CNN
F 1 "200" H 1980 3595 50  0000 R CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 1980 3550 50  0001 C CNN
F 3 "~" H 2050 3550 50  0001 C CNN
	1    2050 3550
	-1   0    0    1   
$EndComp
$Comp
L Device:D_Zener D3
U 1 1 5F5713B8
P 3500 3850
F 0 "D3" V 3454 3930 50  0000 L CNN
F 1 "13" V 3545 3930 50  0000 L CNN
F 2 "Diode_SMD:D_MiniMELF_Handsoldering" H 3500 3850 50  0001 C CNN
F 3 "~" H 3500 3850 50  0001 C CNN
	1    3500 3850
	0    1    1    0   
$EndComp
$Comp
L Isolator:PC817 U2
U 1 1 5F57071E
P 2900 3800
F 0 "U2" H 2900 4125 50  0000 C CNN
F 1 "PC817" H 2900 4034 50  0000 C CNN
F 2 "Package_DIP:DIP-4_W7.62mm" H 2700 3600 50  0001 L CIN
F 3 "http://www.soselectronic.cz/a_info/resource/d/pc817.pdf" H 2900 3800 50  0001 L CNN
	1    2900 3800
	1    0    0    -1  
$EndComp
$Comp
L Device:R R6
U 1 1 5F57040A
P 3200 4050
F 0 "R6" H 3130 4004 50  0000 R CNN
F 1 "1k" H 3130 4095 50  0000 R CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 3130 4050 50  0001 C CNN
F 3 "~" H 3200 4050 50  0001 C CNN
	1    3200 4050
	-1   0    0    1   
$EndComp
$Comp
L Device:R R1
U 1 1 5F5701AE
P 1600 3900
F 0 "R1" V 1393 3900 50  0000 C CNN
F 1 "1k" V 1484 3900 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 1530 3900 50  0001 C CNN
F 3 "~" H 1600 3900 50  0001 C CNN
	1    1600 3900
	0    1    1    0   
$EndComp
$Comp
L Connector_Generic:Conn_01x03 J8
U 1 1 5F5D0080
P 5850 2800
F 0 "J8" H 5930 2842 50  0000 L CNN
F 1 "WS2812" H 5930 2751 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x03_P2.54mm_Vertical" H 5850 2800 50  0001 C CNN
F 3 "~" H 5850 2800 50  0001 C CNN
	1    5850 2800
	1    0    0    -1  
$EndComp
Text GLabel 5650 2700 0    50   Input ~ 0
5V
Text GLabel 5650 2800 0    50   Input ~ 0
D4
$Comp
L power:GND #PWR08
U 1 1 5F5D0088
P 5650 2900
F 0 "#PWR08" H 5650 2650 50  0001 C CNN
F 1 "GND" V 5655 2772 50  0000 R CNN
F 2 "" H 5650 2900 50  0001 C CNN
F 3 "" H 5650 2900 50  0001 C CNN
	1    5650 2900
	0    1    1    0   
$EndComp
Text GLabel 10155 2620 2    50   Input ~ 0
D7
Wire Wire Line
	5285 1625 5285 1575
Wire Wire Line
	5585 1625 5285 1625
NoConn ~ 5585 1425
Text GLabel 5585 1525 0    50   Input ~ 0
B
$Comp
L Connector_Generic:Conn_01x05 J11
U 1 1 5F689FE4
P 5785 1625
F 0 "J11" H 5865 1667 50  0000 L CNN
F 1 "PSU" H 5865 1576 50  0000 L CNN
F 2 "airpcb_smd:HW-108" H 5785 1625 50  0001 C CNN
F 3 "~" H 5785 1625 50  0001 C CNN
	1    5785 1625
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J3
U 1 1 5F563401
P 5185 1375
F 0 "J3" V 5385 1325 50  0000 L CNN
F 1 "DC" V 5285 1280 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x02_P2.54mm_Vertical" H 5185 1375 50  0001 C CNN
F 3 "~" H 5185 1375 50  0001 C CNN
	1    5185 1375
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR06
U 1 1 5F5E84C7
P 5585 1825
F 0 "#PWR06" H 5585 1575 50  0001 C CNN
F 1 "GND" H 5590 1652 50  0000 C CNN
F 2 "" H 5585 1825 50  0001 C CNN
F 3 "" H 5585 1825 50  0001 C CNN
	1    5585 1825
	1    0    0    -1  
$EndComp
Text GLabel 4920 2105 0    50   Input ~ 0
5V
Text GLabel 4585 1575 0    50   Input ~ 0
A
$Comp
L Device:R R8
U 1 1 5F5E47A2
P 4735 1575
F 0 "R8" V 4528 1575 50  0000 C CNN
F 1 "100" V 4619 1575 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 4665 1575 50  0001 C CNN
F 3 "~" H 4735 1575 50  0001 C CNN
	1    4735 1575
	0    1    1    0   
$EndComp
$Comp
L Diode:1N4001 D5
U 1 1 5F5E3E56
P 5035 1575
F 0 "D5" H 5035 1792 50  0000 C CNN
F 1 "1N4001" H 5035 1701 50  0000 C CNN
F 2 "Diode_SMD:D_SMA" H 5035 1400 50  0001 C CNN
F 3 "http://www.vishay.com/docs/88503/1n4001.pdf" H 5035 1575 50  0001 C CNN
	1    5035 1575
	-1   0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x08 J5
U 1 1 6050FFAB
P 7265 1955
F 0 "J5" H 7183 2472 50  0000 C CNN
F 1 "Conn_01x08" H 7183 2381 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x08_P2.54mm_Vertical" H 7265 1955 50  0001 C CNN
F 3 "~" H 7265 1955 50  0001 C CNN
	1    7265 1955
	-1   0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x08 J6
U 1 1 60518E11
P 7955 1965
F 0 "J6" H 7873 2482 50  0000 C CNN
F 1 "Conn_01x08" H 7873 2391 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x08_P2.54mm_Vertical" H 7955 1965 50  0001 C CNN
F 3 "~" H 7955 1965 50  0001 C CNN
	1    7955 1965
	-1   0    0    -1  
$EndComp
Text GLabel 7465 1655 2    50   Input ~ 0
RST
Text GLabel 7465 1755 2    50   Input ~ 0
A0
Text GLabel 7465 1855 2    50   Input ~ 0
D0
Text GLabel 7465 1955 2    50   Input ~ 0
D5
Text GLabel 7465 2055 2    50   Input ~ 0
D6
Text GLabel 7465 2155 2    50   Input ~ 0
D7
Text GLabel 7465 2255 2    50   Input ~ 0
D8
Text GLabel 7465 2355 2    50   Input ~ 0
3V3
Text GLabel 8155 1665 2    50   Input ~ 0
5V
$Comp
L power:GND #PWR0101
U 1 1 605317E9
P 8155 1765
F 0 "#PWR0101" H 8155 1515 50  0001 C CNN
F 1 "GND" V 8160 1637 50  0000 R CNN
F 2 "" H 8155 1765 50  0001 C CNN
F 3 "" H 8155 1765 50  0001 C CNN
	1    8155 1765
	0    -1   -1   0   
$EndComp
Text GLabel 8155 1865 2    50   Input ~ 0
D4
Text GLabel 8155 1965 2    50   Input ~ 0
D3
Text GLabel 8155 2065 2    50   Input ~ 0
SDA
Text GLabel 8155 2165 2    50   Input ~ 0
SCL
Text GLabel 8155 2265 2    50   Input ~ 0
ESP_RX
Text GLabel 8155 2365 2    50   Input ~ 0
ESP_TX
Text Notes 9630 4180 0    50   ~ 0
ESP12E Module
Text Notes 9425 1170 0    50   ~ 0
Wemos D1 mini
Text Notes 7405 1290 0    50   ~ 0
Expansion headers\nfor Wemos D1 mini
Wire Notes Line
	1000 2445 4040 2445
Wire Notes Line
	1000 750  4040 750 
$Comp
L power:VCC #PWR0102
U 1 1 60619A73
P 7630 3755
F 0 "#PWR0102" H 7630 3605 50  0001 C CNN
F 1 "VCC" V 7645 3882 50  0000 L CNN
F 2 "" H 7630 3755 50  0001 C CNN
F 3 "" H 7630 3755 50  0001 C CNN
	1    7630 3755
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR0103
U 1 1 6061A22D
P 7615 3905
F 0 "#PWR0103" H 7615 3655 50  0001 C CNN
F 1 "GND" V 7620 3777 50  0000 R CNN
F 2 "" H 7615 3905 50  0001 C CNN
F 3 "" H 7615 3905 50  0001 C CNN
	1    7615 3905
	0    1    1    0   
$EndComp
$Comp
L power:PWR_FLAG #FLG0101
U 1 1 6061AD4A
P 7630 3755
F 0 "#FLG0101" H 7630 3830 50  0001 C CNN
F 1 "PWR_FLAG" V 7630 3883 50  0000 L CNN
F 2 "" H 7630 3755 50  0001 C CNN
F 3 "~" H 7630 3755 50  0001 C CNN
	1    7630 3755
	0    1    1    0   
$EndComp
$Comp
L power:GNDPWR #PWR0104
U 1 1 6061FDF8
P 7615 3905
F 0 "#PWR0104" H 7615 3705 50  0001 C CNN
F 1 "GNDPWR" V 7620 3797 50  0000 R CNN
F 2 "" H 7615 3855 50  0001 C CNN
F 3 "" H 7615 3855 50  0001 C CNN
	1    7615 3905
	0    -1   -1   0   
$EndComp
Text GLabel 7590 4145 0    50   Input ~ 0
A
Text GLabel 7590 4245 0    50   Input ~ 0
B
$Comp
L power:PWR_FLAG #FLG0102
U 1 1 6051A0D4
P 7590 4145
F 0 "#FLG0102" H 7590 4220 50  0001 C CNN
F 1 "PWR_FLAG" V 7590 4273 50  0000 L CNN
F 2 "" H 7590 4145 50  0001 C CNN
F 3 "~" H 7590 4145 50  0001 C CNN
	1    7590 4145
	0    1    1    0   
$EndComp
$Comp
L power:PWR_FLAG #FLG0103
U 1 1 6051FCA8
P 7590 4245
F 0 "#FLG0103" H 7590 4320 50  0001 C CNN
F 1 "PWR_FLAG" V 7590 4373 50  0000 L CNN
F 2 "" H 7590 4245 50  0001 C CNN
F 3 "~" H 7590 4245 50  0001 C CNN
	1    7590 4245
	0    1    1    0   
$EndComp
Text GLabel 5585 1725 0    50   Input ~ 0
PSU_OUT
$Comp
L Connector_Generic:Conn_01x03 J9
U 1 1 60558595
P 5120 2205
F 0 "J9" H 5200 2247 50  0000 L CNN
F 1 "5V3V" H 5200 2156 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x03_P2.54mm_Vertical" H 5120 2205 50  0001 C CNN
F 3 "~" H 5120 2205 50  0001 C CNN
	1    5120 2205
	1    0    0    -1  
$EndComp
Text GLabel 4920 2305 0    50   Input ~ 0
3V3
Text GLabel 4920 2205 0    50   Input ~ 0
PSU_OUT
$EndSCHEMATC
