/*
 * The file is licensed under the Apache License, Version 2.0
 * (c) 2021 Helmut Tschemernjak
 * 31515 Wunstorf (Hannover) Germany
 */
 
#ifndef __WS2812LEDBASE_H__
#define __WS2812LEDBASE_H__

#include <stdint.h>


class WS2812LedBase {
public:
	enum LedColorModel {
		LED_RGB = 3,		// 3 channels equals 24 bit
		LED_RGBW = 4,		// 3 channels equals 32 bit
	};
	/*
	 * some predefined LED colors for easy use of color names.
	 * See Update()
	 */
	enum RGBColors {
		RED			= 0xff000000,
		GREEN		= 0x00ff0000,
		BLUE		= 0x0000ff00,
		WHITE		= 0xffffff00,
		WHITE_RGBW	= 0x000000ff,
		SILVER		= 0xc0c0c000,
		GRAY		= 0x80808000,
		GRAY_RGBW	= 0x0008080,
		OLIVE		= 0x80800000,
		MAROON		= 0x80000000,
		YELLOW		= 0xffff0000,
		AQUA		= 0x00ffff00,
		NAVY		= 0x00008000,
		FUCHSIA		= 0xff00ff00,
		PURPLE		= 0x80008000,
		ORANGE		= 0xff8c0000,
		PINK 		= 0xC7158500,
		SKYBLUE		= 0x5050f000,
		SKYBLUE2	= 0x007f7f00,
		TURQUOISE	= 0x30C03000,
		BLACK		= 0x00000000,
	};
	
	/*
	 * WS2812Led constructor allows creating a WS2812 LED chain instance
	 * The first parameter is the GPIO pin which drives the WS2812 LEDs
	 * The second parameter is the RMT channel (up to 8 channels are available on the ESP32)
	 * e.g. second LED stripe can be driven by a second channel.
	 *
	 * For the ESP32-S2 the on-board LED (gpio no. 18):
	 * WS2812Led wsLED(18);
	 * wsLED.Update(wsLED.RED);
	 */
	WS2812LedBase(int pin, int ledCount = 1, LedColorModel colorModel = LED_RGB, bool debug = false);

	/*
	 * will free existing resources.
	 */
	~WS2812LedBase();

	/*
	 * Transfer the set LED colors,from previous Update() calls, to the LED stripe
	 */
	virtual void Show(void) = 0;

	/*
	 * Maximum number of LED stripes (WS2812Led instances) supported systemwide:
	 * Updating the LEDs stripes keeps system resources busy. During updates
	 * there may be limitations on the number of parallel updates.
	*/
	virtual int MaxIntances(void) = 0;
  
  	/*
  	 * The Init call will be delayed and issued at the first API access
  	 * after the constructor. The reason for the delay is that in Arduino
  	 * global variables some libraries not initialized.
	 */
 	virtual void Init(void) = 0;

	/*
	 * Update allows setting the RGB colors (Red, Green, Blue, White) from 0-255, e.g.:
	 * Update(20, 0, 0)  // for a light red
	 * Update(255, 0, 0) // for a strong red
	 *
	 * By default the first LED is set, otherwise specify LED index, e,g:
	 * Update(255, 0, 0, 1); // for LED index 1 equals the second LED
	 *
	 * The fourth parameter specifies if the new color will be shown right away (default),
	 * or set to false to allow multiple Updates() calls for different LEDs
	 * When done, calling Show() will transfer the previously defined colors into the LEDs
	 */
	void Update(uint8_t red, uint8_t green, uint8_t blue, uint8_t white = 0, int LEDIndex = 0, bool show = true);

	/*
	 * same as Update(), but for all LEDs
	 */
	void UpdateAll(uint8_t red, uint8_t green, uint8_t blue, uint8_t white = 0, bool show = true);

	/*
	 * Update allows to setting named colors, e.g.:
	 * Update(WS2812Led::NAVY);
	 * or Update(wsLed.NAVY); for a darker blue
	 */
	void Update(RGBColors color, int LEDIndex = 0, bool show = true);
  
  	/*
  	 * Update the entire LED stripe
	 */
	void UpdateAll(RGBColors color, bool show = true);
	
	/*
	 * Rotates/rolls the LED stripe to the right or left
	 * The number of steps (LEDs) can be specified via the count
	 * Default is the direction forward which means LED 1 becomes LED 2, etc.
	 * the last LED becomes the first LED.
	 */
	void Rotate(int steps = 1, bool directionForward = true, bool show = true);
  
	/*
	 * Clears all LEDs by setting the color to zero
	 */
	void Clear(void);
	
	/*
	 * In the destruction of the WS2812Led class the LEDs are turned off.
	 * For some use cases it can be useful to keep the LEDs stripe turned on.
	 */
	void ClearAtExit(bool yesNo);
	
	/*
	 * Brightness allows setting the brightness between 0-255
	 * Full brightness (255) is the default.
	 * When a stripe is shown all color values will be reduced
	 * according to the brightness
	 * Example: Brightness(128) // sets the brightness to 50%
	 */
	void Brightness(uint8_t brightness);
	
	/*
	 * Gets the LED color type, at present LED_RGB or LED_RGBW
	 */
	int LedColorType(void);
	
	/*
	 * Gets the number of configured LEDs (defined in the Constructor))
	 */
	int NumberOfLeds(void);
  
  protected:
	bool _initDone;
	bool _debug;
	int _pin;
	int _numLEDs;
	uint8_t _brightness = 0xff;
	LedColorModel _bytesPerColor;
	uint8_t *_ledValues;
	bool _clearLedsOnExit = true;

	void _DumpLedBuffer(void);

	/*
	 * Here are the timing values according to the WS2812B datasheet for a single bit:
	 * Signal of _T0H_SEC +_T0H_SEC represents one OFF bit
	 * Signal of _T1H_SEC +_T1H_SEC represents one ON bit
	 * See const float in WS2812LedBase.cpp
	 */
	static const float _T0H_SEC;
	static const float _T0L_SEC;
	static const float _T1H_SEC;
	static const float _T1L_SEC;
	static const float _TRST_SEC;
};

#endif // __WS2812LEDBASE_H__
