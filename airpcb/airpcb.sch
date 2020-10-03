EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Device:R R2
U 1 1 5F560E22
P 1750 1750
F 0 "R2" H 1820 1796 50  0000 L CNN
F 1 "10k" H 1820 1705 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P5.08mm_Horizontal" V 1680 1750 50  0001 C CNN
F 3 "~" H 1750 1750 50  0001 C CNN
	1    1750 1750
	1    0    0    -1  
$EndComp
$Comp
L Device:R R3
U 1 1 5F561166
P 1900 1600
F 0 "R3" V 1693 1600 50  0000 C CNN
F 1 "100" V 1784 1600 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P5.08mm_Horizontal" V 1830 1600 50  0001 C CNN
F 3 "~" H 1900 1600 50  0001 C CNN
	1    1900 1600
	0    1    1    0   
$EndComp
$Comp
L Diode:1N4001 D2
U 1 1 5F563589
P 2200 1600
F 0 "D2" H 2200 1817 50  0000 C CNN
F 1 "1N4001" H 2200 1726 50  0000 C CNN
F 2 "Diode_THT:D_A-405_P2.54mm_Vertical_AnodeUp" H 2200 1425 50  0001 C CNN
F 3 "http://www.vishay.com/docs/88503/1n4001.pdf" H 2200 1600 50  0001 C CNN
	1    2200 1600
	-1   0    0    -1  
$EndComp
$Comp
L Isolator:PC817 U1
U 1 1 5F566455
P 2650 1700
F 0 "U1" H 2650 2025 50  0000 C CNN
F 1 "PC817" H 2650 1934 50  0000 C CNN
F 2 "Package_DIP:DIP-4_W7.62mm" H 2450 1500 50  0001 L CIN
F 3 "http://www.soselectronic.cz/a_info/resource/d/pc817.pdf" H 2650 1700 50  0001 L CNN
	1    2650 1700
	1    0    0    -1  
$EndComp
$Comp
L Device:R R5
U 1 1 5F567057
P 2950 1950
F 0 "R5" H 3020 1996 50  0000 L CNN
F 1 "1k" H 3020 1905 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P5.08mm_Horizontal" V 2880 1950 50  0001 C CNN
F 3 "~" H 2950 1950 50  0001 C CNN
	1    2950 1950
	1    0    0    -1  
$EndComp
$Comp
L Device:D_Zener D1
U 1 1 5F5679E8
P 1600 1900
F 0 "D1" H 1600 2117 50  0000 C CNN
F 1 "13" H 1600 2026 50  0000 C CNN
F 2 "Diode_THT:D_A-405_P2.54mm_Vertical_AnodeUp" H 1600 1900 50  0001 C CNN
F 3 "~" H 1600 1900 50  0001 C CNN
	1    1600 1900
	-1   0    0    -1  
$EndComp
Wire Wire Line
	2350 1800 2350 1900
Wire Wire Line
	2350 1900 1750 1900
Connection ~ 1750 1900
$Comp
L power:GND #PWR03
U 1 1 5F569069
P 2950 2100
F 0 "#PWR03" H 2950 1850 50  0001 C CNN
F 1 "GND" H 2955 1927 50  0000 C CNN
F 2 "" H 2950 2100 50  0001 C CNN
F 3 "" H 2950 2100 50  0001 C CNN
	1    2950 2100
	1    0    0    -1  
$EndComp
Wire Wire Line
	1750 1600 1300 1600
Connection ~ 1750 1600
Wire Wire Line
	1300 1700 1300 1900
Wire Wire Line
	1300 1900 1450 1900
Text GLabel 3200 1600 2    50   Input ~ 0
3V3
Wire Wire Line
	3200 1600 2950 1600
Text GLabel 1300 1700 0    50   Input ~ 0
B
Text GLabel 1300 1600 0    50   Input ~ 0
A
Wire Wire Line
	2950 1800 3200 1800
Connection ~ 2950 1800
Text GLabel 3200 1800 2    50   Input ~ 0
RX
Text Notes 1250 1050 0    50   ~ 0
Read circuit
Text Notes 1300 3050 0    50   ~ 0
Write circuit
Wire Notes Line
	1000 750  1000 2450
