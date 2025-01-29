#include "ESP32S3Buzzer.h"
#include "Arduino.h"
#include <driver/ledc.h>

ESP32S3Buzzer::ESP32S3Buzzer(uint8_t pin, uint8_t channel)
    : _pin(pin), _ledcChannel(channel), _timerIndex(channel), _isPlaying(false), _startTime(0), _currentDuration(0), _isToneOn(false), _currentCycle(0) {}

void ESP32S3Buzzer::begin() {
  ledcSetup(_ledcChannel, 0, 13);
  ledcAttachPin(_pin, _ledcChannel);
}

void ESP32S3Buzzer::end() {
  ledcDetachPin(_pin);
  ledcWriteTone(_ledcChannel, 0);
}

void ESP32S3Buzzer::tone(uint32_t freq, uint32_t onDuration, uint32_t offDuration, uint16_t cycles) {
  Tone newTone = {freq, onDuration, offDuration, cycles};
  _toneQueue.push(newTone);
}

void ESP32S3Buzzer::update() {
  if (_isPlaying) {
    if (millis() - _startTime >= _currentDuration) {
      if (_isToneOn) {
        _isToneOn = false;
        ledcWriteTone(_ledcChannel, 0);
        _currentDuration = _toneQueue.front().offDuration;
        _currentCycle++;
      } else {
        if (_currentCycle < _toneQueue.front().cycles) {
          _isToneOn = true;
          ledcWriteTone(_ledcChannel, _toneQueue.front().frequency);
          _currentDuration = _toneQueue.front().onDuration;
        } else {
          _isPlaying = false;
          _toneQueue.pop();
        }
      }
      _startTime = millis();
    }
  } else if (!_toneQueue.empty()) {
    _isPlaying = true;
    _isToneOn = true;
    ledcWriteTone(_ledcChannel, _toneQueue.front().frequency);
    _startTime = millis();
    _currentDuration = _toneQueue.front().onDuration;
    _currentCycle = 0;
  }
}
