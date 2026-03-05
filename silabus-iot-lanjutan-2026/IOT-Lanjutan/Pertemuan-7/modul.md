# Modul Pertemuan 7 — IOT Lanjutan
# MQTT Part 1 — Arsitektur Pub/Sub & Setup Broker Mosquitto

---

## 1. Maksud dan Tujuan Materi

### Maksud
MQTT (Message Queuing Telemetry Transport) adalah protokol standar IoT berbasis Publish/Subscribe. Berbeda dari HTTP yang request-response, MQTT memungkinkan komunikasi event-driven yang ringan dan efisien. Pertemuan ini mencakup teori MQTT, instalasi broker Mosquitto, dan pengujian dengan tools MQTT Explorer.

### Tujuan Pembelajaran
1. **Menjelaskan** arsitektur Pub/Sub: Publisher, Subscriber, Broker, Topic.
2. **Menginstal** MQTT Broker Mosquitto di komputer lokal.
3. **Menggunakan** MQTT Explorer dan CLI tools untuk publish/subscribe.
4. **Memahami** QoS levels, Retained Messages, dan Last Will & Testament.

---

## 2. Teori Materi

### 2.1 Mengapa MQTT, Bukan HTTP?

| Aspek | HTTP | **MQTT** |
|---|---|---|
| Model | Request-Response | **Publish/Subscribe** |
| Overhead | Tinggi (headers besar) | **Sangat rendah (2 byte header min)** |
| Koneksi | Short-lived per request | **Persistent (keep-alive)** |
| Push data | Harus polling | **Push real-time!** |
| Bandwidth | Tinggi | **Sangat hemat** |
| IoT devices | Ratusan | **Jutaan** |

### 2.2 Arsitektur MQTT

```
  Publisher                Broker                 Subscriber
  (ESP32 sensor)          (Mosquitto)            (Dashboard)

  ┌──────────┐    PUBLISH    ┌──────────┐    FORWARD    ┌──────────┐
  │  ESP32   ├──────────────►│  Broker  ├──────────────►│  Client  │
  │  sensor  │  topic:       │ Mosquitto│              │  MQTT    │
  │          │  "iot/suhu"   │          │              │  Explorer│
  └──────────┘  payload:     └──────────┘              └──────────┘
                "28.5"            ▲
                            SUBSCRIBE │
                          topic: "iot/suhu"
```

### 2.3 Konsep Utama

**Topic:** Hierarkis, dipisah `/`. Contoh: `rumah/kamar/suhu`, `iot/node1/data`
- Wildcard `+`: satu level. `rumah/+/suhu` → match `rumah/kamar/suhu`, `rumah/dapur/suhu`
- Wildcard `#`: semua level. `rumah/#` → match semua di bawah `rumah/`

**QoS (Quality of Service):**
- QoS 0: Fire and forget (mungkin hilang)
- QoS 1: At least once (bisa duplikat)
- QoS 2: Exactly once (paling reliable, paling lambat)

**Retained Message:** Broker menyimpan pesan terakhir di topic. Client baru langsung dapat nilai terakhir.

**LWT (Last Will & Testament):** Pesan yang otomatis di-publish broker jika client disconnect tidak wajar.

---

## 3. Panduan Praktikum

### Praktikum 3.1 — Instalasi Mosquitto

**Linux:**
```bash
sudo apt install mosquitto mosquitto-clients
sudo systemctl enable mosquitto
sudo systemctl start mosquitto
```

**macOS:**
```bash
brew install mosquitto
brew services start mosquitto
```

**Windows:** Download installer dari [mosquitto.org/download](https://mosquitto.org/download/)

**Konfigurasi (`/etc/mosquitto/mosquitto.conf`):**
```
listener 1883
allow_anonymous true
```

Restart: `sudo systemctl restart mosquitto`

### Praktikum 3.2 — Test CLI

```bash
# Terminal 1: Subscribe
mosquitto_sub -h localhost -t "test/hello" -v

# Terminal 2: Publish
mosquitto_pub -h localhost -t "test/hello" -m "Hello MQTT!"

# Test Retain
mosquitto_pub -h localhost -t "test/retain" -m "Data tersimpan" -r

# Test QoS
mosquitto_pub -h localhost -t "test/qos" -m "QoS 1" -q 1
```

### Praktikum 3.3 — MQTT Explorer

1. Download MQTT Explorer: [mqtt-explorer.com](https://mqtt-explorer.com/)
2. Connect ke `localhost:1883`
3. Publish manual ke topic → lihat data real-time
4. Test retained messages, QoS levels

---

## 4. Referensi

1. **HiveMQ.** *MQTT Essentials.* [https://www.hivemq.com/mqtt-essentials/](https://www.hivemq.com/mqtt-essentials/)
2. **Mosquitto.** [https://mosquitto.org/](https://mosquitto.org/)
3. **MQTT Explorer.** [https://mqtt-explorer.com/](https://mqtt-explorer.com/)