Wire Notes Line
	1000 2450 3700 2450
Wire Notes Line
	3700 2450 3700 750 
Wire Notes Line
	3700 750  1000 750 
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
P 9400 1950
F 0 "U3" H 9400 850 50  0000 C CNN
F 1 "WeMos_D1_mini" H 9400 750 50  0000 C CNN
F 2 "Module:WEMOS_D1_mini_light" H 9400 800 50  0001 C CNN
F 3 "https://wiki.wemos.cc/products:d1:d1_mini#documentation" H 7550 800 50  0001 C CNN
	1    9400 1950
	1    0    0    -1  
$EndComp
Text GLabel 9500 1150 1    50   Input ~ 0
3V3
Text GLabel 9800 1850 2    50   Input ~ 0
D3
Text GLabel 9800 2250 2    50   Input ~ 0
RX
Text GLabel 9800 2350 2    50   Input ~ 0
TX
$Comp
L power:GND #PWR011
U 1 1 5F5DE558
P 9400 2750
F 0 "#PWR011" H 9400 2500 50  0001 C CNN
F 1 "GND" H 9405 2577 50  0000 C CNN
F 2 "" H 9400 2750 50  0001 C CNN
F 3 "" H 9400 2750 50  0001 C CNN
	1    9400 2750
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J4
U 1 1 5F5E144A
P 5450 1500
F 0 "J4" H 5530 1492 50  0000 L CNN
F 1 "AB" H 5530 1401 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x02_P2.54mm_Vertical" H 5450 1500 50  0001 C CNN
F 3 "~" H 5450 1500 50  0001 C CNN
	1    5450 1500
	1    0    0    -1  
$EndComp
Text GLabel 5250 1500 0    50   Input ~ 0
A
Text GLabel 5250 1600 0    50   Input ~ 0
B
$Comp
L Diode:1N4001 D5
U 1 1 5F5E3E56
P 5250 2200
F 0 "D5" H 5250 2417 50  0000 C CNN
F 1 "1N4001" H 5250 2326 50  0000 C CNN
F 2 "Diode_THT:D_A-405_P2.54mm_Vertical_AnodeUp" H 5250 2025 50  0001 C CNN
F 3 "http://www.vishay.com/docs/88503/1n4001.pdf" H 5250 2200 50  0001 C CNN
	1    5250 2200
	-1   0    0    -1  
$EndComp
$Comp
L Device:R R8
U 1 1 5F5E47A2
P 4950 2200
F 0 "R8" V 4743 2200 50  0000 C CNN
F 1 "100" V 4834 2200 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P5.08mm_Horizontal" V 4880 2200 50  0001 C CNN
F 3 "~" H 4950 2200 50  0001 C CNN
	1    4950 2200
	0    1    1    0   
$EndComp
Text GLabel 4800 2200 0    50   Input ~ 0
A
Text GLabel 5500 2300 0    50   Input ~ 0
B
Text GLabel 9300 1150 1    50   Input ~ 0
5V
Text GLabel 5500 2450 0    50   Input ~ 0
5V
$Comp
L power:GND #PWR06
U 1 1 5F5E84C7
P 5500 2550
F 0 "#PWR06" H 5500 2300 50  0001 C CNN
F 1 "GND" H 5505 2377 50  0000 C CNN
F 2 "" H 5500 2550 50  0001 C CNN
F 3 "" H 5500 2550 50  0001 C CNN
	1    5500 2550
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x04 J2
U 1 1 5F5E8FD1
P 5350 3150
F 0 "J2" H 5430 3142 50  0000 L CNN
F 1 "DHT" H 5430 3051 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x04_P2.54mm_Vertical" H 5350 3150 50  0001 C CNN
F 3 "~" H 5350 3150 50  0001 C CNN
	1    5350 3150
	1    0    0    -1  
$EndComp
Text GLabel 4700 3050 0    50   Input ~ 0
3V3
Text GLabel 4700 3250 0    50   Input ~ 0
D3
NoConn ~ 5150 3250
$Comp
L power:GND #PWR05
U 1 1 5F5EE6BC
P 5150 3350
F 0 "#PWR05" H 5150 3100 50  0001 C CNN
F 1 "GND" H 5155 3177 50  0000 C CNN
F 2 "" H 5150 3350 50  0001 C CNN
F 3 "" H 5150 3350 50  0001 C CNN
	1    5150 3350
	1    0    0    -1  
