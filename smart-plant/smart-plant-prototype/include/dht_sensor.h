#pragma once
#include <Arduino.h>
#include <DHT.h>
#include "config.h"

// Wrapper non-blocking untuk DHT22. Update dipanggil tiap loop(),
// pembacaan aktual dibatasi DHT_READ_INTERVAL (≥ 2 detik untuk DHT22).
class DhtSensor {
public:
  void  begin();
  void  update();              // non-blocking, panggil di loop()

  float getTemperatureC() const;   // °C — NAN jika invalid
  float getHumidity()     const;   // % RH — NAN jika invalid
  bool  isValid()         const;   // true bila pembacaan terakhir sukses

private:
  DHT           _dht{PIN_DHT22, DHT22};
  float         _tempC   = NAN;
  float         _humPct  = NAN;
  bool          _valid   = false;
  unsigned long _lastMs  = 0;
};
