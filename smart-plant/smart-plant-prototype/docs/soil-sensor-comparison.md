# Komparasi Soil Sensor — Pasar Indonesia

> **Riset desk per April 2026.** Harga adalah rentang dari Tokopedia/Shopee (top seller) dan distributor lokal. Verifikasi ulang sebelum keputusan pembelian — harga marketplace sangat fluktuatif.

Lihat [`soil-sensor-comparison-plan.md`](./soil-sensor-comparison-plan.md) untuk metodologi.

---

## Ringkasan Rekomendasi

| Tier Produk | Sensor Pilihan | Alasan Singkat |
|-------------|----------------|----------------|
| **Prototype** (PoC, ESP8266) | Capacitive Soil Moisture v1.2 | Murah, output analog 0–3V, kompatibel A0, sudah cocok dengan kode `SoilSensor` saat ini |
| **Free Edition** (consumer indoor) | Capacitive v2.0 (waterproof PCB) | Stabilitas lebih baik, board ter-coating tahan kelembapan, harga masih < Rp 50 rb |
| **Pro Edition** (outdoor / serius) | RS485 4-in-1 industrial (JXBS / S-Soil MTEC-02A / generic Modbus) | IP68, multi-parameter (moisture+temp+EC), butuh upgrade ke ESP32 |

---

## Tabel Komparasi Lengkap

### Hobbyist Tier

| Spec | YL-69 / FC-28 (Resistive) | Capacitive v1.2 | Capacitive v2.0 |
|------|---------------------------|-----------------|-----------------|
| **Tipe** | Resistive (2 probe besi) | Capacitive (single PCB probe) | Capacitive (single PCB, improved) |
| **Tegangan** | 3.3–5V | 3.3–5.5V | 3.3–5.5V |
| **Output** | Analog + Digital (LM393) | Analog 0–3V | Analog 0–3V |
| **Akurasi** | Rendah, drift cepat karena korosi | Sedang–baik | Baik |
| **Anti-korosi** | ❌ Tidak (probe besi karatan dlm minggu) | ✅ Ya (PCB tertutup) | ✅ Ya + PCB ter-coating |
| **IP rating** | Tidak ada | Tidak (board exposed) | Sebagian (board coating) |
| **Lifetime** | 1–3 bulan kontak terus | 6–12 bulan | 12–24 bulan |
| **Harga (Rp)** | 8.000–25.000 | 15.000–35.000 (varian premium s/d 60.000) | 25.000–50.000 |
| **Ketersediaan ID** | Sangat banyak | Sangat banyak | Banyak |
| **ESP8266 ready?** | ✅ A0 langsung | ✅ A0 langsung (ideal) | ✅ A0 langsung |
| **Catatan** | Hindari untuk produk akhir, hanya untuk eksperimen | Pilihan paling balanced | Worth ekstra Rp 10–20 rb dibanding v1.2 |

### Mid Tier

| Spec | DFRobot SEN0193 Gravity | Capacitive Waterproof Probe Panjang |
|------|-------------------------|-------------------------------------|
| **Tipe** | Capacitive (genuine DFRobot) | Capacitive variant (probe lebih panjang, board terlindung housing) |
| **Tegangan** | 3.3–5.5V | 3.3–5.5V |
| **Output** | Analog 0–3V (PH2.0-3P) | Analog 0–3V |
| **Akurasi** | Baik, kalibrasi vendor | Sedang–baik |
| **Anti-korosi** | ✅ Ya | ✅ Ya |
| **IP rating** | Probe ya, board exposed | Probe + board sebagian terlindung |
| **Lifetime** | 2+ tahun (klaim vendor) | 12–24 bulan |
| **Harga (Rp)** | 100.000–180.000 (genuine, sebagian seller s/d 375.000) | 60.000–120.000 |
| **Ketersediaan ID** | Sedang (NA Robotic, Eiot, akhi_shop) | Banyak |
| **ESP8266 ready?** | ✅ A0 langsung | ✅ A0 langsung |
| **Catatan** | Premium hobbyist, dokumentasi resmi DFRobot wiki | Cocok untuk pot besar / pemasangan dalam |

### Industrial Tier (RS485 / Modbus)

