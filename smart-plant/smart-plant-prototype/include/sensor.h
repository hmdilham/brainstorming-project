#pragma once
#include <Arduino.h>
#include "config.h"

class SoilSensor {
public:
  void begin();
  void update();             // non-blocking, call setiap loop()
  int  getPercent() const;   // 0–100 (clamped). Bila invalid, nilai cache terakhir tetap dikembalikan
  int  getRaw()     const;   // raw ADC value (0–1023)
  bool isValid()    const;   // false bila raw di luar kalibrasi ± SOIL_VALID_MARGIN (sensor lepas/short)
  void setCalibration(int dryRaw, int wetRaw);

private:
  int           _dryRaw  = SOIL_RAW_DRY;
  int           _wetRaw  = SOIL_RAW_WET;
  int           _buf[SAMPLE_COUNT] = {};
  int           _idx     = 0;
  int           _raw     = 0;
  int           _pct     = 0;
  bool          _valid   = false;
  unsigned long _lastMs  = 0;

  void _recomputeValid();
};
