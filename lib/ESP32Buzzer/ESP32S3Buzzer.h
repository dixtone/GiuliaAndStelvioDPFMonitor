#ifndef ESP32S3BUZZER_H
#define ESP32S3BUZZER_H

#include "Arduino.h"
#include <queue>

class ESP32S3Buzzer {
public:
  ESP32S3Buzzer(uint8_t pin, uint8_t channel);
  void begin();
  void end();
  void tone(uint32_t freq, uint32_t onDuration, uint32_t offDuration, uint16_t cycles = 1);
  void update();

private:
  uint8_t _pin;
  uint8_t _ledcChannel;
  uint8_t _timerIndex;

  struct Tone {
    uint32_t frequency;
    uint32_t onDuration;
    uint32_t offDuration;
    uint16_t cycles;
  };

  std::queue<Tone> _toneQueue;
  bool _isPlaying;
  uint32_t _startTime;
  uint32_t _currentDuration;
  bool _isToneOn;
  uint16_t _currentCycle;
};

#endif
