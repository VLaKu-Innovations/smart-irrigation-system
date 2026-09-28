#include "irrigation_system.h"

IrrigationSystem::IrrigationSystem(ActuatorDriver& pump, ZoneManager& zone1, ZoneManager& zone2)
  : _pump(pump), _zone1(zone1), _zone2(zone2),
    _state(FSM_STATE_IDLE), _activeZone(nullptr),
    _transitionTimestamp(0), _wateringStartTimestamp(0), _lastTelemetryTimestamp(0) {}

void IrrigationSystem::begin() {
  _pump.begin();
  _zone1.begin();
  _zone2.begin();
  _state = FSM_STATE_IDLE;
  _activeZone = nullptr;
}

void IrrigationSystem::update() {
  _pump.update();
  _zone1.update();
  _zone2.update();

  executeSequencing();

  // Periodic Telemetry Logging (every 5 seconds)
  if (millis() - _lastTelemetryTimestamp >= SENSOR_SAMPLE_INTERVAL_MS) {
    _lastTelemetryTimestamp = millis();
    printStatusReport();
  }
}

void IrrigationSystem::executeSequencing() {
  unsigned long now = millis();

  switch (_state) {
    case FSM_STATE_IDLE: {
      // Priority check: Zone 1 first, then Zone 2
      if (_zone1.needsWater()) {
        _activeZone = &_zone1;
      } else if (_zone2.needsWater()) {
        _activeZone = &_zone2;
      } else {
        _activeZone = nullptr;
      }

      if (_activeZone != nullptr) {
        Serial.printf("[SYSTEM] Initiating watering for Zone %d (Moisture: %.1f%%)...\n",
                      _activeZone->getZoneId(), _activeZone->getAverageMoisture());
        _activeZone->openValve(); // 1. Open valve FIRST
        _transitionTimestamp = now;
        _state = FSM_STATE_VALVE_OPENING;
      }
      break;
    }

    case FSM_STATE_VALVE_OPENING: {
      // 2. Wait 500ms anti-hammer buffer for valve to seat open
      if (now - _transitionTimestamp >= ANTI_HAMMER_DELAY_MS) {
        Serial.println("[SYSTEM] Valve open. Starting 12V Diaphragm Pump...");
        _pump.turnOn();
        _wateringStartTimestamp = now;
        _state = FSM_STATE_WATERING;
      }
      break;
    }

    case FSM_STATE_WATERING: {
      if (_activeZone == nullptr) {
        _state = FSM_STATE_PUMP_STOPPING;
        break;
      }

      unsigned long duration = now - _wateringStartTimestamp;
      bool satisfied = _activeZone->isSatisfied();
      bool maxTimeReached = (duration >= MAX_WATERING_RUN_MS);
      bool watchdogAlert = _pump.isWatchdogTriggered();

      if (satisfied || maxTimeReached || watchdogAlert) {
        Serial.printf("[SYSTEM] Terminating watering for Zone %d. Reason: %s (Duration: %lu ms, Moisture: %.1f%%)\n",
                      _activeZone->getZoneId(),
                      satisfied ? "TARGET_REACHED" : (maxTimeReached ? "MAX_TIME_SAFETY_CUTOFF" : "WATCHDOG_TRIP"),
                      duration, _activeZone->getAverageMoisture());
        _pump.turnOff(); // 3. Stop pump FIRST
        _transitionTimestamp = now;
        _state = FSM_STATE_PUMP_STOPPING;
      }
      break;
    }

    case FSM_STATE_PUMP_STOPPING: {
      // 4. Wait 500ms anti-hammer buffer for line pressure to normalize
      if (now - _transitionTimestamp >= ANTI_HAMMER_DELAY_MS) {
        Serial.printf("[SYSTEM] Line pressure normalized. Closing Zone %d valve...\n",
                      _activeZone ? _activeZone->getZoneId() : 0);
        if (_activeZone != nullptr) {
          _activeZone->completeWateringCycle();
        }
        _activeZone = nullptr;
        _state = FSM_STATE_IDLE;
      }
      break;
    }

    default:
      _state = FSM_STATE_IDLE;
      _activeZone = nullptr;
      break;
  }
}

SystemFSMState IrrigationSystem::getState() const {
  return _state;
}

int IrrigationSystem::getActiveZoneId() const {
  return _activeZone ? _activeZone->getZoneId() : 0;
}

void IrrigationSystem::printStatusReport() const {
  const char* stateNames[] = {"IDLE", "VALVE_OPENING", "WATERING", "PUMP_STOPPING", "VALVE_CLOSING"};
  Serial.printf("[%7lu s] Z1: %4.1f%% (%s) | Z2: %4.1f%% (%s) | Pump: %s | State: %s\n",
                millis() / 1000,
                _zone1.getAverageMoisture(),
                _zone1.isLockout() ? "FAULT" : (_zone1.isInCooldown() ? "SOAK" : (_zone1.needsWater() ? "DRY" : "OK")),
                _zone2.getAverageMoisture(),
                _zone2.isLockout() ? "FAULT" : (_zone2.isInCooldown() ? "SOAK" : (_zone2.needsWater() ? "DRY" : "OK")),
                _pump.isOn() ? "ON" : "OFF",
                stateNames[_state]);
}
