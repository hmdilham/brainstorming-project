# Perbandingan Sensor YHDC untuk Proyek NILM
**Tanggal riset**: 2026-04-27  
**Konteks**: ESP32 ADC 12-bit (0–3.3V), 220V AC 50Hz, fitur: P, Q, Inrush_ratio, H3(150Hz), H5(250Hz), H7(350Hz), H9(450Hz)

---

## 1. Sensor Arus — CT Series (Split-Core Current Transformer)

| Spec | SCT-013 | SCT-016 | SCT-019 | SCT-024TS | SCT-036TS |
|---|---|---|---|---|---|
| **Aperture** | 13mm | 16mm | 19×19mm | 24mm | 36mm |
| **Rated Input** | 5–100A | 10–120A | 50–200A | 50–400A | 50–600A |
| **Output** | 50mA atau 0.333V/1V | 10–100mA atau 0.333V/1V/3V | 33mA atau 0.333V | 0.333V/1V/3V | 5A/1A/0.1A atau 0.333V |
| **Core Material** | Ferrite | Ferrite | **Permalloy** | Silicon steel (laminasi) | Silicon steel |
| **Bandwidth** | 50Hz–1kHz | 50Hz–1kHz | **20Hz–1kHz** | 50–60Hz saja | 50–60Hz saja |
| **H9 (450Hz)** | ✅ PASS | ✅ PASS | ✅ PASS | ❌ GAGAL | ❌ GAGAL |
| **Phase error @50Hz** | 1–4° | ≤10° (typical ±1°) | 2.6° (158 arcmin) | +0.5±0.5° | Tidak dispec |
| **Akurasi** | 1% | 0.5–1% | 0.5–1% | 0.2–1% | 1% |
| **Linearitas** | ≤0.2% | ≤0.2% | ≤0.2% | ≤0.2% | - |
| **Interface ESP32 3.3V** | Perlu bias 1.65V + voltage divider | Mudah (0.333V variant + bias) | Mudah (0.333V variant + bias) | Perlu redesain circuit | Tidak cocok |
| **Cocok 0.1–16A?** | Ya (pilih varian 15A atau 30A) | Ya (varian 10A) | Kurang (mulai 50A, low-current buruk) | Tidak | Tidak |
| **Standar** | IEC 61869-2 | IEC 61869-2 | GB20840-2014 | IEC 61869-2 | - |
| **Harga estimasi** | $3–8 | $8–20 | $10–25 | $15–30 | $20–50 |
| **URL produk** | http://en.yhdc.com/product/SCT013-401.html | allDatasheet | http://en.yhdc.com/product/380.html | poweruc.com | manuals.plus |

### Rekomendasi varian SCT untuk NILM:
- **SCT-013-030** (30A/1V): beban rumah tangga 0.5–30A, output 1Vrms → biaskan ke 1.65V
- **SCT-013-015** (15A/1V): untuk beban kecil (charger, lampu), resolusi lebih baik
- **SCT-016-010** (10A/0.333V): paling mudah interface ESP32, langsung + bias

---

## 2. Sensor Arus — HSTS016L (Hall Effect, Split-Core)

| Spec | HSTS016L |
|---|---|
| **Tipe** | Open-loop Hall effect, split-core |
| **Rated Input** | ±10A, ±20A, ±30A, ±50A, ±100A, ±150A, ±200A (bidirectional AC+DC) |
| **Output (3.3V supply)** | **1.65V ± 0.625V** (zero=1.65V, full-scale ±0.625V) |
| **Supply voltage** | +5V standar atau **+3.3V (varian khusus)** |
| **Bandwidth** | **DC sampai 25kHz** — jauh melampaui H9 (450Hz) |
| **H3–H9 capable?** | ✅ PASS semua harmoni dengan margin besar |
| **Phase error** | Sangat kecil (<0.1° estimasi) — Hall langsung sensing medan, bukan transformasi |
| **Response time** | ≤5µs (setara >200kHz bandwidth impuls) |
| **Zero offset** | ≤±15mV |
| **Akurasi** | 1% |
| **Linearitas** | ≤0.1% |
| **Deteksi DC/inrush** | ✅ Ya — CT tidak bisa deteksi DC |
| **Interface ESP32 3.3V** | **Langsung tanpa komponen tambahan** (varian 3.3V) |
| **Load impedance** | ≥10kΩ (cocok dengan ADC ESP32) |
| **Temperature drift** | ±0.02%/°C |
| **Isolasi dielektrik** | 2.5kV 50Hz 1 menit |
| **Aperture** | Φ16mm |
| **Operating temp** | -10°C sampai +70°C |
| **Harga estimasi** | $12–20 |
| **URL produk** | http://en.yhdc.com/product_detail.html?productId=103 |

