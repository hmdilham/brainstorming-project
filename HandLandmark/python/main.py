"""
Hand Landmark Virtual Button - Main Entry Point
================================================
Sistem IoT Computer Vision yang menggunakan MediaPipe Hand Landmark
untuk mendeteksi gesture tangan sebagai tombol virtual yang mengontrol
LED pada ESP32 secara real-time.

Penggunaan:
    python main.py                  # Mode default (serial)
    python main.py --mode wifi      # Mode WiFi WebSocket
    python main.py --mode dummy     # Mode dummy (tanpa hardware)
    python main.py --camera 1       # Gunakan kamera index 1
    python main.py --camera "http://192.168.1.100:4747/video"  # DroidCam WiFi

Kontrol Keyboard:
    q - Quit
    m - Switch mode (serial/wifi/dummy)
    r - Reconnect ke ESP32
    d - Toggle drawing landmarks
    s - Screenshot
"""

import cv2
import sys
import time
import argparse
import numpy as np

from config import *
from hand_detector import HandDetector
from virtual_button import VirtualButton, draw_status_bar
from esp32_comm import create_communicator


def parse_args():
    """Parse command line arguments."""
    parser = argparse.ArgumentParser(description="Hand Landmark Virtual Button Controller")
    
    parser.add_argument("--mode", type=str, default=COMM_MODE,
                        choices=["serial", "wifi", "dummy"],
                        help="Communication mode (default: serial)")
    parser.add_argument("--camera", type=str, default=str(CAMERA_INDEX),
                        help="Camera index atau URL (default: 0)")
    parser.add_argument("--port", type=str, default=SERIAL_PORT,
                        help="Serial port (default: /dev/ttyUSB0)")
    parser.add_argument("--baud", type=int, default=SERIAL_BAUD,
                        help="Serial baud rate (default: 115200)")
    parser.add_argument("--esp-ip", type=str, default=ESP32_IP,
                        help="ESP32 IP address for WiFi mode")
    parser.add_argument("--ws-port", type=int, default=ESP32_WS_PORT,
                        help="WebSocket port (default: 81)")
    
    return parser.parse_args()


def setup_camera(camera_source):
    """
    Setup kamera (DroidCam atau webcam biasa).
    
    Args:
        camera_source: int index atau string URL
        
    Returns:
        cv2.VideoCapture object
    """
    # Coba parse sebagai integer (index kamera)
    try:
        cam_index = int(camera_source)
        cap = cv2.VideoCapture(cam_index)
    except ValueError:
        # String URL (DroidCam WiFi)
        cap = cv2.VideoCapture(camera_source)
    
    if not cap.isOpened():
        print(f"[ERROR] Tidak bisa membuka kamera: {camera_source}")
        print("Tips:")
        print("  - Pastikan DroidCam sudah aktif di HP dan laptop")
        print("  - Coba camera index 0, 1, atau 2")
        print("  - Untuk DroidCam WiFi, gunakan URL: http://IP:4747/video")
        sys.exit(1)
    
    # Set resolusi
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, CAMERA_WIDTH)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, CAMERA_HEIGHT)
    
    actual_w = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    actual_h = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    print(f"[Camera] Opened: {camera_source} ({actual_w}x{actual_h})")
    
    return cap


def draw_title_overlay(frame):
    """Gambar judul di atas frame."""
    h, w = frame.shape[:2]
    
    # Background title
    overlay = frame.copy()
    cv2.rectangle(overlay, (0, 0), (w, 35), (40, 40, 40), -1)
    cv2.addWeighted(overlay, 0.7, frame, 0.3, 0, frame)
    
    # Title text
    cv2.putText(frame, "Hand Landmark Virtual Button", (10, 25),
                cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 2)


def draw_instructions(frame):
    """Gambar instruksi keyboard di frame."""
    h, w = frame.shape[:2]
    instructions = [
        "Q: Quit  |  M: Switch Mode  |  R: Reconnect  |  D: Toggle Landmarks"
    ]
    
    font = cv2.FONT_HERSHEY_SIMPLEX
    y_start = 60
    
    for i, text in enumerate(instructions):
        cv2.putText(frame, text, (10, y_start + i * 20), font, 0.4,
                    (180, 180, 180), 1)


