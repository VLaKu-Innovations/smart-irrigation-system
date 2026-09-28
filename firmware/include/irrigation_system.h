#pragma once

#include <Arduino.h>
#include "config.h"
#include "actuator_driver.h"
#include "zone_manager.h"

enum SystemFSMState {
  FSM_STATE_IDLE,
  FSM_STATE_VALVE_OPENING,
  FSM_STATE_WATERING,
  FSM_STATE_PUMP_STOPPING,
  FSM_STATE_VALVE_CLOSING
};

class IrrigationSystem {
public:
  IrrigationSystem(ActuatorDriver& pump, ZoneManager& zone1, ZoneManager& zone2);

  void begin();
  void update();

  SystemFSMState getState() const;
  int getActiveZoneId() const;
  void printStatusReport() const;

private:
  ActuatorDriver& _pump;
  ZoneManager& _zone1;
  ZoneManager& _zone2;

  SystemFSMState _state;
  ZoneManager* _activeZone;
  unsigned long _transitionTimestamp;
  unsigned long _wateringStartTimestamp;
  unsigned long _lastTelemetryTimestamp;

  void executeSequencing();
};