---

## 3. Sensor Tegangan — ZMPT101B

| Spec | ZMPT101B (transformator bare) | ZMPT101B (modul dengan LM358) |
|---|---|---|
| **Tipe** | Current-type voltage transformer | Modul dengan amplifier LM358 |
| **Input voltage** | 0–250V AC | 0–250V AC |
| **Rasio lilitan** | 1000:1000 (1:1 current ratio) | Sama |
| **Akurasi kelas** | 0.2 | ~3% praktis |
| **Phase error @50Hz** | ≤20 arcmin (≤0.33°) dengan burden 50Ω | Bervariasi, degradasi |
| **Bandwidth** | Beberapa kHz (estimasi) | **Terbatas LM358** — distorsi harmonik |
| **H3–H9 capable?** | Kemungkinan ya (dengan op-amp wideband custom) | ❌ TIDAK — LM358 mendistorsi gelombang |
| **Interface ESP32 3.3V** | Perlu custom circuit | Bisa 3.3V VCC, output centered 1.65V |
| **Isolasi** | 4000V withstand | 4000V withstand |
| **Harga** | ~$0.50–1 (bare) | ~$1.50–4 (modul) |

### Catatan penting ZMPT101B:
> Modul ZMPT101B standar (dengan LM358) **TIDAK COCOK** untuk pengukuran harmonik karena LM358 mendistorsi waveform. Untuk NILM dengan H3–H9, gunakan salah satu:
> 1. Bare transformer + op-amp rail-to-rail wideband (≥1kHz BW)
> 2. Resistor divider + isolation amplifier (AMC1211)
> 3. Asumsikan V=220V konstan dan hanya ukur arus (simplifikasi umum NILM)

---

## 4. Perbandingan Bandwidth untuk Harmonik NILM

| Sensor | H3 (150Hz) | H5 (250Hz) | H7 (350Hz) | H9 (450Hz) | Phase @ harmonik |
|---|---|---|---|---|---|
| SCT-013 (ferrite, 1kHz) | ✅ | ✅ | ✅ | ✅ | Sedang — degradasi ringan naik frekuensi |
| SCT-016 (ferrite, 1kHz) | ✅ | ✅ | ✅ | ✅ | Sedang — max 10° (spec) |
| SCT-019 (permalloy, 1kHz) | ✅ | ✅ | ✅ | ✅ | Baik — permalloy lebih stabil di frekuensi tinggi |
| SCT-024TS (silicon steel) | ❌ | ❌ | ❌ | ❌ | Tidak dirating |
| SCT-036TS (silicon steel) | ❌ | ❌ | ❌ | ❌ | Tidak dirating |
| **HSTS016L (Hall, 25kHz)** | ✅✅ | ✅✅ | ✅✅ | ✅✅ | **Sangat baik — phase error mendekati nol** |
| ZMPT101B modul | ❌ | ❌ | ❌ | ❌ | Distorsi LM358 |
| ZMPT101B bare transformer | ✅ (est.) | ✅ (est.) | ✅ (est.) | ✅ (est.) | 0.33° @50Hz (Class 0.2) |

---

## 5. Kompatibilitas Interface ESP32 ADC

ESP32 ADC: 12-bit, range optimal 0.1–2.4V (non-linier di atas 2.4V), best accuracy di 0.1–2.4V.

