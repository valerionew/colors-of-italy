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
L RF_Module:ESP32-WROOM-32 U?
U 1 1 60040EE5
P 1450 2500
F 0 "U?" H 1450 4081 50  0000 C CNN
F 1 "ESP32-WROOM-32" H 1450 3990 50  0000 C CNN
F 2 "RF_Module:ESP32-WROOM-32" H 1450 1000 50  0001 C CNN
F 3 "https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32_datasheet_en.pdf" H 1150 2550 50  0001 C CNN
	1    1450 2500
	1    0    0    -1  
$EndComp
$Comp
L Sensor_Optical:TEPT4400 Q?
U 1 1 600459D9
P 4750 7400
F 0 "Q?" H 4940 7446 50  0000 L CNN
F 1 "TEPT4400" H 4940 7355 50  0000 L CNN
F 2 "LED_THT:LED_D3.0mm_Clear" H 5230 7260 50  0001 C CNN
F 3 "https://www.vishay.com/docs/81341/tept4400.pdf" H 4750 7400 50  0001 C CNN
	1    4750 7400
	1    0    0    -1  
$EndComp
Text Notes 3350 7050 0    50   ~ 0
fototransistor per adattare la luminosità dei led a quella ambientale
$Comp
L Sensor:BME280 U?
U 1 1 6004718F
P 1950 6400
F 0 "U?" H 1521 6446 50  0000 R CNN
F 1 "BME280" H 1521 6355 50  0000 R CNN
F 2 "Package_LGA:Bosch_LGA-8_2.5x2.5mm_P0.65mm_ClockwisePinNumbering" H 3450 5950 50  0001 C CNN
F 3 "https://ae-bst.resource.bosch.com/media/_tech/media/datasheets/BST-BME280-DS002.pdf" H 1950 6200 50  0001 C CNN
	1    1950 6400
	1    0    0    -1  
$EndComp
Text Notes 1250 5600 0    50   ~ 0
mettere il footprint non fa mai male
$Comp
L Connector:USB_C_Plug_USB2.0 P?
U 1 1 600476E6
P 10350 5350
F 0 "P?" H 10457 6217 50  0000 C CNN
F 1 "USB_C_Plug_USB2.0" H 10457 6126 50  0000 C CNN
F 2 "" H 10500 5350 50  0001 C CNN
F 3 "https://www.usb.org/sites/default/files/documents/usb_type-c.zip" H 10500 5350 50  0001 C CNN
	1    10350 5350
	1    0    0    -1  
$EndComp
Text Notes 10050 4350 0    50   ~ 0
power source
$Comp
L Device:Battery_Cell BT?
U 1 1 6004B3F0
P 7800 5950
F 0 "BT?" H 7918 6046 50  0000 L CNN
F 1 "Battery_Cell" H 7918 5955 50  0000 L CNN
F 2 "" V 7800 6010 50  0001 C CNN
F 3 "~" V 7800 6010 50  0001 C CNN
	1    7800 5950
	1    0    0    -1  
$EndComp
Text Notes 7450 5600 0    50   ~ 0
batteria 18650 just in case
$Comp
L Battery_Management:MCP73811T-420I-OT U?
U 1 1 6004C76A
P 7850 5150
F 0 "U?" H 8294 5196 50  0000 L CNN
F 1 "MCP73811T-420I-OT" H 8294 5105 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23-5" H 7900 4900 50  0001 L CNN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/22036b.pdf" H 7600 5400 50  0001 C CNN
	1    7850 5150
	1    0    0    -1  
$EndComp
Text Notes 3400 5950 0    50   ~ 0
bisogna fare un ragionamento sulle alimentazioni:\nesp32 vuole 3.3V precisi, ma forse non vorrei far passare anche la\ncorrente dei led per quel regolatore. il 595 può farsi carico di 35mA totali\nquindi forse dovremmo mettere dei mosfet in più. 
Text Notes 1150 4700 0    50   ~ 0
esp32 module footprint by https://www.huubzegveld.nl/programmeren.html
Text Notes 3900 3900 0    50   ~ 0
sensore touch in cima per togglare luce on/off\nipotesi 3 bottoni touch -/0/+
$Comp
L LED:WS2812B D?
U 1 1 60187DB4
P 4550 1250
F 0 "D?" H 4600 1500 50  0000 L CNN
F 1 "WS2812B" H 4600 1000 50  0000 L CNN
F 2 "LED_SMD:LED_WS2812B_PLCC4_5.0x5.0mm_P3.2mm" H 4600 950 50  0001 L TNN
F 3 "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf" H 4650 875 50  0001 L TNN
	1    4550 1250
	1    0    0    -1  
