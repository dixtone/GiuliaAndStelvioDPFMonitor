#ifndef definitions_h
#define definitions_h 

#include "header.h"

#define SERIAL_BAUD_RATE 115200
#define DWSERIAL_TXPin 17
#define DWSERIAL_RXPin 16
#define DWSERIAL_MODE SERIAL_8N1
#define DWSERIAL_BAUD_RATE 115200
#define DWIN_SERIAL Serial2

#define BUZZER_PIN 32
#define BUZZER_CHN 0

#define LED_PIN 18

#define ELM_RX 13
#define ELM_TX 22
#define ELM_BAUDRATE 38400

//fs
#define SETTINGS_FILE "/settings.cfg" //nome file dei settaggi
#define GRAPH_OIL_FILE "/oil.log" //nome file del log olio
#define PAIRED_DEVICE_FILE "/paired.cfg" //nome file dei settaggi

#define MAX_BT_DEVICES 4

#define MAX_OIL_LOG 5

//wi-fi settings
#define  device_ssid "SD4-DPF-V2-" //AP Mode device name
#define  device_password "1234"  //AP Mode password

//defining debug mode or dump
#define EEPROM_SIZE 64
#ifndef DEBUG_MODE
#define DEBUG_MODE  false
#endif
#ifndef ELM_DUMP
#define ELM_DUMP false
#endif

//defining if is DIRECT or BluetoothSerial connection
#ifndef ELM_DIRECT
#define ELM_DIRECT false
#endif

#endif