//main libraies
#include <Math.h>
#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <FS.h>
#include <EEPROM.h>

#include "BluetoothSerial.h"
#include "BTAddress.h"
#include "BTAdvertisedDevice.h"
#include "BTScan.h"

#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include"esp_gap_bt_api.h"
#include "esp_err.h"
#include "esp_ota_ops.h"

//project libraries
#include "OBD2.h"
#include "ESP32S3Buzzer.h"
#include "WS2812Led.h"
#include "DWIN.h"