$EndComp
$Comp
L LED:WS2812B D?
U 1 1 601897A1
P 5150 1250
F 0 "D?" H 5200 1500 50  0000 L CNN
F 1 "WS2812B" H 5200 1000 50  0000 L CNN
F 2 "LED_SMD:LED_WS2812B_PLCC4_5.0x5.0mm_P3.2mm" H 5200 950 50  0001 L TNN
F 3 "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf" H 5250 875 50  0001 L TNN
	1    5150 1250
	1    0    0    -1  
$EndComp
$Comp
L LED:WS2812B D?
U 1 1 60189EF1
P 5750 1250
F 0 "D?" H 5800 1500 50  0000 L CNN
F 1 "WS2812B" H 5800 1000 50  0000 L CNN
F 2 "LED_SMD:LED_WS2812B_PLCC4_5.0x5.0mm_P3.2mm" H 5800 950 50  0001 L TNN
F 3 "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf" H 5850 875 50  0001 L TNN
	1    5750 1250
	1    0    0    -1  
$EndComp
$Comp
L LED:WS2812B D?
U 1 1 6018A824
P 6350 1250
F 0 "D?" H 6400 1500 50  0000 L CNN
F 1 "WS2812B" H 6400 1000 50  0000 L CNN
F 2 "LED_SMD:LED_WS2812B_PLCC4_5.0x5.0mm_P3.2mm" H 6400 950 50  0001 L TNN
F 3 "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf" H 6450 875 50  0001 L TNN
	1    6350 1250
	1    0    0    -1  
$EndComp
$Comp
L LED:WS2812B D?
U 1 1 6018C149
P 6950 1250
F 0 "D?" H 7000 1500 50  0000 L CNN
F 1 "WS2812B" H 7000 1000 50  0000 L CNN
F 2 "LED_SMD:LED_WS2812B_PLCC4_5.0x5.0mm_P3.2mm" H 7000 950 50  0001 L TNN
F 3 "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf" H 7050 875 50  0001 L TNN
	1    6950 1250
	1    0    0    -1  
$EndComp
$Comp
L LED:WS2812B D?
U 1 1 6018C3C1
P 7550 1250
F 0 "D?" H 7600 1500 50  0000 L CNN
F 1 "WS2812B" H 7600 1000 50  0000 L CNN
F 2 "LED_SMD:LED_WS2812B_PLCC4_5.0x5.0mm_P3.2mm" H 7600 950 50  0001 L TNN
F 3 "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf" H 7650 875 50  0001 L TNN
	1    7550 1250
	1    0    0    -1  
$EndComp
$Comp
L LED:WS2812B D?
U 1 1 6018C3CB
P 8150 1250
F 0 "D?" H 8200 1500 50  0000 L CNN
F 1 "WS2812B" H 8200 1000 50  0000 L CNN
F 2 "LED_SMD:LED_WS2812B_PLCC4_5.0x5.0mm_P3.2mm" H 8200 950 50  0001 L TNN
F 3 "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf" H 8250 875 50  0001 L TNN
	1    8150 1250
	1    0    0    -1  
$EndComp
$Comp
L LED:WS2812B D?
U 1 1 6018C3D5
P 8750 1250
F 0 "D?" H 8800 1500 50  0000 L CNN
F 1 "WS2812B" H 8800 1000 50  0000 L CNN
F 2 "LED_SMD:LED_WS2812B_PLCC4_5.0x5.0mm_P3.2mm" H 8800 950 50  0001 L TNN
F 3 "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf" H 8850 875 50  0001 L TNN
	1    8750 1250
	1    0    0    -1  
$EndComp
$Comp
L LED:WS2812B D?
U 1 1 6018F10D
P 9350 1250
F 0 "D?" H 9400 1500 50  0000 L CNN
F 1 "WS2812B" H 9400 1000 50  0000 L CNN
F 2 "LED_SMD:LED_WS2812B_PLCC4_5.0x5.0mm_P3.2mm" H 9400 950 50  0001 L TNN
F 3 "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf" H 9450 875 50  0001 L TNN
	1    9350 1250
	1    0    0    -1  
