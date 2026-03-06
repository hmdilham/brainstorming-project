/*
 * Hand Landmark Virtual Button - ESP32 LED Controller
 * 
 * Firmware untuk menerima perintah ON/OFF LED dari Python
 * melalui Serial UART dan/atau WiFi WebSocket.
 * 
 * Serial Commands: LED_ON, LED_OFF, LED_TOGGLE, LED_STATUS
 * WebSocket JSON:  {"action": "on"}, {"action": "off"}, 
 *                  {"action": "toggle"}, {"action": "status"}
 */

#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"

#if ENABLE_WIFI
#include <WiFi.h>
#include <WebSocketsServer.h>
#endif

// ============================================
// Global Variables
// ============================================
bool ledState = false;
String serialBuffer = "";

#if ENABLE_WIFI
WebSocketsServer webSocket(WS_PORT);
bool wifiConnected = false;
#endif

// ============================================
// LED Control Functions
// ============================================
void setLed(bool state) {
    ledState = state;
    digitalWrite(LED_PIN, state ? LED_ON_STATE : !LED_ON_STATE);
}

void toggleLed() {
    setLed(!ledState);
}

// Kirim status LED sebagai JSON
String getLedStatusJson() {
    JsonDocument doc;
    doc["led"] = ledState;
    doc["pin"] = LED_PIN;
    String output;
    serializeJson(doc, output);
    return output;
}

// ============================================
// Serial Command Handler
// ============================================
#if ENABLE_SERIAL
void handleSerialCommand(String cmd) {
    cmd.trim();
    cmd.toUpperCase();
    
    if (cmd == "LED_ON") {
        setLed(true);
        Serial.println(getLedStatusJson());
    } 
    else if (cmd == "LED_OFF") {
        setLed(false);
        Serial.println(getLedStatusJson());
    } 
    else if (cmd == "LED_TOGGLE") {
        toggleLed();
        Serial.println(getLedStatusJson());
    } 
    else if (cmd == "LED_STATUS") {
        Serial.println(getLedStatusJson());
    }
    else if (cmd.length() > 0) {
        Serial.println("{\"error\": \"Unknown command: " + cmd + "\"}");
    }
}

void processSerial() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            if (serialBuffer.length() > 0) {
                handleSerialCommand(serialBuffer);
                serialBuffer = "";
            }
        } else {
            serialBuffer += c;
        }
    }
}
#endif

// ============================================
// WebSocket Handler
// ============================================
#if ENABLE_WIFI
void handleWebSocketCommand(uint8_t clientNum, String payload) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    
    if (error) {
        webSocket.sendTXT(clientNum, "{\"error\": \"Invalid JSON\"}");
        return;
    }
    
    String action = doc["action"].as<String>();
    
    if (action == "on") {
        setLed(true);
    } 
    else if (action == "off") {
        setLed(false);
    } 
    else if (action == "toggle") {
        toggleLed();
    } 
    else if (action == "status") {
        // Hanya kirim status tanpa mengubah state
    }
    else {
        webSocket.sendTXT(clientNum, "{\"error\": \"Unknown action\"}");
        return;
    }
    
    // Kirim status ke semua client yang terhubung
    String status = getLedStatusJson();
    webSocket.broadcastTXT(status);
    
    // Juga kirim ke Serial untuk logging
    #if ENABLE_SERIAL
    Serial.print("[WS] ");
    Serial.println(status);
    #endif
}

void webSocketEvent(uint8_t clientNum, WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            Serial.printf("[WS] Client #%u disconnected\n", clientNum);
            break;
            
        case WStype_CONNECTED: {
            IPAddress ip = webSocket.remoteIP(clientNum);
            Serial.printf("[WS] Client #%u connected from %s\n", clientNum, ip.toString().c_str());
            // Kirim status saat client connect
            webSocket.sendTXT(clientNum, getLedStatusJson());
            break;
        }
        
        case WStype_TEXT:
            handleWebSocketCommand(clientNum, String((char *)payload));
            break;
            
        default:
            break;
    }
}

void setupWiFi() {
    Serial.print("[WiFi] Connecting to ");
    Serial.println(WIFI_SSID);
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    WiFi.setAutoReconnect(true);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.println();
        Serial.print("[WiFi] Connected! IP: ");
        Serial.println(WiFi.localIP());
        Serial.printf("[WiFi] WebSocket server on port %d\n", WS_PORT);
        
        webSocket.begin();
        webSocket.onEvent(webSocketEvent);
    } else {
        Serial.println();
        Serial.println("[WiFi] Connection FAILED - running Serial-only mode");
    }
}
#endif

// ============================================
// Setup & Loop
// ============================================
void setup() {
    // Initialize Serial
    Serial.begin(SERIAL_BAUD);
    delay(1000);
    
    Serial.println("====================================");
    Serial.println("  Hand Landmark LED Controller");
    Serial.println("  ESP32 Firmware v1.0");
    Serial.println("====================================");
    
    // Initialize LED
    pinMode(LED_PIN, OUTPUT);
    setLed(false);
    Serial.printf("[LED] Initialized on GPIO %d\n", LED_PIN);
    
    // Initialize WiFi + WebSocket
    #if ENABLE_WIFI
    setupWiFi();
    #else
    Serial.println("[WiFi] Disabled in config");
    #endif
    
    Serial.println("[Ready] Waiting for commands...");
    Serial.println("  Serial: LED_ON, LED_OFF, LED_TOGGLE, LED_STATUS");
    #if ENABLE_WIFI
    if (wifiConnected) {
        Serial.println("  WebSocket: {\"action\": \"on/off/toggle/status\"}");
    }
    #endif
    Serial.println("====================================");
}

void loop() {
    // Process Serial commands
    #if ENABLE_SERIAL
    processSerial();
    #endif
    
    // Process WebSocket events
    #if ENABLE_WIFI
    if (wifiConnected) {
        webSocket.loop();
    }
    #endif
}
