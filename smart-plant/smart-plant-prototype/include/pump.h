#pragma once
#include <Arduino.h>
#include "config.h"

enum class PumpState : uint8_t {
  IDLE      = 0,   // monitoring, pompa OFF
  WATERING  = 1,   // pompa ON — auto mode
  COOLDOWN  = 2,   // jeda setelah siram
  MANUAL_ON = 3    // pompa ON — dipaksa dari web/MQTT
};

class PumpController {
public:
  void  begin();
  void  update(int soilPct, bool soilValid);  // state machine — panggil setiap loop();
                                              // soilValid=false: stop watering auto, blok auto-start (MANUAL_ON tetap jalan)

  // Manual override
  void  forceOn(unsigned long durationMs = 10000UL);
  void  forceOff();

  bool        isOn()          const;
  PumpState   getState()      const;
  const char* getStateStr()   const;
  unsigned long msSinceChange() const;

  // Override defaults dari config.json
  void setThresholds(int dryPct, int wetPct);
  void setTimings(unsigned long minOnMs, unsigned long maxOnMs, unsigned long cooldownMs);

private:
  PumpState     _state        = PumpState::IDLE;
  bool          _relayOn      = false;
  unsigned long _stateStart   = 0;
  unsigned long _manualEndMs  = 0;
  unsigned long _minOnMs      = PUMP_MIN_ON_MS;
  unsigned long _maxOnMs      = PUMP_MAX_ON_MS;
  unsigned long _cooldownMs   = COOLDOWN_MS;
  int           _dryThreshold = SOIL_DRY_THRESHOLD;
  int           _wetThreshold = SOIL_WET_THRESHOLD;

  void _setRelay(bool on);
};
