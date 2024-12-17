/*
 * The file is licensed under the Apache License, Version 2.0
 * (c) 2021 Helmut Tschemernjak
 * 31515 Wunstorf (Hannover) Germany
 */

#ifndef __ESP32LED_H__
#define __ESP32LED_H__

#include "WS2812LedBase.h"
#include "driver/rmt.h"

class ESP32Led : public WS2812LedBase {
	using WS2812LedBase::WS2812LedBase;
public:
	~ESP32Led();
	void Init(void);
	void Show(void);
	int MaxIntances(void);


private:
	rmt_config_t  _config;
	rmt_item32_t *_led_data_buffer;
	int _my_rmt_channel_no;	// current channel in use for this instance
	static uint16_t _rmt_channel;

	// These values have been verified by measuring pulse timing with a scope.
	// example for the ESP32 with a AHB = 80 MHz and a divider of 2 equals 40 MHz
	uint32_t _T1H; // 32;
	uint32_t _T1L; // 18;
	uint32_t _T0H; // 16;
	uint32_t _T0L; // 34;
	uint32_t _TRST;// 2000
};

#endif // __ESP32LED_H__
