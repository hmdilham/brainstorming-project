"""
ESP32 Communication - Serial dan WiFi WebSocket client.
Mengirim perintah ON/OFF LED ke ESP32.

Mode: FIRE AND FORGET — kirim command, track state lokal.
Tidak perlu baca response dari ESP32 (menghindari serial read issues).
"""

import json
import time
import threading


class ESP32Serial:
    """
    Komunikasi ke ESP32 via Serial UART.
    Fire-and-forget: kirim command tanpa menunggu response.
    State di-track secara lokal.
    """
    
    def __init__(self, port="/dev/ttyUSB0", baud=115200):
        self.port = port
        self.baud = baud
        self.serial_conn = None
        self.connected = False
        self.last_status = {"led": False}
        self._lock = threading.Lock()
    
    def connect(self):
        """Buka koneksi Serial ke ESP32."""
        try:
            import serial
            self.serial_conn = serial.Serial(
                port=self.port,
                baudrate=self.baud,
                timeout=0.1
            )
            time.sleep(2)  # Tunggu ESP32 reset
            
            # Flush semua data lama
            self.serial_conn.reset_input_buffer()
            self.serial_conn.reset_output_buffer()
            time.sleep(0.5)
            while self.serial_conn.in_waiting:
                self.serial_conn.read(self.serial_conn.in_waiting)
                time.sleep(0.05)
            
            self.connected = True
            
            # Pastikan LED mulai dari OFF
            self._write("LED_OFF")
            self.last_status = {"led": False}
            
            print(f"[Serial] Connected to {self.port} @ {self.baud}")
            return True
        except Exception as e:
            print(f"[Serial] Connection failed: {e}")
            self.connected = False
            return False
    
    def _write(self, command):
        """Kirim command via serial (fire-and-forget)."""
        if not self.serial_conn or not self.connected:
            return False
        try:
            with self._lock:
                self.serial_conn.write(f"{command}\n".encode())
                self.serial_conn.flush()
                # Buang response agar buffer tidak penuh
                time.sleep(0.02)
                if self.serial_conn.in_waiting > 0:
                    self.serial_conn.read(self.serial_conn.in_waiting)
            return True
        except Exception as e:
            print(f"[Serial] Write error: {e}")
            self.connected = False
            return False
    
    def toggle(self):
        """Toggle LED — kirim ON/OFF eksplisit berdasarkan state lokal."""
        new_state = not self.last_status.get("led", False)
        cmd = "LED_ON" if new_state else "LED_OFF"
        if self._write(cmd):
            self.last_status["led"] = new_state
            print(f"[Serial] Sent: {cmd}")
        return self.last_status.copy()
    
    def set_led(self, state):
        """Set LED ke state tertentu."""
        cmd = "LED_ON" if state else "LED_OFF"
        if self._write(cmd):
            self.last_status["led"] = state
        return self.last_status.copy()
    
    def get_status(self):
        """Ambil status LED (lokal)."""
        return self.last_status.copy()
    
    def disconnect(self):
        """Tutup koneksi Serial."""
        if self.serial_conn:
            try:
                self._write("LED_OFF")
                self.serial_conn.close()
            except:
                pass
        self.connected = False
        print("[Serial] Disconnected")


class ESP32WebSocket:
    """Komunikasi ke ESP32 via WiFi WebSocket (fire-and-forget)."""
    
    def __init__(self, ip="192.168.1.200", port=81):
        self.ip = ip
        self.port = port
        self.ws = None
        self.connected = False
        self.last_status = {"led": False}
        self._lock = threading.Lock()
    
    def connect(self):
        """Buka koneksi WebSocket ke ESP32."""
        try:
            import websocket
            url = f"ws://{self.ip}:{self.port}"
            print(f"[WebSocket] Connecting to {url}...")
            
            self.ws = websocket.WebSocket()
            self.ws.settimeout(2)
            self.ws.connect(url)
            self.connected = True
            
            # Baca initial status (opsional)
            try:
                initial = self.ws.recv()
                self.last_status = json.loads(initial)
            except:
                pass
            
            print(f"[WebSocket] Connected!")
            return True
        except Exception as e:
            print(f"[WebSocket] Connection failed: {e}")
            self.connected = False
            return False
    
    def toggle(self):
        """Toggle LED."""
        new_state = not self.last_status.get("led", False)
        action = "on" if new_state else "off"
        try:
            with self._lock:
                self.ws.send(json.dumps({"action": action}))
                self.last_status["led"] = new_state
        except Exception as e:
            print(f"[WebSocket] Error: {e}")
            self.connected = False
        return self.last_status.copy()
    
    def set_led(self, state):
        """Set LED ke state tertentu."""
        action = "on" if state else "off"
        try:
            with self._lock:
                self.ws.send(json.dumps({"action": action}))
                self.last_status["led"] = state
        except Exception as e:
            print(f"[WebSocket] Error: {e}")
            self.connected = False
        return self.last_status.copy()
    
    def get_status(self):
        return self.last_status.copy()
    
    def disconnect(self):
        """Tutup koneksi WebSocket."""
        if self.ws:
            try:
                self.ws.close()
            except:
                pass
        self.connected = False
        print("[WebSocket] Disconnected")


class ESP32Dummy:
    """Dummy communicator untuk testing tanpa hardware."""
    
    def __init__(self):
        self.connected = True
        self.last_status = {"led": False}
    
    def connect(self):
        print("[Dummy] Simulated ESP32 connection")
        return True
    
    def toggle(self):
        self.last_status["led"] = not self.last_status["led"]
        state = "ON" if self.last_status["led"] else "OFF"
        print(f"[Dummy] LED {state}")
        return self.last_status.copy()
    
    def set_led(self, state):
        self.last_status["led"] = state
        return self.last_status.copy()
    
    def get_status(self):
        return self.last_status.copy()
    
    def disconnect(self):
        print("[Dummy] Disconnected")


def create_communicator(mode="serial", **kwargs):
    """Factory function untuk membuat communicator."""
    if mode == "serial":
        return ESP32Serial(
            port=kwargs.get("port", "/dev/ttyUSB0"),
            baud=kwargs.get("baud", 115200)
        )
    elif mode == "wifi":
        return ESP32WebSocket(
            ip=kwargs.get("ip", "192.168.1.200"),
            port=kwargs.get("port", 81)
        )
    elif mode == "dummy":
        return ESP32Dummy()
    else:
        raise ValueError(f"Unknown mode: {mode}. Use 'serial', 'wifi', or 'dummy'")
