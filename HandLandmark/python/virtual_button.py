"""
Virtual Button - Tombol virtual yang dirender pada frame kamera.
Mendeteksi "press" ketika ujung jari berada di area tombol.
"""

import cv2
import time
import numpy as np


class VirtualButton:
    """Tombol virtual yang bisa ditekan dengan ujung jari."""
    
    # Warna (BGR)
    COLOR_OFF = (60, 60, 220)       # Merah gelap
    COLOR_ON = (60, 220, 60)        # Hijau
    COLOR_HOVER = (100, 180, 255)   # Orange/kuning
    COLOR_PRESSED = (255, 255, 255) # Putih (flash saat press)
    COLOR_TEXT = (255, 255, 255)    # Putih
    COLOR_BORDER = (200, 200, 200)  # Abu-abu
    
    def __init__(self, x=50, y=50, width=180, height=80, 
                 label_on="LED ON", label_off="LED OFF", cooldown=1.0):
        """
        Args:
            x, y: Posisi tombol (top-left corner)
            width, height: Ukuran tombol
            label_on: Label saat LED ON
            label_off: Label saat LED OFF
            cooldown: Waktu cooldown antar press (detik)
        """
        self.x = x
        self.y = y
        self.width = width
        self.height = height
        self.label_on = label_on
        self.label_off = label_off
        self.cooldown = cooldown
        
        # State
        self.led_state = False
        self.is_hovering = False
        self.is_pressing = False
        self.last_press_time = 0
        self.press_animation = 0  # Counter untuk animasi press
        
    @property
    def center(self):
        """Center point tombol."""
        return (self.x + self.width // 2, self.y + self.height // 2)
    
    @property
    def rect(self):
        """(x1, y1, x2, y2) bounding rectangle."""
        return (self.x, self.y, self.x + self.width, self.y + self.height)
    
    def is_point_inside(self, point):
        """
        Cek apakah titik (x, y) berada di dalam area tombol.
        
        Args:
            point: tuple (x, y)
            
        Returns:
            bool
        """
        if point is None:
            return False
        px, py = point
        return (self.x <= px <= self.x + self.width and 
                self.y <= py <= self.y + self.height)
    
    def check_press(self, finger_pos):
        """
        Cek apakah jari menekan tombol (dengan cooldown).
        
        Args:
            finger_pos: tuple (x, y) posisi ujung jari
            
        Returns:
            bool - True jika tombol baru saja ditekan (toggle event)
        """
        self.is_hovering = self.is_point_inside(finger_pos)
        
        if not self.is_hovering:
            self.is_pressing = False
            return False
        
        current_time = time.time()
        
        # Cek cooldown
        if current_time - self.last_press_time < self.cooldown:
            return False
        
        # Tombol ditekan! (led_state TIDAK diubah di sini,
        # hanya diubah dari response aktual ESP32 di main.py)
        self.is_pressing = True
        self.last_press_time = current_time
        self.press_animation = 10  # Frame counter untuk animasi
        
        return True
    
    def draw(self, frame):
        """
        Render tombol pada frame.
        
        Args:
            frame: BGR frame dari OpenCV
        """
        x1, y1, x2, y2 = self.rect
        
        # Tentukan warna berdasarkan state
        if self.press_animation > 0:
            # Animasi flash saat ditekan
            bg_color = self.COLOR_PRESSED
            self.press_animation -= 1
        elif self.is_hovering:
            bg_color = self.COLOR_HOVER
        elif self.led_state:
            bg_color = self.COLOR_ON
        else:
            bg_color = self.COLOR_OFF
        
        # Gambar background dengan transparansi
        overlay = frame.copy()
        cv2.rectangle(overlay, (x1, y1), (x2, y2), bg_color, -1)
        cv2.addWeighted(overlay, 0.7, frame, 0.3, 0, frame)
        
        # Gambar border
        border_thickness = 3 if self.is_hovering else 2
        cv2.rectangle(frame, (x1, y1), (x2, y2), self.COLOR_BORDER, border_thickness)
        
        # Gambar rounded corners effect (4 lingkaran kecil di sudut)
        corner_radius = 8
        corners = [(x1 + corner_radius, y1 + corner_radius),
                    (x2 - corner_radius, y1 + corner_radius),
                    (x1 + corner_radius, y2 - corner_radius),
                    (x2 - corner_radius, y2 - corner_radius)]
        for corner in corners:
            cv2.circle(frame, corner, corner_radius, self.COLOR_BORDER, border_thickness)
        
        # Label text
        label = self.label_on if self.led_state else self.label_off
        font = cv2.FONT_HERSHEY_SIMPLEX
        font_scale = 0.8
        thickness = 2
        
        text_size = cv2.getTextSize(label, font, font_scale, thickness)[0]
        text_x = x1 + (self.width - text_size[0]) // 2
        text_y = y1 + (self.height + text_size[1]) // 2
        
        # Text shadow
        cv2.putText(frame, label, (text_x + 1, text_y + 1), font, 
                    font_scale, (0, 0, 0), thickness + 1)
        # Text
        cv2.putText(frame, label, (text_x, text_y), font, 
                    font_scale, self.COLOR_TEXT, thickness)
        
        # Status indicator (lingkaran kecil)
        indicator_color = (0, 255, 0) if self.led_state else (0, 0, 255)
        indicator_pos = (x2 - 15, y1 + 15)
        cv2.circle(frame, indicator_pos, 8, indicator_color, -1)
        cv2.circle(frame, indicator_pos, 8, self.COLOR_BORDER, 1)
        
        # Cooldown progress bar (jika masih dalam cooldown)
        current_time = time.time()
        elapsed = current_time - self.last_press_time
        if elapsed < self.cooldown and self.last_press_time > 0:
            progress = elapsed / self.cooldown
            bar_width = int(self.width * progress)
            cv2.rectangle(frame, (x1, y2 + 2), (x1 + bar_width, y2 + 6), 
                         (0, 255, 255), -1)


def draw_status_bar(frame, led_state, hand_detected, comm_mode, connected, fps=0):
    """
    Gambar status bar di bagian bawah frame.
    
    Args:
        frame: BGR frame
        led_state: bool - status LED
        hand_detected: bool - apakah tangan terdeteksi
        comm_mode: str - "serial" atau "wifi"
        connected: bool - status koneksi ke ESP32
        fps: float - frames per second
    """
    h, w = frame.shape[:2]
    bar_height = 40
    bar_y = h - bar_height
    
    # Background bar
    overlay = frame.copy()
    cv2.rectangle(overlay, (0, bar_y), (w, h), (40, 40, 40), -1)
    cv2.addWeighted(overlay, 0.8, frame, 0.2, 0, frame)
    
    # Separator line
    cv2.line(frame, (0, bar_y), (w, bar_y), (100, 100, 100), 1)
    
    font = cv2.FONT_HERSHEY_SIMPLEX
    font_scale = 0.5
    y_text = bar_y + 26
    
    # LED Status
    led_color = (0, 255, 0) if led_state else (0, 0, 255)
    led_text = "LED: ON" if led_state else "LED: OFF"
    cv2.circle(frame, (15, y_text - 5), 6, led_color, -1)
    cv2.putText(frame, led_text, (28, y_text), font, font_scale, led_color, 1)
    
    # Hand Status
    hand_color = (0, 255, 0) if hand_detected else (100, 100, 100)
    hand_text = "Hand: OK" if hand_detected else "Hand: --"
    cv2.putText(frame, hand_text, (130, y_text), font, font_scale, hand_color, 1)
    
    # Connection Status
    conn_color = (0, 255, 0) if connected else (0, 0, 255)
    conn_text = f"ESP32 ({comm_mode}): {'OK' if connected else 'DISCONNECTED'}"
    cv2.putText(frame, conn_text, (250, y_text), font, font_scale, conn_color, 1)
    
    # FPS
    if fps > 0:
        fps_text = f"FPS: {fps:.0f}"
        cv2.putText(frame, fps_text, (w - 90, y_text), font, font_scale, 
                    (200, 200, 200), 1)
