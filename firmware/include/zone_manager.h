#pragma once

#include <Arduino.h>
#include "config.h"
#include "soil_sensor.h"
#include "actuator_driver.h"

enum ZoneStatus {
  ZONE_STATUS_IDLE,
  ZONE_STATUS_WATERING,
  ZONE_STATUS_SOAKING,
  ZONE_STATUS_LOCKOUT
};

class ZoneManager {
public:
  ZoneManager(int zoneId, SoilSensor& sensorA, SoilSensor& sensorB, ActuatorDriver& valve);

  void begin();
  void update();

  bool needsWater() const;
  bool isSatisfied() const;
  bool isInCooldown() const;
  bool isLockout() const;

  void openValve();
  void closeValve();
  void completeWateringCycle();

  float getAverageMoisture() const;
  ZoneStatus getStatus() const;
  int getZoneId() const;
  ActuatorDriver& getValve();

private:
  int _zoneId;
  SoilSensor& _sensorA;
  SoilSensor& _sensorB;
  ActuatorDriver& _valve;

  ZoneStatus _status;
  float _averageMoisture;
  unsigned long _soakUntilTimestamp;
};
