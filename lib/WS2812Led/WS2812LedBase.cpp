/*
 * The file is licensed under the Apache License, Version 2.0
 * (c) 2021 Helmut Tschemernjak
 * 31515 Wunstorf (Hannover) Germany
 */

#include "WS2812Led.h"
#include <string.h>

const float WS2812LedBase::_T0H_SEC = 0.000000400; 	// 400ns
const float WS2812LedBase::_T0L_SEC = 0.000000850;	// 850ns
const float WS2812LedBase::_T1H_SEC = 0.000000800;	// 800ns
const float WS2812LedBase::_T1L_SEC = 0.000000450;	// 450ns
const float WS2812LedBase::_TRST_SEC = 0.000050000;	// 50us


WS2812LedBase::WS2812LedBase(int pin, int ledCount, LedColorModel colorModel, bool debug)
{
    _numLEDs = ledCount;
    _debug = debug;
    _pin = pin;
	_bytesPerColor = colorModel;

	// number of LEDS: 3 colors equals 24 bit, or 4 colors equals 32 bit
    _ledValues = new uint8_t[_numLEDs * _bytesPerColor];
    memset(_ledValues, 0, _numLEDs * _bytesPerColor);

    _initDone = false; // we need a delayed init to ensure that Arduino libs are inited.
}


WS2812LedBase::~WS2812LedBase()
{
	if (!_initDone)
		return;

	delete[] _ledValues;
};


void
WS2812LedBase::Update(uint8_t red, uint8_t green, uint8_t blue, uint8_t white, int LEDIndex, bool show)
{
	if (!_initDone)
		Init();
		
    if (LEDIndex > _numLEDs-1)
      return;
      
    uint8_t *curRGBValue = _ledValues + (LEDIndex * _bytesPerColor);
    
    switch(_bytesPerColor) {
    case LED_RGB: // 3 channels equals 24 bit
		*curRGBValue++ = green; // the WS2812 order is Green/Red/Blue
		*curRGBValue++ = red;
		*curRGBValue++ = blue;
    	break;
    case LED_RGBW: //  4 channels equals 32 bit
		*curRGBValue++ = green; // the WS2812 order is Green/Red/Blue/White
		*curRGBValue++ = red;
		*curRGBValue++ = blue;
		*curRGBValue++ = white;
	default:
		break;
	}

    if (show)
      Show();
};


void
WS2812LedBase::UpdateAll(uint8_t red, uint8_t green, uint8_t blue, uint8_t white, bool show)
{
	if (!_initDone)
		Init();
		
    for (int i = 0; i < _numLEDs; i++) {
      this->Update(red, green, blue, white, i, false);
    }
    if (show)
      Show();
}


void
WS2812LedBase::Update(RGBColors color, int LEDIndex, bool show) {
	if (!_initDone)
		Init();
		
	uint8_t red = 	(color >> 24) & 0xff;
	uint8_t green = (color >> 16) & 0xff;
	uint8_t blue = 	(color >> 8) & 0xff;
	uint8_t white = color & 0xff;
	this->Update(red, green, blue, white, LEDIndex, show);
}
  
void
WS2812LedBase::UpdateAll(RGBColors color, bool show) {
	if (!_initDone)
		Init();
		
    for (int i = 0; i < _numLEDs; i++) {
       this->Update(color, i, false);
    }
    if (show)
      Show();
}

#if 0
#include <Arduino.h>
void
WS2812LedBase::_DumpLedBuffer(void)
{
	for (int i = 0; i < _numLEDs; i++) {
		Serial.printf("LED%d: ", i);
		for (int c = 0; c < _bytesPerColor; c++) {
			Serial.printf("%02x ", _ledValues[(i * _bytesPerColor) + c]);
		}
		Serial.printf("\n");
	}
}
#endif

void
WS2812LedBase::Rotate(int steps, bool directionForward, bool show)
{
	size_t slen = _numLEDs * _bytesPerColor;

	// _DumpLedBuffer();
	for(int cnt = 0; cnt < steps; cnt++) {
		char savedLed[4];

		/*
		 * memmove allows overlapping memory moves, that's the trick.
		 * We just need to save one LED, move the others and restore the
		 * saved LED into its wrapped position.
		 */
		if (directionForward) {
			memcpy(savedLed, &_ledValues[slen - _bytesPerColor], _bytesPerColor); // save last LED
			memmove(_ledValues+_bytesPerColor, _ledValues+0, slen-_bytesPerColor);
			memcpy(_ledValues, savedLed, _bytesPerColor);
		} else {
			memcpy(savedLed, _ledValues, _bytesPerColor); // save first LED
			memmove(_ledValues, _ledValues +_bytesPerColor, slen-_bytesPerColor);
			memcpy(&_ledValues[slen-_bytesPerColor], savedLed, _bytesPerColor);
		}
	}
	if (show)
		Show();
}

void
WS2812LedBase::Clear()
{
	if (!_initDone)
		Init();
		 
	memset(_ledValues, 0, _numLEDs * _bytesPerColor);
	Show();
}

void
WS2812LedBase::Brightness(uint8_t brightness)
{
	if (!_initDone)
		Init();
		
	_brightness = brightness;
}

int
WS2812LedBase::LedColorType(void)
{
	return _bytesPerColor;
}


int
WS2812LedBase::NumberOfLeds(void)
{
	return _numLEDs;
}

void
WS2812LedBase::ClearAtExit(bool yesNo)
{
	_clearLedsOnExit = yesNo;
}

