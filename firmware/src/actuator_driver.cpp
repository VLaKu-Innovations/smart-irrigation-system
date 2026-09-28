#include "actuator_driver.h"

ActuatorDriver::ActuatorDriver(int pin, const char* name, unsigned long maxRunDurationMs)
  : _pin(pin), _name(name), _maxRunDurationMs(maxRunDurationMs),
    _isOn(false), _watchdogTriggered(false), _turnOnTimestamp(0), _totalRunDurationMs(0) {}

void ActuatorDriver::begin() {
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
  _isOn = false;
  _watchdogTriggered = false;
}

void ActuatorDriver::update() {
  if (_isOn) {
    unsigned long elapsed = millis() - _turnOnTimestamp;
    if (elapsed >= _maxRunDurationMs) {
      // Emergency safety shutoff
      turnOff();
      _watchdogTriggered = true;
      Serial.printf("[WATCHDOG ALERT] Actuator '%s' exceeded max duration (%lu ms)! Forced OFF.\n",
                    _name, _maxRunDurationMs);
    }
  }
}

void ActuatorDriver::turnOn() {
  if (_isOn) return;
  digitalWrite(_pin, HIGH);
  _isOn = true;
  _turnOnTimestamp = millis();
}

void ActuatorDriver::turnOff() {
  if (!_isOn) return;
  digitalWrite(_pin, LOW);
  _isOn = false;
  _totalRunDurationMs += (millis() - _turnOnTimestamp);
}

bool ActuatorDriver::isOn() const {
  return _isOn;
}

bool ActuatorDriver::isWatchdogTriggered() const {
  return _watchdogTriggered;
}

void ActuatorDriver::clearWatchdog() {
  _watchdogTriggered = false;
}

unsigned long ActuatorDriver::getCurrentRunDurationMs() const {
  if (!_isOn) return 0;
  return millis() - _turnOnTimestamp;
}

unsigned long ActuatorDriver::getTotalRunDurationMs() const {
  return _totalRunDurationMs;
}

const char* ActuatorDriver::getName() const {
  return _name;
}