| Spec | JXBS-3001-TR (4–7 in 1) | Seeed S-Soil MTEC-02A | NPK 7-in-1 generic |
|------|-------------------------|-----------------------|-------------------|
| **Parameter** | Moisture + Temp + EC + pH (+ NPK opsional) | Moisture + Temp + EC | Moisture + Temp + EC + pH + N + P + K |
| **Tegangan** | 5–30V (butuh PSU 12V) | 5–30V | 5–30V |
| **Output** | RS485 / Modbus RTU | RS485 / Modbus RTU | RS485 / Modbus RTU |
| **IP rating** | IP68 (probe stainless) | IP68 | IP68 |
| **Akurasi** | ±2–3% moisture, ±0.3°C temp | ±2% moisture, ±0.5°C temp | Bervariasi per vendor |
| **Lifetime** | 5+ tahun field use | 5+ tahun | 3–5 tahun |
| **Harga (Rp)** | 800.000–1.500.000 (Indo); ~Rp 540 rb di India | 1.200.000–2.000.000 | 950.000–8.500.000 (varies wildly) |
| **Ketersediaan ID** | Sedang (import via Tokopedia/Shopee) | Sedang | Banyak (Makmurindo, Boga Jaya, Microthings) |
| **ESP8266 ready?** | ⚠️ Butuh MAX485 + SoftwareSerial (kurang stabil) | ⚠️ Sama | ⚠️ Sama |
| **ESP32 ready?** | ✅ UART2 hardware | ✅ | ✅ |
| **Catatan** | Best value untuk industrial | Reputasi vendor lebih baik | Hati-hati klon murah dengan akurasi NPK diragukan |

### Spesialis / Agriculture-grade

| Spec | Watermark 200SS (Tensiometer) |
|------|-------------------------------|
| **Tipe** | Granular matrix tensiometer (resistive AC) |
| **Output** | Resistance (butuh data logger atau AM400) |
| **Range** | 0–239 cb (kPa) — ukur water tension, bukan % moisture |
| **Lifetime** | 5+ tahun (industri terbukti sejak 1978) |
| **Harga (Rp)** | ~650.000 (sensor saja, ~$40 USD) + biaya import; reader/logger terpisah ratusan ribu lagi |
| **Ketersediaan ID** | Tidak retail; harus import dari distributor luar |
| **Use case** | Irigasi presisi pertanian skala besar — **overkill untuk Smart Plant home** |
| **Catatan** | Skip untuk produk consumer. Bahan referensi kalau mau ekspansi ke agritech. |

---

## Rekomendasi Detail per Tier

### 1. Prototype — Capacitive Soil Moisture v1.2

- **Harga target**: Rp 20.000–30.000.
- **Alasan**: Sudah dipakai di kode (`src/sensor.cpp`, pin A0, threshold dry/wet di `include/config.h`). Tidak perlu refactor.
- **Tindakan**: Beli 3–5 unit untuk eksperimen (variasi seller untuk uji konsistensi sample-to-sample).
- **Risiko**: Drift kalibrasi karena variasi manufaktur Cina — wajib `setCalibration()` per unit.

### 2. Free Edition — Capacitive v2.0

- **Harga target**: Rp 35.000–50.000.
- **Alasan**: Upgrade kecil dari v1.2, board ter-coating jauh lebih tahan kondensasi indoor jangka panjang. Selisih harga < Rp 20 rb tapi lifetime 2x lipat.
- **Tindakan**: Setelah PoC sukses, ganti BOM dari v1.2 ke v2.0. Kode tidak berubah (sama-sama analog 0–3V).
- **Alternatif**: Capacitive waterproof variant (probe panjang) untuk pot besar.

### 3. Pro Edition — RS485 4-in-1 (JXBS-3001-TR atau S-Soil MTEC-02A)

- **Harga target**: Rp 800.000–1.500.000.
- **Alasan**: Multi-parameter (moisture+temp+EC), IP68, lifetime 5+ tahun. EC penting untuk indikator pemupukan.
- **Tindakan**:
  1. **Migrasi MCU ke ESP32** (UART2 hardware mandatory untuk RS485 stabil).
  2. Tambah module MAX485 (Rp 15.000) untuk konversi UART ↔ RS485.
  3. Library: `ModbusMaster` atau `eModbus`.
  4. Skip ESP8266 untuk tier ini.
- **Tidak rekomendasi**: NPK 7-in-1 generic Cina < Rp 2 jt — akurasi NPK biasanya tidak terkalibrasi, hanya proxy konduktivitas. Kalau perlu NPK serius, beli yang > Rp 3 jt dengan kalibrasi pabrik.

