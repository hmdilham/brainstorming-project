#include "pump.h"

void PumpController::begin() {
  pinMode(PIN_RELAY, OUTPUT);
  _setRelay(false);  // pastikan pompa mati saat boot
}

void PumpController::_setRelay(bool on) {
  _relayOn = on;
  // Relay active LOW: LOW = coil energized = pompa ON
  digitalWrite(PIN_RELAY, on ? LOW : HIGH);
}

void PumpController::update(int soilPct, bool soilValid) {
  unsigned long now     = millis();
  unsigned long elapsed = now - _stateStart;

  // Safety: kalau sensor lepas saat WATERING auto → stop segera (tidak ada feedback
  // untuk tahu kapan harus berhenti, lebih aman cut). MANUAL_ON dibiarkan karena user
  // eksplisit minta dan punya tombol stop sendiri.
  if (!soilValid && _state == PumpState::WATERING) {
    Serial.println("[Pump] STOP — sensor invalid (disconnected?), aborting auto-watering");
    _setRelay(false);
    _stateStart = now;
    _state = PumpState::COOLDOWN;
    return;
  }

  switch (_state) {

    case PumpState::MANUAL_ON:
      // Tunggu hingga durasi manual selesai
      if (now >= _manualEndMs) {
        _setRelay(false);
        _stateStart = now;
        _state = PumpState::COOLDOWN;
      }
      break;

    case PumpState::IDLE:
      // Mulai siram jika tanah kering melewati threshold — TAPI hanya kalau sensor valid
      if (soilValid && soilPct < _dryThreshold) {
        _setRelay(true);
        _stateStart = now;
        _state = PumpState::WATERING;
        Serial.printf("[Pump] START — soil %d%% < %d%%\n", soilPct, _dryThreshold);
      }
      break;

    case PumpState::WATERING: {
      bool wetEnough  = (soilPct > _wetThreshold);
      bool minElapsed = (elapsed >= _minOnMs);
      bool maxElapsed = (elapsed >= _maxOnMs);

      if (maxElapsed) {
        // Safety cutoff — pompa terlalu lama
        Serial.printf("[Pump] SAFETY cutoff after %lu ms\n", elapsed);
        _setRelay(false);
        _stateStart = now;
        _state = PumpState::COOLDOWN;
      } else if (wetEnough && minElapsed) {
        // Tanah sudah cukup basah DAN minimal ON sudah terpenuhi
        Serial.printf("[Pump] STOP — soil %d%% > %d%%\n", soilPct, _wetThreshold);
        _setRelay(false);
        _stateStart = now;
        _state = PumpState::COOLDOWN;
      }
      break;
    }

    case PumpState::COOLDOWN:
      if (elapsed >= _cooldownMs) {
        _state = PumpState::IDLE;
        Serial.println("[Pump] COOLDOWN done → IDLE");
      }
      break;
  }
}

void PumpController::forceOn(unsigned long durationMs) {
  Serial.printf("[Pump] FORCE ON for %lu ms\n", durationMs);
  _setRelay(true);
  _manualEndMs = millis() + durationMs;
  _stateStart  = millis();
  _state       = PumpState::MANUAL_ON;
}

void PumpController::forceOff() {
  Serial.println("[Pump] FORCE OFF");
  _setRelay(false);
  _stateStart = millis();
  _state      = PumpState::COOLDOWN;
}

bool        PumpController::isOn()          const { return _relayOn; }
PumpState   PumpController::getState()      const { return _state; }
unsigned long PumpController::msSinceChange() const { return millis() - _stateStart; }

const char* PumpController::getStateStr() const {
  switch (_state) {
    case PumpState::IDLE:      return "IDLE";
    case PumpState::WATERING:  return "WATERING";
    case PumpState::COOLDOWN:  return "COOLDOWN";
    case PumpState::MANUAL_ON: return "MANUAL";
    default:                   return "UNKNOWN";
  }
}

void PumpController::setThresholds(int dryPct, int wetPct) {
  _dryThreshold = dryPct;
  _wetThreshold = wetPct;
}

void PumpController::setTimings(unsigned long minOnMs, unsigned long maxOnMs, unsigned long cooldownMs) {
  _minOnMs    = minOnMs;
  _maxOnMs    = maxOnMs;
  _cooldownMs = cooldownMs;
}