def main():
    """Main loop aplikasi."""
    args = parse_args()
    
    print("=" * 50)
    print("  Hand Landmark Virtual Button Controller")
    print("=" * 50)
    
    # ---- Setup Camera ----
    cap = setup_camera(args.camera)
    
    # ---- Setup Hand Detector ----
    detector = HandDetector(max_hands=1, detection_confidence=0.7)
    print("[MediaPipe] Hand detector initialized")
    
    # ---- Setup Virtual Button ----
    button = VirtualButton(
        x=BUTTON_X, y=BUTTON_Y,
        width=BUTTON_WIDTH, height=BUTTON_HEIGHT,
        cooldown=PRESS_COOLDOWN
    )
    
    # ---- Setup ESP32 Communication ----
    comm_mode = args.mode
    comm = create_communicator(
        mode=comm_mode,
        port=args.port,
        baud=args.baud,
        ip=args.esp_ip
    )
    
    print(f"[Comm] Mode: {comm_mode}")
    if not comm.connect():
        print("[WARN] Tidak bisa konek ke ESP32. Lanjut tanpa hardware...")
        if comm_mode != "dummy":
            print("[WARN] Switching ke dummy mode")
            comm = create_communicator("dummy")
            comm.connect()
            comm_mode = "dummy"
    
    # ---- Main Variables ----
    show_landmarks = SHOW_LANDMARKS
    fps = 0
    prev_time = time.time()
    frame_count = 0
    
    print("\n[Ready] Arahkan jari telunjuk ke tombol virtual untuk toggle LED!")
    print("[Keys] Q=Quit, M=SwitchMode, R=Reconnect, D=ToggleLandmarks\n")
    
    # ---- Main Loop ----
    try:
        while True:
            ret, frame = cap.read()
            if not ret:
                print("[ERROR] Frame tidak terbaca dari kamera")
                break
            
            # Flip horizontal (mirror mode) agar lebih intuitif
            frame = cv2.flip(frame, 1)
            
            # ---- Detect Hands ----
            hands = detector.detect(frame)
            hand_detected = len(hands) > 0
            
            # ---- Proses Gesture ----
            finger_pos = None
            if hand_detected:
                # Ambil posisi ujung jari telunjuk
                finger_pos = detector.get_finger_tip(hands[0], "index")
                
                # Gambar lingkaran kecil di ujung jari
                if finger_pos:
                    cv2.circle(frame, finger_pos, 10, (0, 255, 255), 2)
                    cv2.circle(frame, finger_pos, 3, (0, 255, 255), -1)
            
            # ---- Check Button Press ----
            pressed = button.check_press(finger_pos)
            
            if pressed:
                # Fire-and-forget: kirim command, state di-track lokal
                response = comm.toggle()
                button.led_state = response.get("led", False)
                print(f"[Toggle] LED {'ON' if button.led_state else 'OFF'}")
            
            # ---- Draw Overlay ----
            # Draw landmarks
            if show_landmarks and hand_detected:
                detector.draw_landmarks(frame, hands)
            
            # Draw title
            draw_title_overlay(frame)
            
            # Draw virtual button
            button.draw(frame)
            
            # Draw status bar
            draw_status_bar(
                frame, 
                led_state=button.led_state,
                hand_detected=hand_detected,
                comm_mode=comm_mode,
                connected=comm.connected,
                fps=fps
            )
            
            # ---- Calculate FPS ----
            frame_count += 1
            current_time = time.time()
            if current_time - prev_time >= 1.0:
                fps = frame_count / (current_time - prev_time)
                frame_count = 0
                prev_time = current_time
            
            # ---- Display ----
            cv2.imshow(WINDOW_NAME, frame)
            
            # ---- Keyboard Input ----
            key = cv2.waitKey(1) & 0xFF
            
            if key == ord('q'):
                print("[Quit] Shutting down...")
                break
                
            elif key == ord('d'):
                show_landmarks = not show_landmarks
                print(f"[Landmarks] {'ON' if show_landmarks else 'OFF'}")
                
            elif key == ord('r'):
                print("[Reconnect] Mencoba reconnect ke ESP32...")
                comm.disconnect()
                if not comm.connect():
                    print("[Reconnect] Gagal!")
                else:
                    print("[Reconnect] Berhasil!")
                    
            elif key == ord('m'):
                # Cycle mode: serial → wifi → dummy → serial
                modes = ["serial", "wifi", "dummy"]
                current_idx = modes.index(comm_mode) if comm_mode in modes else 0
                next_idx = (current_idx + 1) % len(modes)
                new_mode = modes[next_idx]
                
                print(f"[Mode] Switching {comm_mode} → {new_mode}")
                comm.disconnect()
                comm_mode = new_mode
                comm = create_communicator(
                    mode=comm_mode,
                    port=args.port,
                    baud=args.baud,
                    ip=args.esp_ip
                )
                if not comm.connect():
                    print(f"[Mode] {new_mode} connection failed")
                    
            elif key == ord('s'):
                # Screenshot
                filename = f"screenshot_{int(time.time())}.png"
                cv2.imwrite(filename, frame)
                print(f"[Screenshot] Saved: {filename}")
    
    except KeyboardInterrupt:
        print("\n[Interrupted] Shutting down...")
    
    finally:
        # ---- Cleanup ----
        cap.release()
        detector.release()
        comm.disconnect()
        cv2.destroyAllWindows()
        print("[Done] Application closed.")


if __name__ == "__main__":
    main()
