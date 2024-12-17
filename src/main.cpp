/**
 *  Based on T5L2 ASIC 4 Inches 960*400 COF and 480x480 COB
 *  
 *  Display DWIN DMG40960F040_01WTC or DMG48480C028_03WTC
 *  
 *  DWIN display communication with ESP32
 *  
 *  DWIN communication based on https://github.com/dwinhmi/DWIN_DGUS_HMI/tree/master
 *  
 *  UART2 GPIO 16 -> TX DWIN
 *        GPIO 17 -> RX DWIN
 *        
 *  IF ELM Direct 
 *        GPIO 13 ->  ELM_RX 
 *        GPIO 22 ->  ELM_TX 
 *        ELM_BAUDRATE 38400
 *
 *  Buzzer GPIO 32      
 *  ESP32 nodemcu (library 2.0.2)
 */


#include <Arduino.h>
#include "definitions.h"
#include "structs.h"
#include "myApplication.h"
#include "myDisplay.h"
#include "myUtils.h"
#include "myBtUtils.h"

//app definitions
EspSoftwareSerial::UART ElmConnector;
BluetoothSerial BluetoothConnector;
ELM327 elm327;

ESP32S3Buzzer buzzer(BUZZER_PIN, BUZZER_CHN);

WS2812Led wsLED(LED_PIN, 1, WS2812Led::LED_RGBW);

DWIN hmi(DWIN_SERIAL, DWSERIAL_RXPin, DWSERIAL_TXPin, DWSERIAL_BAUD_RATE);

myUtils utils;
myBtUtils btUtils;
myDisplay display(&hmi);
myApplication app;

TaskHandle_t asyncTask;

//main
void setup() {
  
  Serial.begin(SERIAL_BAUD_RATE);
 
  app.Init();
  
}

void loop() {  
  app.Loop();
  delay(10);
}