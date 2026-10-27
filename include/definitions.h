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
#define BUZZER_CHN 5

#define LED_PIN 18

#define CAN_TX_PIN 13
#define CAN_RX_PIN 14
#define START_STOP_PIN 23

//fs
#define SETTINGS_FILE "/settings.cfg" //nome file dei settaggi
#define SETTINGS_TMP_FILE "/settings.tmp" //file temporaneo per scrittura sicura dei settaggi
#define GRAPH_OIL_FILE "/oil.log" //nome file del log olio
#define GRAPH_OIL_TMP_FILE "/oil.tmp" //file temporaneo per scrittura sicura del log olio
#define PAIRED_DEVICE_FILE "/paired.cfg" //nome file dei settaggi
#define PAIRED_DEVICE_TMP_FILE "/paired.tmp" //file temporaneo per scrittura sicura del dispositivo associato

#define MAX_BT_DEVICES 4

#define MAX_OIL_LOG 5

//oil log: write only when car is on since a while and level is stable,
//to avoid flash writes during engine cranking (voltage drop)
#define OIL_LOG_START_DELAY 60000UL //ms after power on before first write
#define OIL_LOG_STABLE_TIME 60000UL //ms oil level must be stable before write
#if GAS_ENGINE
#define OIL_LOG_TOLERANCE 0.05 //liters
#else
#define OIL_LOG_TOLERANCE 0.5 //mm
#endif

//alarms
#define ALARM_MIN_INTERVAL 5 //seconds, minimum alarm repeat interval

//wi-fi settings
#define  device_ssid "SD4-DPF-V2-" //AP Mode device name
#define  device_password "1234"  //AP Mode password

//defining debug mode or dump
#define EEPROM_SIZE 64
#ifndef DEBUG_MODE
#define DEBUG_MODE  false
#endif
//defining if is DIRECT or BluetoothSerial connection
//when DIRECT we use extern SJA1000 module to read direct from canbus
#ifndef CANBUS_DIRECT
#define CANBUS_DIRECT false
#endif

//defining if we need to disable START and STOP on startup
//only works when CANBUS_DIRECT = true
#ifndef DISABLE_STARTSTOP
#define DISABLE_STARTSTOP false
#endif

//defining bluetooth ble connection
#ifndef BLUETOOTH_BLE
#define BLUETOOTH_BLE false
#endif

//gas engine
#ifndef GAS_ENGINE
#define GAS_ENGINE false
#endif

//oil log
#ifndef DISABLE_OIL_LOG
#define DISABLE_OIL_LOG false
#endif

#endif