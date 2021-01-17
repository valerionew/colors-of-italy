EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 4
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
Text Notes 10350 2550 0    50   ~ 0
Valle d'Aosta\nPiemonte\nLiguria\nLombardia\nTrentino-Alto Adige\nVeneto\nFriuli-Venezia Giulia\nEmilia Romagna\nToscana\nUmbria\nMarche\nLazio\nAbruzzo\nMolise\nCampania\nPuglia\nBasilicata\nCalabria\nSicilia\nSardegna
$Sheet
S 9450 1100 500  300 
U 60042504
F0 "valledaosta" 50
F1 "valledaosta.sch" 50
F2 "R" I L 9450 1150 50 
F3 "G" I L 9450 1250 50 
F4 "B" I L 9450 1350 50 
F5 "com" I R 9950 1250 50 
$EndSheet
$Sheet
S 9450 1700 500  300 
U 60043E22
F0 "piemonte" 50
F1 "piemonte.sch" 50
F2 "R" I L 9450 1750 50 
F3 "G" I L 9450 1850 50 
F4 "B" I L 9450 1950 50 
F5 "com" I R 9950 1850 50 
$EndSheet
$Sheet
S 9450 2250 500  300 
U 6004407C
F0 "lombardia" 50
F1 "lombardia.sch" 50
F2 "R" I L 9450 2300 50 
F3 "G" I L 9450 2400 50 
F4 "B" I L 9450 2500 50 
F5 "com" I R 9950 2400 50 
$EndSheet
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
$Comp
L 74xx:74HC595 U?
U 1 1 6004F01F
P 3250 2300
F 0 "U?" H 3250 3081 50  0000 C CNN
F 1 "74HC595" H 3250 2990 50  0000 C CNN
F 2 "" H 3250 2300 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/sn74hc595.pdf" H 3250 2300 50  0001 C CNN
	1    3250 2300
	1    0    0    -1  
$EndComp
$Comp
L 74xx:74HC595 U?
U 1 1 600501D1
P 4600 2300
F 0 "U?" H 4600 3081 50  0000 C CNN
F 1 "74HC595" H 4600 2990 50  0000 C CNN
F 2 "" H 4600 2300 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/sn74hc595.pdf" H 4600 2300 50  0001 C CNN
	1    4600 2300
	1    0    0    -1  
$EndComp
$Comp
L 74xx:74HC595 U?
U 1 1 60051E8F
P 5900 2300
F 0 "U?" H 5900 3081 50  0000 C CNN
F 1 "74HC595" H 5900 2990 50  0000 C CNN
F 2 "" H 5900 2300 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/sn74hc595.pdf" H 5900 2300 50  0001 C CNN
	1    5900 2300
	1    0    0    -1  
$EndComp
Text Notes 2700 1300 0    50   ~ 0
bisogna fare un ragionamento sulle alimentazioni:\nesp32 vuole 3.3V precisi, ma forse non vorrei far passare anche la\ncorrente dei led per quel regolatore. il 595 può farsi carico di 35mA totali\nquindi forse dovremmo mettere dei mosfet in più. 
Wire Wire Line
	8600 1150 9350 1150
Wire Wire Line
	9450 1750 9350 1750
Wire Wire Line
	9350 1750 9350 1150
Connection ~ 9350 1150
Wire Wire Line
	9350 1150 9450 1150
Wire Wire Line
	9450 2300 9350 2300
Wire Wire Line
	9350 2300 9350 1750
Connection ~ 9350 1750
Wire Wire Line
	8600 1250 9250 1250
Wire Wire Line
	9450 1850 9250 1850
Wire Wire Line
	9250 1850 9250 1250
Connection ~ 9250 1250
Wire Wire Line
	9250 1250 9450 1250
Wire Wire Line
	9450 2400 9250 2400
Wire Wire Line
	9250 2400 9250 1850
Connection ~ 9250 1850
Wire Wire Line
	8600 1350 9150 1350
Wire Wire Line
	9450 2500 9150 2500
Wire Wire Line
	9150 2500 9150 1950
Connection ~ 9150 1350
Wire Wire Line
	9150 1350 9450 1350
Wire Wire Line
	9450 1950 9150 1950
Connection ~ 9150 1950
Wire Wire Line
	9150 1950 9150 1350
Text Notes 8000 900  0    50   ~ 0
Il multiplexing può tenere accese tutte le regioni dello stesso colore insieme\nCosì sarebbe a 4 fasi invece che 20\nQuesto aumenta considerevolmente la corrente
Text Notes 1150 4700 0    50   ~ 0
esp32 module footprint by https://www.huubzegveld.nl/programmeren.html
Text Notes 4300 3650 0    50   ~ 0
sensore touch in cima per togglare luce on/off\nipotesi 3 bottoni touch -/0/+
$EndSCHEMATC
