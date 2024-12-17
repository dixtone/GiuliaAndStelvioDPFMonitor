/*
 * The file is licensed under the Apache License, Version 2.0
 * (c) 2021 Helmut Tschemernjak
 * 31515 Wunstorf (Hannover) Germany
 */

#ifndef __WS2812LED_H__
#define __WS2812LED_H__

#include "ESP32Led.h"

#ifdef ARDUINO_ARCH_ESP32
class WS2812Led : public ESP32Led {
	using ESP32Led::ESP32Led;
};

#else
 #error "unsupported implementation"
#endif

#endif // __WS2812LED_H__
