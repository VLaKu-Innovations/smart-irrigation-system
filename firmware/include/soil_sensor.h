#pragma once

#include <Arduino.h>
#include "config.h"

class SoilSensor {
public:
  SoilSensor(int pin, int airVal, int waterVal, const char* name);

  void begin();
  void update();

  float getMoisturePercent() const;
  int getRawADC() const;
  float getVoltage() const;
  bool isFaulty() const;
  const char* getName() const;

  void setCalibration(int airVal, int waterVal);

private:
  int _pin;
  int _airVal;
  int _waterVal;
  const char* _name;

  int _lastRawADC;
  float _smoothedPercent;
  bool _isFaulty;

  int readOversampledADC();
  float rawToPercent(int raw);
};
