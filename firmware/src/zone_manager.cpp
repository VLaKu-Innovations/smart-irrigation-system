#include "zone_manager.h"

ZoneManager::ZoneManager(int zoneId, SoilSensor& sensorA, SoilSensor& sensorB, ActuatorDriver& valve)
  : _zoneId(zoneId), _sensorA(sensorA), _sensorB(sensorB), _valve(valve),
    _status(ZONE_STATUS_IDLE), _averageMoisture(50.0f), _soakUntilTimestamp(0) {}

void ZoneManager::begin() {
  _sensorA.begin();
  _sensorB.begin();
  _valve.begin();
  _status = ZONE_STATUS_IDLE;
}

void ZoneManager::update() {
  _sensorA.update();
  _sensorB.update();
  _valve.update();

  // Evaluate sensor health and compute average
  bool aFaulty = _sensorA.isFaulty();
  bool bFaulty = _sensorB.isFaulty();

  if (aFaulty && bFaulty) {
    _status = ZONE_STATUS_LOCKOUT;
    return;
  } else if (aFaulty) {
    _averageMoisture = _sensorB.getMoisturePercent();
  } else if (bFaulty) {
    _averageMoisture = _sensorA.getMoisturePercent();
  } else {
    _averageMoisture = (_sensorA.getMoisturePercent() + _sensorB.getMoisturePercent()) / 2.0f;
  }

  // Handle Soak Cooldown Transition
  if (_status == ZONE_STATUS_SOAKING) {
    if (millis() >= _soakUntilTimestamp) {
      _status = ZONE_STATUS_IDLE;
      Serial.printf("[ZONE %d] Soil soak cooldown complete. Returning to IDLE.\n", _zoneId);
    }
  }
}

bool ZoneManager::needsWater() const {
  if (_status == ZONE_STATUS_LOCKOUT || _status == ZONE_STATUS_SOAKING || _status == ZONE_STATUS_WATERING) {
    return false;
  }
  return (_averageMoisture < MOISTURE_TRIGGER_LOW_PCT);
}

bool ZoneManager::isSatisfied() const {
  return (_averageMoisture >= MOISTURE_SATISFIED_HIGH_PCT);
}

bool ZoneManager::isInCooldown() const {
  return (_status == ZONE_STATUS_SOAKING);
}

bool ZoneManager::isLockout() const {
  return (_status == ZONE_STATUS_LOCKOUT);
}

void ZoneManager::openValve() {
  _valve.turnOn();
  _status = ZONE_STATUS_WATERING;
}

void ZoneManager::closeValve() {
  _valve.turnOff();
}

void ZoneManager::completeWateringCycle() {
  closeValve();
  _status = ZONE_STATUS_SOAKING;
  _soakUntilTimestamp = millis() + SOAK_COOLDOWN_MS;
  Serial.printf("[ZONE %d] Watering cycle finished. Entering soak cooldown for %lu seconds.\n",
                _zoneId, SOAK_COOLDOWN_MS / 1000);
}

float ZoneManager::getAverageMoisture() const {
  return _averageMoisture;
}

ZoneStatus ZoneManager::getStatus() const {
  return _status;
}

int ZoneManager::getZoneId() const {
  return _zoneId;
}

ActuatorDriver& ZoneManager::getValve() {
  return _valve;
}