$EndComp
$Comp
L Device:R R7
U 1 1 5F5F1CC3
P 4850 3150
F 0 "R7" V 4643 3150 50  0000 C CNN
F 1 "10k" V 4734 3150 50  0000 C CNN
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P5.08mm_Horizontal" V 4780 3150 50  0001 C CNN
F 3 "~" H 4850 3150 50  0001 C CNN
	1    4850 3150
	0    1    1    0   
$EndComp
Wire Wire Line
	5150 3050 4700 3050
Wire Wire Line
	4700 3050 4700 3150
Wire Wire Line
	5000 3150 5150 3150
Wire Wire Line
	4700 3250 5000 3250
Wire Wire Line
	5000 3250 5000 3150
Connection ~ 5000 3150
$Comp
L Connector_Generic:Conn_01x04 J1
U 1 1 5F5F93C3
P 5050 4050
F 0 "J1" H 5130 4042 50  0000 L CNN
F 1 "SPI" H 5130 3951 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x04_P2.54mm_Vertical" H 5050 4050 50  0001 C CNN
F 3 "~" H 5050 4050 50  0001 C CNN
	1    5050 4050
	1    0    0    -1  
$EndComp
Text GLabel 4850 3950 0    50   Input ~ 0
3V3
$Comp
L power:GND #PWR04
U 1 1 5F5F9DA7
P 4850 4050
F 0 "#PWR04" H 4850 3800 50  0001 C CNN
F 1 "GND" V 4855 3922 50  0000 R CNN
F 2 "" H 4850 4050 50  0001 C CNN
F 3 "" H 4850 4050 50  0001 C CNN
	1    4850 4050
	0    1    1    0   
$EndComp
Text GLabel 4850 4150 0    50   Input ~ 0
SCL
Text GLabel 4850 4250 0    50   Input ~ 0
SDA
Text GLabel 9800 1650 2    50   Input ~ 0
SCL
Text GLabel 9800 1750 2    50   Input ~ 0
SDA
Wire Notes Line
	4450 1150 6500 1150
Wire Notes Line
	6500 1150 6500 4450
Wire Notes Line
	6500 4450 4450 4450
Wire Notes Line
	4450 4450 4450 1150
Text Notes 4600 1350 0    50   ~ 0
Connectors
$Comp
L Connector_Generic:Conn_01x08 J10
U 1 1 5F61916D
P 7550 1650
F 0 "J10" H 7630 1642 50  0000 L CNN
F 1 "Wemos_header_right" H 7630 1551 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x08_P2.54mm_Vertical" H 7550 1650 50  0001 C CNN
F 3 "~" H 7550 1650 50  0001 C CNN
	1    7550 1650
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x08 J9
U 1 1 5F61A91F
P 7450 3000
F 0 "J9" H 7368 2375 50  0000 C CNN
F 1 "Wemos_header_left" H 7368 2466 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x08_P2.54mm_Vertical" H 7450 3000 50  0001 C CNN
F 3 "~" H 7450 3000 50  0001 C CNN
	1    7450 3000
	-1   0    0    1   
$EndComp
Text GLabel 7650 3300 2    50   Input ~ 0
5V
Text GLabel 7650 2900 2    50   Input ~ 0
SDA
Text GLabel 7650 2800 2    50   Input ~ 0
SCL
Text GLabel 7650 3000 2    50   Input ~ 0
D3
Text GLabel 7650 3100 2    50   Input ~ 0
D4
$Comp
L power:GND #PWR09
U 1 1 5F61CE8F
P 7650 3200
F 0 "#PWR09" H 7650 2950 50  0001 C CNN
F 1 "GND" V 7655 3072 50  0000 R CNN
F 2 "" H 7650 3200 50  0001 C CNN
F 3 "" H 7650 3200 50  0001 C CNN
	1    7650 3200
	0    -1   -1   0   