---

## Catatan Operasional

### Kalibrasi (semua sensor analog)

Setiap unit perlu 2-titik kalibrasi:

1. **Dry**: sensor di udara terbuka → catat raw ADC sebagai `SOIL_RAW_DRY`.
2. **Wet**: sensor di air bersih → catat raw ADC sebagai `SOIL_RAW_WET`.

Update via web UI `/api/config` atau hardcode di `include/config.h`. Sudah didukung di `SoilSensor::setCalibration()`.

### Strategi Sourcing

- **Volume kecil (< 10 unit)**: Tokopedia/Shopee, ambil seller ⭐4.8+ dengan banyak ulasan.
- **Volume produksi (> 50 unit)**: Aliexpress (lead time 2–4 minggu) atau langsung kontak distributor lokal seperti Ichibot, JogjaRobotika, Makmurindo Jaya Perkasa.
- **Industrial RS485**: Microthings, BogaJayaAbadi, atau import langsung dari JXCT/Seeed.

### Failure Modes yang Sering Terjadi

| Sensor | Mode Kegagalan Umum |
|--------|---------------------|
| Resistive YL-69 | Probe karatan dalam 2–8 minggu, output drift jadi tinggi semua |
| Capacitive v1.2 | Air masuk ke board exposed → korslet, NaN output |
| Capacitive v2.0 | Coating retak setelah > 1 tahun outdoor → mirip v1.2 |
| RS485 generic | Komunikasi corrupt jika kabel > 50m tanpa termination resistor 120Ω |

---

## Sources

### Tokopedia listings
- [Capacitive Soil Moisture (search)](https://www.tokopedia.com/find/capacitive-soil-moisture)
- [Soil Moisture Sensor (search)](https://www.tokopedia.com/find/soil-moisture-sensor)
- [Soil Sensor 7-in-1 RS485 — Makmurindo Jaya Perkasa](https://www.tokopedia.com/makmurindojayaperkasa/soil-sensor-7-in-1-rs485-alat-uji-tanah-npk-ph-ec-suhu-kelembapan-1731509703618561809)
- [DFRobot Capacitive — NA Robotic](https://www.tokopedia.com/nasrula/dfrobot-capacitive-soil-moisture-sensor)
- [DFRobot Capacitive Corrosion Resistant — Eiot](https://www.tokopedia.com/eiot/dfrobot-analog-capacitive-soil-moisture-sensor-corrosion-resistant)
- [YL-69 / FC-28 — Prima Terang](https://www.tokopedia.com/primaterang/sensor-kelembaban-tanah-soil-moisture-hygrometer-module-fc-28-yl-69)
- [Soil moisture sensor v1.2 — DT Production](https://www.tokopedia.com/dtproduction/soil-moisture-sensor-v1-2-modul-sensor-kelembaban-tanah-high-quality)

### Distributor lokal
- [Ichibot Store — Capacitive Soil Moisture](https://store.ichibot.id/product/capacitive-soil-moisture-sensor-kelembaban-tanah/)
- [Jogja Robotika — Capacitive v1.2](https://jogjarobotika.com/sensor-temperatur/3003-analog-capacitive-soil-moisture-sensor-v12-sensor-kelembaban-tanah-v12.html)
- [Microthings — Soil NPK Sensor](https://www.microthings.id/product/soil-npk-sensor/)
- [Darmasakti — RS485 7-in-1](https://darmasakti.com/jual/rs485-soil-npk-ph-ec-temp-humidity-sensor-tester-7-in-1-soil)

### Datasheet & dokumentasi
- [DFRobot SEN0193 wiki](https://wiki.dfrobot.com/Capacitive_Soil_Moisture_Sensor_SKU_SEN0193)
- [DFRobot RS485 4-in-1](https://www.dfrobot.com/product-2830.html)
- [Seeed S-Soil MTEC-02A](https://solution.seeedstudio.com/product/industrial-soil-moisture-temperature-ec-sensor-modbus-rtu-rs485-s-soil-mtec-02a/)
- [JXBS-3001-TR datasheet](https://5.imimg.com/data5/SELLER/Doc/2022/6/IB/TY/WK/5551405/soil-sensor-jxbs-3001-tr-rs485-2.pdf)
- [Irrometer Watermark 200SS](https://www.irrometer.com/200ss.html)
