/*
 * The file is licensed under the Apache License, Version 2.0
 * (c) 2021 Helmut Tschemernjak
 * 31515 Wunstorf (Hannover) Germany
 */

#include <Arduino.h>
#include "ESP32Led.h"

uint16_t ESP32Led::_rmt_channel = 0;

ESP32Led::~ESP32Led() {
	if (!_initDone)
		return;

	if (_clearLedsOnExit)
		Clear();

	ESP_ERROR_CHECK(rmt_wait_tx_done(_config.channel, portMAX_DELAY)); // flush pending
			  
	rmt_driver_uninstall(_config.channel);

	delete[] _led_data_buffer;
	_rmt_channel &= (1<<_my_rmt_channel_no); // release the channel

	gpio_set_direction(_config.gpio_num, GPIO_MODE_INPUT);
}

void
ESP32Led::Init() {
	if (_initDone)
		return;
		
	if (_debug)
		Serial.printf("WS2812Led pin=%d\n", _pin);
	pinMode(_pin, OUTPUT);
	/*
	 * find the next available rmt channel
	 * the ESP32 has 8, the ESP32-C3 has only 4 channels.
	 */
	_my_rmt_channel_no = -1;
	for (int i = 0; i < RMT_CHANNEL_MAX; i++) {
	  if ((_rmt_channel & (1<<i)) == false) {
		_my_rmt_channel_no = i;
		_rmt_channel |= (1<<i);
		break;
	  }
	}
	if (_debug) {
	  Serial.printf("rmt_channel: %d\n", _rmt_channel);
	  Serial.printf("_my_rmt_channel_no: %d\n", _my_rmt_channel_no);
	}
	ESP_ERROR_CHECK(_my_rmt_channel_no < 0);

	rmt_config_t newconfig = RMT_DEFAULT_CONFIG_TX((gpio_num_t)_pin, (rmt_channel_t)_my_rmt_channel_no);
	newconfig.clk_div = 2;
	memcpy(&_config, &newconfig, sizeof(_config));

	ESP_ERROR_CHECK(rmt_config(&_config));
	ESP_ERROR_CHECK(rmt_driver_install(_config.channel, 0, 0));

	_led_data_buffer = new rmt_item32_t[(_numLEDs * (_bytesPerColor * 8))+1]; // count * bits per led + 1 reset code

	uint32_t clock = 0; // usually 40 MHz, half of AHB
	rmt_get_counter_clock(_config.channel, &clock);
	float cyles = 1.0/(float)clock;

	_T0H = (_T0H_SEC / cyles) + 0.5; // see timings in WS2812LedBase.cpp
	_T0L = (_T0L_SEC / cyles) + 0.5;
	_T1H = (_T1H_SEC / cyles) + 0.5;
	_T1L = (_T1L_SEC / cyles) + 0.5;
	/*
	 * the program delay/overhead between two Show()'s is significant
	 * therefore we subtract this delay from the RST period to
	 * allow the fasted updates within the WS2812B spec
	 */
	float ESP_RMT_START_DELAY = 0.000015;
#ifdef ARDUINO_ESP32C3_DEV
	ESP_RMT_START_DELAY = 0.000022;
#endif
	if (_TRST_SEC > ESP_RMT_START_DELAY)
		_TRST = ((_TRST_SEC-ESP_RMT_START_DELAY) / cyles) + 0.5;
	else
		_TRST = (_TRST_SEC / cyles) + 0.5;
	if (_debug) {
		Serial.printf("_T0H: %d, _T0L: %d\n", _T0H, _T0L);
		Serial.printf("_T1H: %d, _T1L: %d\n", _T1H, _T1L);
		Serial.printf("_TRST: %d\n", _TRST);
	}
	delayMicroseconds(((1.0/_TRST_SEC)/1000)); // force the stripe to turn into reset (50µsec)
	_initDone = true;
}

void
ESP32Led::Show(void)
{
	if (!_initDone)
		Init();
		
    ESP_ERROR_CHECK(rmt_wait_tx_done(_config.channel, portMAX_DELAY)); // wait for pending LED TX request

     /*
      * convert the RGB color (24 bits) into the RMT buffer consisting of
      * rmt_item32_t items for each bit, 24 bit (3 x 8) per led
      */
     for (uint32_t ledIndex = 0; ledIndex < _numLEDs; ledIndex++) { // index of current LED
      for (int colorIndex = 0; colorIndex < _bytesPerColor; colorIndex++) { // index 0=Green, 1=Red, 2=Blue, 3=White
          u_int8_t curValue = *(_ledValues + (ledIndex * _bytesPerColor) + colorIndex);
          if (_brightness != 0xff && curValue > 0) {
              curValue = (curValue * _brightness) >> 8;
              // Serial.printf("new curValue=%d\n", curValue);
          }
          for (int bit = 0; bit < 8; bit++) {
              if (_debug) {
                Serial.printf("Led(%d): bit(%d) = %d\n", ledIndex,  (ledIndex * (_bytesPerColor * 8)) + (colorIndex * 8) + bit, curValue & (1<<bit) ? 1 : 0);
              }
              if (curValue & (1<<(7-bit))) // high bit first
                _led_data_buffer[(ledIndex * (_bytesPerColor * 8)) + (colorIndex * 8) + bit] = (rmt_item32_t){_T1H, 1, _T1L, 0}; // bit on signal
               else
                _led_data_buffer[(ledIndex * (_bytesPerColor * 8)) + (colorIndex * 8) + bit] = (rmt_item32_t){_T0H, 1, _T0L, 0}; // bit off signal
          }
       }
     }

	 _led_data_buffer[(_numLEDs * (_bytesPerColor * 8))] = (rmt_item32_t){_TRST, 0, 0, 0}; // reset signal

    // finally start transmitting the pattern using the RMT controller,
    // data includes bits for all leds + plus a 50us reset at the last item.
    ESP_ERROR_CHECK(rmt_write_items(_config.channel, _led_data_buffer, (_numLEDs * (_bytesPerColor * 8)) + 1, false));
}


int
ESP32Led::MaxIntances(void)
{
	if (!_initDone)
		Init();
		
    return RMT_CHANNEL_MAX +1;
}
