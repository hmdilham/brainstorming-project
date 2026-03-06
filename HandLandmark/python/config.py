"""
Konfigurasi untuk Hand Landmark Virtual Button
Sesuaikan nilai-nilai di bawah ini dengan setup Anda.
"""

# ============================================
# Camera Configuration (DroidCam)
# ============================================
# Opsi 1: DroidCam via USB (biasanya index 0 atau 1)
#CAMERA_INDEX = 0

# Opsi 2: DroidCam via WiFi (uncomment dan isi IP DroidCam)
CAMERA_INDEX = "http://192.168.100.20:4747/video"

# Resolusi kamera
CAMERA_WIDTH = 640
CAMERA_HEIGHT = 480

# ============================================
# Communication Mode
# ============================================
# Pilih: "serial" atau "wifi"
COMM_MODE = "serial"

# ============================================
# Serial Configuration
# ============================================
SERIAL_PORT = "/dev/ttyUSB0"    # Linux: /dev/ttyUSB0 atau /dev/ttyACM0
                                 # Windows: COM3, COM4, dst.
SERIAL_BAUD = 115200

# ============================================
# WiFi Configuration (ESP32 WebSocket)
# ============================================
ESP32_IP = "192.168.1.200"       # IP address ESP32 (lihat di Serial Monitor)
ESP32_WS_PORT = 81

# ============================================
# Virtual Button Configuration
# ============================================
# Posisi tombol (x, y) - relatif terhadap frame
BUTTON_X = 50                   # Posisi X dari kiri
BUTTON_Y = 50                   # Posisi Y dari atas
BUTTON_WIDTH = 180
BUTTON_HEIGHT = 80

# ============================================
# Gesture Configuration  
# ============================================
PRESS_COOLDOWN = 1.0             # Detik - cooldown setelah tekan tombol
FINGER_LANDMARK = 8             # Index finger tip (landmark #8)

# ============================================
# Display Configuration
# ============================================
WINDOW_NAME = "Hand Landmark Virtual Button"
SHOW_FPS = True
SHOW_LANDMARKS = True