$EndComp
Text GLabel 7650 2700 2    50   Input ~ 0
ESP_RX
Text GLabel 7650 2600 2    50   Input ~ 0
ESP_TX
Text GLabel 9000 1550 0    50   Input ~ 0
RST
Text GLabel 7350 1350 0    50   Input ~ 0
RST
Text GLabel 9800 1450 2    50   Input ~ 0
A0
Text GLabel 7350 1450 0    50   Input ~ 0
A0
Text GLabel 9800 1550 2    50   Input ~ 0
D0
Text GLabel 7350 1550 0    50   Input ~ 0
D0
Text GLabel 9800 2050 2    50   Input ~ 0
D5
Text GLabel 9800 2150 2    50   Input ~ 0
D6
Text GLabel 9800 1950 2    50   Input ~ 0
D4
Text GLabel 7350 1650 0    50   Input ~ 0
D5
Text GLabel 7350 1750 0    50   Input ~ 0
D6
Text GLabel 7350 1850 0    50   Input ~ 0
D7
Text GLabel 7350 1950 0    50   Input ~ 0
D8
Text GLabel 7350 2050 0    50   Input ~ 0
3V3
$Comp
L RF_Module:ESP-12E U4
U 1 1 5F622C8F
P 9400 4550
F 0 "U4" H 9400 5531 50  0000 C CNN
F 1 "ESP-12E" H 9400 5440 50  0000 C CNN
F 2 "RF_Module:ESP-12E" H 9400 4550 50  0001 C CNN
F 3 "http://wiki.ai-thinker.com/_media/esp8266/esp8266_series_modules_user_manual_v1.1.pdf" H 9050 4650 50  0001 C CNN
	1    9400 4550
	1    0    0    -1  
$EndComp
Text GLabel 8800 3950 0    50   Input ~ 0
RST
Text GLabel 9000 1950 0    50   Input ~ 0
ESP_TX
Text GLabel 9000 1850 0    50   Input ~ 0
ESP_RX
Text GLabel 10000 4050 2    50   Input ~ 0
ESP_TX
Text GLabel 10000 4250 2    50   Input ~ 0
ESP_RX
Text GLabel 9400 3750 2    50   Input ~ 0
3V3
Text GLabel 10000 4450 2    50   Input ~ 0
SCL
Text GLabel 10000 4350 2    50   Input ~ 0
SDA
Text GLabel 10000 3950 2    50   Input ~ 0
D3
Text GLabel 10000 4150 2    50   Input ~ 0
D4
Text GLabel 10000 4950 2    50   Input ~ 0
D0
Text GLabel 10000 4550 2    50   Input ~ 0
D6
Text GLabel 10000 4650 2    50   Input ~ 0
D7
Text GLabel 10000 4750 2    50   Input ~ 0
D5
Text GLabel 10000 4850 2    50   Input ~ 0
D8
Text GLabel 8800 4350 0    50   Input ~ 0
A0
$Comp
L Connector_Generic:Conn_01x03 J7
U 1 1 5F6377E8
P 6000 4100
F 0 "J7" H 6080 4142 50  0000 L CNN
F 1 "PWR" H 6080 4051 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x03_P2.54mm_Vertical" H 6000 4100 50  0001 C CNN
F 3 "~" H 6000 4100 50  0001 C CNN
	1    6000 4100
	1    0    0    -1  
$EndComp
Text GLabel 5800 4000 0    50   Input ~ 0
5V
Text GLabel 5800 4100 0    50   Input ~ 0
3V3
$Comp
L power:GND #PWR07
U 1 1 5F638CFB
P 5800 4200
F 0 "#PWR07" H 5800 3950 50  0001 C CNN
F 1 "GND" V 5805 4072 50  0000 R CNN
F 2 "" H 5800 4200 50  0001 C CNN
F 3 "" H 5800 4200 50  0001 C CNN
	1    5800 4200
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR012
U 1 1 5F63CC7E
P 9400 5250
F 0 "#PWR012" H 9400 5000 50  0001 C CNN
F 1 "GND" H 9405 5077 50  0000 C CNN
F 2 "" H 9400 5250 50  0001 C CNN
F 3 "" H 9400 5250 50  0001 C CNN
	1    9400 5250
	1    0    0    -1  