$EndComp
$Comp
L LED:WS2812B D?
U 1 1 6018F6F3
P 9950 1250
F 0 "D?" H 10000 1500 50  0000 L CNN
F 1 "WS2812B" H 10000 1000 50  0000 L CNN
F 2 "LED_SMD:LED_WS2812B_PLCC4_5.0x5.0mm_P3.2mm" H 10000 950 50  0001 L TNN
F 3 "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf" H 10050 875 50  0001 L TNN
	1    9950 1250
	1    0    0    -1  
$EndComp
$Comp
L LED:WS2812B D?
U 1 1 60192A32
P 4550 2250
F 0 "D?" H 4600 2500 50  0000 L CNN
F 1 "WS2812B" H 4600 2000 50  0000 L CNN
F 2 "LED_SMD:LED_WS2812B_PLCC4_5.0x5.0mm_P3.2mm" H 4600 1950 50  0001 L TNN
F 3 "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf" H 4650 1875 50  0001 L TNN
	1    4550 2250
	1    0    0    -1  
$EndComp
$Comp
L LED:WS2812B D?
U 1 1 60192D76
P 5150 2250
F 0 "D?" H 5200 2500 50  0000 L CNN
F 1 "WS2812B" H 5200 2000 50  0000 L CNN
F 2 "LED_SMD:LED_WS2812B_PLCC4_5.0x5.0mm_P3.2mm" H 5200 1950 50  0001 L TNN
F 3 "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf" H 5250 1875 50  0001 L TNN
	1    5150 2250
	1    0    0    -1  
$EndComp
$Comp
L LED:WS2812B D?
U 1 1 60192D80
P 5750 2250
F 0 "D?" H 5800 2500 50  0000 L CNN
F 1 "WS2812B" H 5800 2000 50  0000 L CNN
F 2 "LED_SMD:LED_WS2812B_PLCC4_5.0x5.0mm_P3.2mm" H 5800 1950 50  0001 L TNN
F 3 "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf" H 5850 1875 50  0001 L TNN
	1    5750 2250
	1    0    0    -1  
$EndComp
$Comp
L LED:WS2812B D?
U 1 1 60192D8A
P 6350 2250
F 0 "D?" H 6400 2500 50  0000 L CNN
F 1 "WS2812B" H 6400 2000 50  0000 L CNN
F 2 "LED_SMD:LED_WS2812B_PLCC4_5.0x5.0mm_P3.2mm" H 6400 1950 50  0001 L TNN
F 3 "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf" H 6450 1875 50  0001 L TNN
	1    6350 2250
	1    0    0    -1  
$EndComp
$Comp
L LED:WS2812B D?
U 1 1 60192D94
P 6950 2250
F 0 "D?" H 7000 2500 50  0000 L CNN
F 1 "WS2812B" H 7000 2000 50  0000 L CNN
F 2 "LED_SMD:LED_WS2812B_PLCC4_5.0x5.0mm_P3.2mm" H 7000 1950 50  0001 L TNN
F 3 "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf" H 7050 1875 50  0001 L TNN
	1    6950 2250
	1    0    0    -1  
$EndComp
$Comp
L LED:WS2812B D?
U 1 1 60192D9E
P 7550 2250
F 0 "D?" H 7600 2500 50  0000 L CNN
F 1 "WS2812B" H 7600 2000 50  0000 L CNN
F 2 "LED_SMD:LED_WS2812B_PLCC4_5.0x5.0mm_P3.2mm" H 7600 1950 50  0001 L TNN
F 3 "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf" H 7650 1875 50  0001 L TNN
	1    7550 2250
	1    0    0    -1  
$EndComp
$Comp
L LED:WS2812B D?
U 1 1 60192DA8
P 8150 2250
F 0 "D?" H 8200 2500 50  0000 L CNN
F 1 "WS2812B" H 8200 2000 50  0000 L CNN
F 2 "LED_SMD:LED_WS2812B_PLCC4_5.0x5.0mm_P3.2mm" H 8200 1950 50  0001 L TNN
F 3 "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf" H 8250 1875 50  0001 L TNN
	1    8150 2250
	1    0    0    -1  
