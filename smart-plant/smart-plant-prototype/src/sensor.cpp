#include "sensor.h"

void SoilSensor::begin() {
  // Pre-fill buffer dengan pembacaan awal agar avg langsung valid
  int first = analogRead(PIN_SOIL_ADC);
  for (int i = 0; i < SAMPLE_COUNT; i++) _buf[i] = first;
  _raw = first;
  _pct = constrain(map(first, _dryRaw, _wetRaw, 0, 100), 0, 100);
  _recomputeValid();
}

void SoilSensor::update() {
  if (millis() - _lastMs < SENSOR_INTERVAL) return;
  _lastMs = millis();

  // Simpan ke circular buffer
  _buf[_idx] = analogRead(PIN_SOIL_ADC);
  _idx = (_idx + 1) % SAMPLE_COUNT;

  // Hitung rata-rata
  long sum = 0;
  for (int i = 0; i < SAMPLE_COUNT; i++) sum += _buf[i];
  _raw = (int)(sum / SAMPLE_COUNT);

  // Map ke persentase, clamp 0–100
  _pct = constrain(map(_raw, _dryRaw, _wetRaw, 0, 100), 0, 100);

  _recomputeValid();
}

int  SoilSensor::getPercent() const { return _pct; }
int  SoilSensor::getRaw()     const { return _raw; }
bool SoilSensor::isValid()    const { return _valid; }

void SoilSensor::setCalibration(int dryRaw, int wetRaw) {
  _dryRaw = dryRaw;
  _wetRaw = wetRaw;
  _recomputeValid();
}

// Sensor capacitive yang sehat menghasilkan raw di rentang [wetRaw, dryRaw]
// (urut tergantung tipe sensor — pakai min/max biar aman).
// Pin floating saat sensor dicabut → raw ~0 → di luar rentang → invalid.
// Short / kabel putus → raw 0 atau 1023 → invalid.
void SoilSensor::_recomputeValid() {
  int lo = (_wetRaw < _dryRaw ? _wetRaw : _dryRaw) - SOIL_VALID_MARGIN;
  int hi = (_wetRaw > _dryRaw ? _wetRaw : _dryRaw) + SOIL_VALID_MARGIN;
  _valid = (_raw >= lo) && (_raw <= hi);
}
