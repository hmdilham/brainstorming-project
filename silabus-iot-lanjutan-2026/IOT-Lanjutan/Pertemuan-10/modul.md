# Modul Pertemuan 10 — IOT Lanjutan
# Home Assistant — Automasi, Dashboard & Integrasi MQTT

---

## 1. Maksud dan Tujuan Materi

### Maksud
Home Assistant menjadi pusat kendali seluruh sistem IoT. Pertemuan ini membahas pembuatan automasi YAML/UI, dashboard Lovelace yang cantik, integrasi MQTT auto-discovery dari ESP32 PubSubClient, serta notifikasi ke Telegram/Companion App.

### Tujuan Pembelajaran
1. **Membuat** automasi HA: trigger + condition + action (YAML dan UI).
2. **Membangun** dashboard Lovelace: gauge, graph, entity, button.
3. **Mengintegrasikan** ESP32 PubSubClient → HA MQTT auto-discovery.
4. **Mengirim** notifikasi dari HA ke Telegram.

---

## 2. Praktikum

### 2.1 — HA Automasi Lengkap

```yaml
# Automasi 1: Nyalakan kipas saat panas + matikan saat dingin
automation:
  - alias: "Kipas Otomatis"
    trigger:
      - platform: numeric_state
        entity_id: sensor.suhu_custom
        above: 30
    condition:
      - condition: state
        entity_id: switch.relay_custom
        state: "off"
    action:
      - service: switch.turn_on
        target:
          entity_id: switch.relay_custom
      - service: notify.notify
        data:
          title: "IoT Alert 🌡️"
          message: "Suhu {{ states('sensor.suhu_custom') }}°C — Kipas dinyalakan!"

  - alias: "Kipas Mati saat Sejuk"
    trigger:
      - platform: numeric_state
        entity_id: sensor.suhu_custom
        below: 27
    action:
      - service: switch.turn_off
        target:
          entity_id: switch.relay_custom

  # Automasi 2: LED jadwal (nyala malam, mati pagi)
  - alias: "LED Malam"
    trigger:
      - platform: time
        at: "18:00:00"
    action:
      - service: switch.turn_on
        target:
          entity_id: switch.led_custom

  - alias: "LED Pagi"
    trigger:
      - platform: time
        at: "06:00:00"
    action:
      - service: switch.turn_off
        target:
          entity_id: switch.led_custom
```

### 2.2 — Dashboard Lovelace

```yaml
# Lovelace dashboard config (UI atau YAML mode)
views:
  - title: IoT Dashboard
    cards:
      # Gauge suhu
      - type: gauge
        entity: sensor.suhu_custom
        name: "Suhu Ruangan"
        min: 0
        max: 50
        severity:
          green: 0
          yellow: 28
          red: 35

      # Gauge kelembaban
      - type: gauge
        entity: sensor.kelembaban_custom
        name: "Kelembaban"
        min: 0
        max: 100

      # Grafik historis
      - type: history-graph
        title: "24 Jam Terakhir"
        hours_to_show: 24
        entities:
          - entity: sensor.suhu_custom
            name: "Suhu"
          - entity: sensor.kelembaban_custom
            name: "Kelembaban"

      # Kontrol switch
      - type: entities
        title: "Kontrol Perangkat"
        entities:
          - entity: switch.led_custom
            name: "LED Ruangan"
            icon: "mdi:led-on"
          - entity: switch.relay_custom
            name: "Relay Kipas"
            icon: "mdi:fan"

      # Status
      - type: entities
        title: "Status Sistem"
        entities:
          - entity: sensor.wifi_signal
          - entity: sensor.uptime
          - entity: sensor.boot_counter
```

### 2.3 — ESP32 PubSubClient → HA MQTT Auto-Discovery

Agar ESP32 yang menggunakan PubSubClient (bukan ESPHome) otomatis muncul di HA:

```cpp
// Snippet: Kirim MQTT Discovery config saat connect
void publishDiscovery() {
  // Temperature sensor discovery
  JsonDocument doc;
  doc["name"] = "ESP32 Suhu";
  doc["stat_t"] = "iot/kamar/sensor";      // State topic
  doc["val_tpl"] = "{{ value_json.suhu }}"; // Value template
  doc["unit_of_meas"] = "°C";
  doc["dev_cla"] = "temperature";
  doc["uniq_id"] = "esp32_suhu_01";
  doc["dev"]["ids"] = "esp32_01";
  doc["dev"]["name"] = "ESP32 IoT";
  doc["dev"]["mf"] = "Espressif";

  String payload;
  serializeJson(doc, payload);
  mqtt.publish("homeassistant/sensor/esp32_suhu/config", payload.c_str(), true);
}
```

### 2.4 — Notifikasi Telegram

1. Buat bot Telegram via @BotFather → dapatkan token
2. Di HA `configuration.yaml`:
```yaml
telegram_bot:
  - platform: polling
    api_key: "YOUR_BOT_TOKEN"
    allowed_chat_ids:
      - 123456789  # Chat ID Anda

notify:
  - platform: telegram
    name: "telegram_notify"
    chat_id: 123456789
```

3. Gunakan dalam automasi:
```yaml
action:
  - service: notify.telegram_notify
    data:
      message: "⚠️ Suhu ruangan mencapai {{ states('sensor.suhu_custom') }}°C!"
```

---

## 3. Referensi

1. **Home Assistant.** *Automations.* [https://www.home-assistant.io/docs/automation/](https://www.home-assistant.io/docs/automation/)
2. **Home Assistant.** *Lovelace.* [https://www.home-assistant.io/dashboards/](https://www.home-assistant.io/dashboards/)
3. **Home Assistant.** *MQTT Discovery.* [https://www.home-assistant.io/integrations/mqtt/#mqtt-discovery](https://www.home-assistant.io/integrations/mqtt/#mqtt-discovery)
4. **Home Assistant.** *Telegram.* [https://www.home-assistant.io/integrations/telegram/](https://www.home-assistant.io/integrations/telegram/)