$EndComp
$Comp
L LED:WS2812B D?
U 1 1 60192DB2
P 8750 2250
F 0 "D?" H 8800 2500 50  0000 L CNN
F 1 "WS2812B" H 8800 2000 50  0000 L CNN
F 2 "LED_SMD:LED_WS2812B_PLCC4_5.0x5.0mm_P3.2mm" H 8800 1950 50  0001 L TNN
F 3 "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf" H 8850 1875 50  0001 L TNN
	1    8750 2250
	1    0    0    -1  
$EndComp
$Comp
L LED:WS2812B D?
U 1 1 60192DBC
P 9350 2250
F 0 "D?" H 9400 2500 50  0000 L CNN
F 1 "WS2812B" H 9400 2000 50  0000 L CNN
F 2 "LED_SMD:LED_WS2812B_PLCC4_5.0x5.0mm_P3.2mm" H 9400 1950 50  0001 L TNN
F 3 "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf" H 9450 1875 50  0001 L TNN
	1    9350 2250
	1    0    0    -1  
$EndComp
$Comp
L LED:WS2812B D?
U 1 1 60192DC6
P 9950 2250
F 0 "D?" H 10000 2500 50  0000 L CNN
F 1 "WS2812B" H 10000 2000 50  0000 L CNN
F 2 "LED_SMD:LED_WS2812B_PLCC4_5.0x5.0mm_P3.2mm" H 10000 1950 50  0001 L TNN
F 3 "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf" H 10050 1875 50  0001 L TNN
	1    9950 2250
	1    0    0    -1  
$EndComp
Wire Wire Line
	4550 1550 4550 1600
Wire Wire Line
	4550 1600 5150 1600
Wire Wire Line
	9950 1600 9950 1550
Wire Wire Line
	9350 1550 9350 1600
Connection ~ 9350 1600
Wire Wire Line
	9350 1600 9950 1600
Wire Wire Line
	8750 1550 8750 1600
Connection ~ 8750 1600
Wire Wire Line
	8750 1600 9350 1600
Wire Wire Line
	8150 1550 8150 1600
Connection ~ 8150 1600
Wire Wire Line
	8150 1600 8750 1600
Wire Wire Line
	6950 1550 6950 1600
Connection ~ 6950 1600
Wire Wire Line
	6950 1600 7550 1600
Wire Wire Line
	6350 1550 6350 1600
Connection ~ 6350 1600
Wire Wire Line
	6350 1600 6950 1600
Wire Wire Line
	5750 1550 5750 1600
Connection ~ 5750 1600
Wire Wire Line
	5750 1600 6350 1600
Wire Wire Line
	5150 1550 5150 1600
Connection ~ 5150 1600
Wire Wire Line
	5150 1600 5750 1600
Wire Wire Line
	4550 2550 4550 2600
Wire Wire Line
	4550 2600 5150 2600
Wire Wire Line
	9950 2600 9950 2550
Wire Wire Line
	9350 2550 9350 2600
Connection ~ 9350 2600
Wire Wire Line
	9350 2600 9950 2600
Wire Wire Line
	8750 2550 8750 2600
Connection ~ 8750 2600
Wire Wire Line
	8750 2600 9350 2600
Wire Wire Line
	8150 2550 8150 2600
Connection ~ 8150 2600
Wire Wire Line
	8150 2600 8750 2600
Wire Wire Line
	6950 2550 6950 2600
Connection ~ 6950 2600
Wire Wire Line
	6950 2600 7550 2600
Wire Wire Line
	6350 2550 6350 2600
Connection ~ 6350 2600
Wire Wire Line
	6350 2600 6950 2600
Wire Wire Line
	5750 2550 5750 2600
Connection ~ 5750 2600
Wire Wire Line
	5750 2600 6350 2600
Wire Wire Line
	5150 2550 5150 2600
Connection ~ 5150 2600
Wire Wire Line
	5150 2600 5750 2600
Wire Wire Line
	7550 1550 7550 1600
Connection ~ 7550 1600
Wire Wire Line
	7550 1600 8150 1600
Wire Wire Line
	7550 2550 7550 2600
Connection ~ 7550 2600
Wire Wire Line
	7550 2600 8150 2600
Wire Wire Line
	4550 1950 4550 1900
Wire Wire Line
	4550 1900 5150 1900
Wire Wire Line
	9950 1900 9950 1950
Wire Wire Line
	9350 1950 9350 1900