| Sensor | Kerumitan Interface | Output Range | Verdict |
|---|---|---|---|
| **SCT-013-030 (30A/1V)** | Sedang — perlu bias 1.65V + R divider | 1Vrms → 2.83Vpp, center 1.65V (peak: 0.24V–3.06V, sangat mepet) | Perlu hati-hati amplitude |
| **SCT-016 (0.333V)** | Mudah — bias + langsung | 0.333Vrms = 0.94Vpp; aman dalam 3.3V | Direkomendasikan |
| **SCT-019 (0.333V)** | Mudah tapi resolusi rendah | 0.333Vrms @200A; sangat kecil di arus rendah | Tidak cocok untuk <5A |
| **HSTS016L (3.3V variant)** | **Paling mudah — langsung ke ADC** | 1.025V–2.275V (zero=1.65V, ±0.625V) | **Ideal langsung connect** |
| ZMPT101B modul | Sedang — tuning potensiometer | 0–3.3V; tune amplitudo | Hanya untuk Vrms fundamental |

---

## 6. Rekomendasi Final

### Sensor Arus Terbaik: **HSTS016L (varian 30A, supply 3.3V)**
**Alasan:**
1. Bandwidth DC–25kHz — H3, H5, H7, H9 semua tercover dengan margin 55x
2. Phase error mendekati nol pada 50–450Hz → P dan Q lebih akurat
3. Varian 3.3V: output 1.65V ± 0.625V → langsung ke GPIO35/GPIO34 ESP32 tanpa komponen tambahan
4. Deteksi DC dan inrush → fitur Inrush_ratio lebih akurat
5. Range ±30A cukup untuk beban rumah tangga 0.1A–16A
6. Response time ≤5µs → tidak ada delay phase yang signifikan

**Kekurangan:** Drift temperatur ±0.02%/°C; open-loop Hall kurang stabil dibanding precision CT. Tapi untuk NILM fingerprinting (bukan billing meter), ini tidak masalah — rasio harmonik relatif lebih penting dari nilai absolut.

### Sensor Arus Pilihan Kedua: **SCT-019 (permalloy, 0.333V output)** — hanya jika arus >5A
**Alasan:** Core permalloy lebih baik dari ferrite untuk harmonik; bandwidth 20Hz–1kHz; phase error 2.6° dapat dikompensasi software.  
**Batasan:** Rating minimum 50A — resolusi buruk untuk beban kecil (<5A).

### Sensor Arus Hindari untuk NILM Harmonik:
- SCT-024TS, SCT-036TS: bandwidth 50–60Hz saja, harmoni tidak terdeteksi
- SCT-013-100A: resolusi rendah di arus kecil, gunakan varian 15A/30A

### Sensor Tegangan: **ZMPT101B bare transformer + custom op-amp** atau **skip jika asumsi V=220V konstan**

---

## 7. URL Referensi Produk

| Produk | URL |
|---|---|
| SCT-013 (YHDC) | http://en.yhdc.com/product/SCT013-401.html |
| SCT-019 (YHDC) | http://en.yhdc.com/product/380.html |
| HSTS016L (YHDC) | http://en.yhdc.com/product_detail.html?productId=103 |
| HSTS016L (PowerUC, spec detail) | https://www.poweruc.pl/products/hall-split-core-current-sensor-hsts016l-rated-input-10-200a-rated-output-2-5-0-625v |
| SCT-016 (PowerUC) | https://www.poweruc.pl/products/split-core-current-transformer-sct016-rated-input-120a |
| SCT-019 (PowerUC) | https://www.poweruc.pl/products/split-core-current-transformer-sct019-rated-input-200a-rated-output-0-333v |
| SCT-024TS | https://poweruc.com/product/current-transformers-sct024ts |
| OpenEnergyMonitor SCT-013 Report | https://docs.openenergymonitor.org/electricity-monitoring/ct-sensors/yhdc-sct-013-000-ct-sensor-report.html |
| ZMPT101B Distortion Study (PMC) | https://pmc.ncbi.nlm.nih.gov/articles/PMC9611784/ |

> **Catatan**: en.yhdc.com memiliki SSL certificate expired per April 2026. Akses via HTTP (bukan HTTPS).

---

*Riset: Claude Sonnet 4.6 | 2026-04-27*
