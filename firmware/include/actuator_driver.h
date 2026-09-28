#pragma once

#include <Arduino.h>
#include "config.h"

class ActuatorDriver {
public:
  ActuatorDriver(int pin, const char* name, unsigned long maxRunDurationMs = MAX_WATERING_RUN_MS);

  void begin();
  void update();

  void turnOn();
  void turnOff();

  bool isOn() const;
  bool isWatchdogTriggered() const;
  void clearWatchdog();

  unsigned long getCurrentRunDurationMs() const;
  unsigned long getTotalRunDurationMs() const;
  const char* getName() const;

private:
  int _pin;
  const char* _name;
  unsigned long _maxRunDurationMs;

  bool _isOn;
  bool _watchdogTriggered;
  unsigned long _turnOnTimestamp;
  unsigned long _totalRunDurationMs;
};
