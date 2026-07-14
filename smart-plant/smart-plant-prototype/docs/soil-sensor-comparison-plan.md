# Rencana Perbandingan Soil Sensor (Pasar Indonesia)

Dokumen ini adalah **rencana** (bukan hasil). Hasil komparasi final ada di
[`soil-sensor-comparison.md`](./soil-sensor-comparison.md).

> **Status revisi**: revisi pasca-riset desk April 2026 — kandidat dan kriteria diperbarui berdasarkan temuan harga & spec lapangan.

## 1. Tujuan

Memilih soil moisture sensor terbaik untuk 3 tier produk Smart Plant:

| Tier | Target Pengguna | Constraint Utama |
|------|-----------------|------------------|
| **Prototype** | PoC internal (USB-only) | Murah, mudah, kompatibel ESP8266 ADC |
| **Free Edition** | Consumer indoor | Tahan 6–12 bulan, plug-and-play, < Rp 50 rb |
| **Pro Edition** | Hobbyist outdoor / mini greenhouse | Tahan > 2 tahun, akurat, multi-parameter opsional |

## 2. Kriteria Perbandingan

| Kategori | Parameter |
|----------|-----------|
| **Teknis** | Tipe (resistive/capacitive/TDR/RS485), output (analog/I2C/RS485), tegangan kerja, range output, akurasi, response time |
| **Daya tahan** | Material probe, anti-korosi, IP rating, lifetime estimasi |
| **Integrasi** | Pin yang dipakai, library ESP8266, kalibrasi (auto/manual) |
| **Komersial** | Harga (Rp), stok di Tokopedia/Shopee, garansi, lead time |
| **Ekstra** | Multi-parameter (NPK/EC/temp/pH), waterproof board |

## 3. Kandidat Sensor (short-list pasca-riset)

### Tier hobbyist (< Rp 50 rb)
- Resistive YL-69 / FC-28 / HL-69 (~Rp 8–25 rb) — **hanya untuk eksperimen, tidak untuk produk akhir**
- Capacitive Soil Moisture v1.2 (~Rp 15–35 rb) — **kandidat Prototype**
- Capacitive Soil Moisture v2.0 (~Rp 25–50 rb) — **kandidat Free Edition**

### Tier mid (Rp 50 rb – 400 rb)
- DFRobot SEN0193 / SEN0308 Gravity (Rp 100–180 rb genuine; sebagian seller s/d Rp 375 rb)
- Capacitive waterproof probe panjang (Rp 60–120 rb)

### Tier industrial (> Rp 800 rb)
- JXBS-3001-TR RS485 4-in-1 (Rp 800 rb–1,5 jt) — **kandidat Pro Edition**
- Seeed S-Soil MTEC-02A (Rp 1,2–2 jt)
- NPK 7-in-1 generic RS485 (Rp 950 rb–8,5 jt) — **hati-hati klon murah, akurasi NPK diragukan di harga < Rp 3 jt**

### Dihapus dari short-list
- ~~Watermark 200SS tensiometer~~ — overkill untuk consumer Smart Plant, butuh data logger terpisah, tidak retail di Indonesia. Tetap referensi untuk ekspansi ke agritech presisi di kemudian hari.

## 4. Metode Pengumpulan Data

1. **Riset desk** ✅ selesai April 2026 — hasil di `soil-sensor-comparison.md`.
2. **Eksperimen lab** (opsional, belum dijalankan)
   - Uji 3–5 sampel terpotensi di tanah kering, lembab, basah.
   - Catat raw ADC + drift selama 7 hari direndam.
   - Acuan: gravimetric water content (timbang basah/kering).
3. **Field test** (opsional, belum dijalankan)
   - Pasang 2 minggu di pot tanaman nyata.

### Distributor lokal yang teridentifikasi (untuk sourcing volume)
- Hobbyist: Ichibot Store, Jogja Robotika, Tokopedia top-seller (Prima Terang, Serial-Electronic, DT Production).
- Mid (DFRobot genuine): NA Robotic, Eiot, akhi_shop.
- Industrial RS485: Makmurindo Jaya Perkasa, Boga Jaya Abadi Teknik, Microthings, Darmasakti.

## 5. Output Deliverable

| File | Status |
|------|--------|
| `docs/soil-sensor-comparison-plan.md` | ← Dokumen ini |
| `docs/soil-sensor-comparison.md` | Tabel komparasi + rekomendasi per tier |
| `docs/soil-sensor-test-log.csv` | (opsional) Data eksperimen lab |

## 6. Rekomendasi Final (sudah dikonfirmasi via riset)

- **Prototype**: Capacitive v1.2 — sudah cocok dengan kode `SoilSensor` analog di `src/sensor.cpp`. Beli 3–5 unit untuk uji konsistensi sample-to-sample.
- **Free**: Capacitive v2.0 (board ter-coating). Selisih harga < Rp 20 rb dari v1.2 untuk lifetime 2x lipat. Drop-in replacement, kode tidak berubah.
- **Pro**: JXBS-3001-TR RS485 4-in-1 atau Seeed S-Soil MTEC-02A. **Wajib upgrade MCU ke ESP32** (UART2 hardware) + tambah modul MAX485.

Detail lengkap, harga per tier, dan failure mode lihat [`soil-sensor-comparison.md`](./soil-sensor-comparison.md).

## 7. Catatan Teknis ESP8266

- ESP8266 hanya punya **1 ADC pin (A0)** dengan range 0–1.0V (board NodeMCU sudah ada divider ke 3.3V).
- Sensor analog harus output 0–3.3V agar terbaca penuh.
- RS485 butuh MAX485 + UART hardware → ESP8266 hanya punya 1 UART (Serial), bentrok dengan upload firmware. Solusi: SoftwareSerial (kurang reliable) atau pindah ke ESP32.
- I2C soil sensor (jarang) bisa jadi opsi premium tanpa habiskan ADC.