$EndComp
NoConn ~ 8800 4750
NoConn ~ 8800 4850
NoConn ~ 8800 5050
NoConn ~ 8800 4150
$Comp
L power:VCC #PWR010
U 1 1 5F6466FB
P 8650 850
F 0 "#PWR010" H 8650 700 50  0001 C CNN
F 1 "VCC" V 8665 977 50  0000 L CNN
F 2 "" H 8650 850 50  0001 C CNN
F 3 "" H 8650 850 50  0001 C CNN
	1    8650 850 
	0    -1   -1   0   
$EndComp
Text GLabel 8650 850  2    50   Input ~ 0
5V
NoConn ~ 8800 4550
NoConn ~ 8800 4650
NoConn ~ 8800 4950
$Comp
L Connector_Generic:Conn_01x02 J3
U 1 1 5F563401
P 5400 2000
F 0 "J3" V 5600 1950 50  0000 L CNN
F 1 "DC Switch" V 5500 1800 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x02_P2.54mm_Vertical" H 5400 2000 50  0001 C CNN
F 3 "~" H 5400 2000 50  0001 C CNN
	1    5400 2000
	0    -1   -1   0   
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J5
U 1 1 5F5A73B4
P 5700 2200
F 0 "J5" H 5780 2192 50  0000 L CNN
F 1 "PSU_IN" H 5780 2101 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x02_P2.54mm_Vertical" H 5700 2200 50  0001 C CNN
F 3 "~" H 5700 2200 50  0001 C CNN
	1    5700 2200
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J6
U 1 1 5F5AA2BF
P 5700 2450
F 0 "J6" H 5780 2442 50  0000 L CNN
F 1 "PSU_OUT" H 5780 2351 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x02_P2.54mm_Vertical" H 5700 2450 50  0001 C CNN
F 3 "~" H 5700 2450 50  0001 C CNN
	1    5700 2450
	1    0    0    -1  
$EndComp
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
TX
Connection ~ 2050 3700
Text GLabel 2050 3400 1    50   Input ~ 0
3V3
$Comp
L Transistor_BJT:PN2222A Q1
U 1 1 5F584BBB
P 1950 3900
F 0 "Q1" H 2140 3946 50  0000 L CNN
F 1 "PN2222A" H 2140 3855 50  0000 L CNN
F 2 "Package_TO_SOT_THT:TO-92_Inline" H 2150 3825 50  0001 L CIN
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
F 2 "Diode_THT:D_A-405_P2.54mm_Vertical_AnodeUp" H 3650 3525 50  0001 C CNN
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
F 2 "Package_TO_SOT_THT:TO-92_Inline" H 3600 4125 50  0001 L CIN
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
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P5.08mm_Horizontal" V 1980 3550 50  0001 C CNN
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
F 2 "Diode_THT:D_A-405_P2.54mm_Vertical_AnodeUp" H 3500 3850 50  0001 C CNN
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
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P5.08mm_Horizontal" V 3130 4050 50  0001 C CNN
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
F 2 "Resistor_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P5.08mm_Horizontal" V 1530 3900 50  0001 C CNN
F 3 "~" H 1600 3900 50  0001 C CNN
	1    1600 3900
	0    1    1    0   
$EndComp
$Comp
L Connector_Generic:Conn_01x03 J8
U 1 1 5F5D0080
P 6100 3200
F 0 "J8" H 6180 3242 50  0000 L CNN
F 1 "WS2812" H 6180 3151 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x03_P2.54mm_Vertical" H 6100 3200 50  0001 C CNN
F 3 "~" H 6100 3200 50  0001 C CNN
	1    6100 3200
	1    0    0    -1  
$EndComp
Text GLabel 5900 3100 0    50   Input ~ 0
5V
Text GLabel 5900 3200 0    50   Input ~ 0
D4
$Comp
L power:GND #PWR08
U 1 1 5F5D0088
P 5900 3300
F 0 "#PWR08" H 5900 3050 50  0001 C CNN
F 1 "GND" V 5905 3172 50  0000 R CNN
F 2 "" H 5900 3300 50  0001 C CNN
F 3 "" H 5900 3300 50  0001 C CNN
	1    5900 3300
	0    1    1    0   
$EndComp
$EndSCHEMATC