Connection ~ 9350 1900
Wire Wire Line
	9350 1900 9950 1900
Wire Wire Line
	8750 1950 8750 1900
Connection ~ 8750 1900
Wire Wire Line
	8750 1900 9350 1900
Wire Wire Line
	8150 1950 8150 1900
Connection ~ 8150 1900
Wire Wire Line
	8150 1900 8750 1900
Wire Wire Line
	6950 1950 6950 1900
Connection ~ 6950 1900
Wire Wire Line
	6950 1900 7550 1900
Wire Wire Line
	6350 1950 6350 1900
Connection ~ 6350 1900
Wire Wire Line
	6350 1900 6950 1900
Wire Wire Line
	5750 1950 5750 1900
Connection ~ 5750 1900
Wire Wire Line
	5750 1900 6350 1900
Wire Wire Line
	5150 1950 5150 1900
Connection ~ 5150 1900
Wire Wire Line
	5150 1900 5750 1900
Wire Wire Line
	7550 1950 7550 1900
Connection ~ 7550 1900
Wire Wire Line
	7550 1900 8150 1900
Wire Wire Line
	4550 950  4550 900 
Wire Wire Line
	4550 900  5150 900 
Wire Wire Line
	9950 900  9950 950 
Wire Wire Line
	9350 950  9350 900 
Connection ~ 9350 900 
Wire Wire Line
	9350 900  9950 900 
Wire Wire Line
	8750 950  8750 900 
Connection ~ 8750 900 
Wire Wire Line
	8750 900  9350 900 
Wire Wire Line
	8150 950  8150 900 
Connection ~ 8150 900 
Wire Wire Line
	8150 900  8750 900 
Wire Wire Line
	6950 950  6950 900 
Connection ~ 6950 900 
Wire Wire Line
	6950 900  7550 900 
Wire Wire Line
	6350 950  6350 900 
Connection ~ 6350 900 
Wire Wire Line
	6350 900  6950 900 
Wire Wire Line
	5750 950  5750 900 
Connection ~ 5750 900 
Wire Wire Line
	5750 900  6350 900 
Wire Wire Line
	5150 950  5150 900 
Connection ~ 5150 900 
Wire Wire Line
	5150 900  5750 900 
Wire Wire Line
	7550 950  7550 900 
Connection ~ 7550 900 
Wire Wire Line
	7550 900  8150 900 
Wire Wire Line
	10250 1250 10500 1250
Wire Wire Line
	10500 1250 10500 1750
Wire Wire Line
	10500 1750 4100 1750
Wire Wire Line
	4100 1750 4100 2250
Wire Wire Line
	4100 2250 4250 2250
Wire Wire Line
	9950 1600 10700 1600
Wire Wire Line
	10700 1600 10700 2600
Wire Wire Line
	10700 2600 9950 2600
Connection ~ 9950 1600
Connection ~ 9950 2600
Wire Wire Line
	9950 1900 10850 1900
Wire Wire Line
	10850 1900 10850 900 
Wire Wire Line
	10850 900  9950 900 
Connection ~ 9950 1900
Connection ~ 9950 900 
$Comp
L power:+5V #PWR?
U 1 1 601AA733
P 10850 800
F 0 "#PWR?" H 10850 650 50  0001 C CNN
F 1 "+5V" H 10865 973 50  0000 C CNN
F 2 "" H 10850 800 50  0001 C CNN
F 3 "" H 10850 800 50  0001 C CNN
	1    10850 800 
	1    0    0    -1  
$EndComp
Wire Wire Line
	10850 800  10850 900 
Connection ~ 10850 900 
$Comp
L power:GND #PWR?
U 1 1 601AD992
P 10700 2650
F 0 "#PWR?" H 10700 2400 50  0001 C CNN
F 1 "GND" H 10705 2477 50  0000 C CNN
F 2 "" H 10700 2650 50  0001 C CNN
F 3 "" H 10700 2650 50  0001 C CNN
	1    10700 2650
	1    0    0    -1  
$EndComp
Text GLabel 2100 2200 2    50   Input ~ 0
LED_CHAIN
Wire Wire Line
	2100 2200 2050 2200
Text GLabel 4200 1250 0    50   Input ~ 0
LED_CHAIN
Wire Wire Line
	4200 1250 4250 1250
$EndSCHEMATC
