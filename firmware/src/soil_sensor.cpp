#include "soil_sensor.h"

SoilSensor::SoilSensor(int pin, int airVal, int waterVal, const char* name)
  : _pin(pin), _airVal(airVal), _waterVal(waterVal), _name(name),
    _lastRawADC(0), _smoothedPercent(50.0f), _isFaulty(false) {}

void SoilSensor::begin() {
  pinMode(_pin, INPUT);
  // Prime the filter with initial reading
  _lastRawADC = readOversampledADC();
  _smoothedPercent = rawToPercent(_lastRawADC);
}

void SoilSensor::update() {
  _lastRawADC = readOversampledADC();

  // Validate range for fault detection
  if (_lastRawADC < SENSOR_FAULT_MIN_ADC || _lastRawADC > SENSOR_FAULT_MAX_ADC) {
    _isFaulty = true;
  } else {
    _isFaulty = false;
    float currentPct = rawToPercent(_lastRawADC);
    // Exponential Moving Average (EMA) smoothing
    _smoothedPercent = (SENSOR_EMA_ALPHA * currentPct) + ((1.0f - SENSOR_EMA_ALPHA) * _smoothedPercent);
  }
}

int SoilSensor::readOversampledADC() {
  long sum = 0;
  for (int i = 0; i < OVERSAMPLE_BURST_COUNT; i++) {
    sum += analogRead(_pin);
    delayMicroseconds(250);
  }
  return static_cast<int>(sum / OVERSAMPLE_BURST_COUNT);
}

float SoilSensor::rawToPercent(int raw) {
  if (_airVal == _waterVal) return 0.0f;
  float pct = ((float)(_airVal - raw) / (float)(_airVal - _waterVal)) * 100.0f;
  if (pct < 0.0f) pct = 0.0f;
  if (pct > 100.0f) pct = 100.0f;
  return pct;
}

float SoilSensor::getMoisturePercent() const {
  return _smoothedPercent;
}

int SoilSensor::getRawADC() const {
  return _lastRawADC;
}

float SoilSensor::getVoltage() const {
  return (_lastRawADC * 3.3f) / 4095.0f;
}

bool SoilSensor::isFaulty() const {
  return _isFaulty;
}

const char* SoilSensor::getName() const {
  return _name;
}

void SoilSensor::setCalibration(int airVal, int waterVal) {
  _airVal = airVal;
  _waterVal = waterVal;
}
