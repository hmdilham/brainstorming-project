#include "dht_sensor.h"

void DhtSensor::begin() {
  _dht.begin();
  // Pembacaan pertama sengaja ditunda — DHT22 butuh ~1 detik setelah power-on
  // untuk stabil. Loop pertama akan mengisi nilai begitu DHT_READ_INTERVAL terlewati.
  _lastMs = millis();
}

void DhtSensor::update() {
  if (millis() - _lastMs < DHT_READ_INTERVAL) return;
  _lastMs = millis();

  float t = _dht.readTemperature();   // °C
  float h = _dht.readHumidity();      // %

  if (isnan(t) || isnan(h)) {
    _valid = false;
    return;
  }

  _tempC  = t;
  _humPct = h;
  _valid  = true;
}

float DhtSensor::getTemperatureC() const { return _tempC; }
float DhtSensor::getHumidity()     const { return _humPct; }
bool  DhtSensor::isValid()         const { return _valid; }
