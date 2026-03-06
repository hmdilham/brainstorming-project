"""
Hand Detector - MediaPipe Hand Landmark Wrapper
Mendeteksi tangan dan mengekstrak posisi landmark jari.

Kompatibel dengan MediaPipe Tasks API (versi baru).
Model akan otomatis didownload jika belum ada.
"""

import cv2
import os
import urllib.request
import numpy as np

# Coba import MediaPipe - deteksi versi API
try:
    import mediapipe as mp
    from mediapipe.tasks import python as mp_python
    from mediapipe.tasks.python import vision as mp_vision
    USE_TASKS_API = True
    print("[MediaPipe] Using Tasks API (new)")
except ImportError:
    try:
        import mediapipe as mp
        _test = mp.solutions.hands
        USE_TASKS_API = False
        print("[MediaPipe] Using Solutions API (legacy)")
    except AttributeError:
        raise ImportError(
            "MediaPipe tidak terinstall dengan benar. "
            "Coba: pip install mediapipe --upgrade"
        )

# Model download URL
MODEL_URL = "https://storage.googleapis.com/mediapipe-models/hand_landmarker/hand_landmarker/float16/latest/hand_landmarker.task"
MODEL_FILENAME = "hand_landmarker.task"

# Koneksi antar landmark untuk menggambar skeleton tangan
HAND_CONNECTIONS = [
    (0, 1), (1, 2), (2, 3), (3, 4),       # Thumb
    (0, 5), (5, 6), (6, 7), (7, 8),       # Index
    (0, 9), (9, 10), (10, 11), (11, 12),  # Middle  
    (0, 13), (13, 14), (14, 15), (15, 16),# Ring
    (0, 17), (17, 18), (18, 19), (19, 20),# Pinky
    (5, 9), (9, 13), (13, 17)             # Palm
]


def download_model(model_dir=None):
    """Download hand_landmarker.task model jika belum ada."""
    if model_dir is None:
        model_dir = os.path.dirname(os.path.abspath(__file__))
    
    model_path = os.path.join(model_dir, MODEL_FILENAME)
    
    if os.path.exists(model_path):
        return model_path
    
    print(f"[MediaPipe] Downloading model: {MODEL_FILENAME}...")
    try:
        urllib.request.urlretrieve(MODEL_URL, model_path)
        print(f"[MediaPipe] Model saved to: {model_path}")
        return model_path
    except Exception as e:
        print(f"[MediaPipe] Download failed: {e}")
        print(f"[MediaPipe] Download manually from: {MODEL_URL}")
        print(f"[MediaPipe] Save as: {model_path}")
        return None


class HandLandmarks:
    """
    Wrapper untuk normalized landmarks agar format konsisten
    antara old/new API.
    """
    def __init__(self, landmarks, frame_shape):
        """
        Args:
            landmarks: list of landmark objects dengan .x, .y, .z
            frame_shape: (h, w, c) dari frame
        """
        self.landmarks = landmarks
        self.frame_shape = frame_shape
    
    def get_pixel(self, idx):
        """Dapatkan posisi pixel (x, y) dari landmark index."""
        h, w, _ = self.frame_shape
        lm = self.landmarks[idx]
        return (int(lm.x * w), int(lm.y * h))
    
    def get_normalized(self, idx):
        """Dapatkan posisi normalized (0-1) dari landmark index."""
        lm = self.landmarks[idx]
        return (lm.x, lm.y, lm.z)


class HandDetector:
    """Wrapper untuk MediaPipe Hands dengan utilitas deteksi jari."""
    
    # Landmark indices
    WRIST = 0
    THUMB_TIP = 4
    INDEX_FINGER_TIP = 8
    MIDDLE_FINGER_TIP = 12
    RING_FINGER_TIP = 16
    PINKY_TIP = 20
    
    # Finger MCP (base) landmarks untuk cek jari terangkat
    INDEX_FINGER_MCP = 5
    MIDDLE_FINGER_MCP = 9
    RING_FINGER_MCP = 13
    PINKY_MCP = 17
    
    def __init__(self, max_hands=1, detection_confidence=0.7, tracking_confidence=0.5):
        self.frame_shape = None
        self._last_results = []
        
        if USE_TASKS_API:
            self._init_tasks_api(max_hands, detection_confidence)
        else:
            self._init_solutions_api(max_hands, detection_confidence, tracking_confidence)
    
    def _init_tasks_api(self, max_hands, detection_confidence):
        """Initialize menggunakan MediaPipe Tasks API (baru)."""
        model_path = download_model()
        if model_path is None:
            raise FileNotFoundError(
                f"Model {MODEL_FILENAME} tidak ditemukan. "
                f"Download dari: {MODEL_URL}"
            )
        
        base_options = mp_python.BaseOptions(
            model_asset_path=model_path
        )
        options = mp_vision.HandLandmarkerOptions(
            base_options=base_options,
            num_hands=max_hands,
            min_hand_detection_confidence=detection_confidence,
            min_hand_presence_confidence=detection_confidence,
            min_tracking_confidence=0.5,
            running_mode=mp_vision.RunningMode.VIDEO
        )
        self._detector = mp_vision.HandLandmarker.create_from_options(options)
        self._use_tasks = True
        self._timestamp_ms = 0
    
    def _init_solutions_api(self, max_hands, detection_confidence, tracking_confidence):
        """Initialize menggunakan MediaPipe Solutions API (legacy)."""
        self.mp_hands = mp.solutions.hands
        self.mp_drawing = mp.solutions.drawing_utils
        self.mp_drawing_styles = mp.solutions.drawing_styles
        
        self._detector = self.mp_hands.Hands(
            static_image_mode=False,
            max_num_hands=max_hands,
            min_detection_confidence=detection_confidence,
            min_tracking_confidence=tracking_confidence
        )
        self._use_tasks = False
    
    def detect(self, frame):
        """
        Deteksi tangan pada frame.
        
        Args:
            frame: BGR frame dari OpenCV
            
        Returns:
            list of HandLandmarks objects
        """
        self.frame_shape = frame.shape
        rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        
        if self._use_tasks:
            return self._detect_tasks(rgb_frame)
        else:
            return self._detect_solutions(rgb_frame)
    
    def _detect_tasks(self, rgb_frame):
        """Deteksi menggunakan Tasks API."""
        mp_image = mp.Image(image_format=mp.ImageFormat.SRGB, data=rgb_frame)
        
        self._timestamp_ms += 33  # ~30fps
        result = self._detector.detect_for_video(mp_image, self._timestamp_ms)
        
        self._last_results = []
        if result.hand_landmarks:
            for hand_lms in result.hand_landmarks:
                wrapped = HandLandmarks(hand_lms, self.frame_shape)
                self._last_results.append(wrapped)
        
        return self._last_results
    
    def _detect_solutions(self, rgb_frame):
        """Deteksi menggunakan Solutions API (legacy)."""
        results = self._detector.process(rgb_frame)
        
        self._last_results = []
        if results.multi_hand_landmarks:
            for hand_lms in results.multi_hand_landmarks:
                wrapped = HandLandmarks(hand_lms.landmark, self.frame_shape)
                self._last_results.append(wrapped)
        
        return self._last_results
    
    def get_finger_tip(self, hand_landmarks, finger="index"):
        """
        Dapatkan posisi ujung jari dalam pixel.
        
        Args:
            hand_landmarks: HandLandmarks object
            finger: "thumb", "index", "middle", "ring", "pinky"
            
        Returns:
            tuple (x, y) posisi ujung jari dalam pixel
        """
        finger_map = {
            "thumb": self.THUMB_TIP,
            "index": self.INDEX_FINGER_TIP,
            "middle": self.MIDDLE_FINGER_TIP,
            "ring": self.RING_FINGER_TIP,
            "pinky": self.PINKY_TIP
        }
        landmark_id = finger_map.get(finger, self.INDEX_FINGER_TIP)
        return hand_landmarks.get_pixel(landmark_id)
    
    def is_finger_up(self, hand_landmarks, finger="index"):
        """
        Cek apakah jari terangkat (tip lebih tinggi dari MCP).
        
        Returns:
            bool - True jika jari terangkat
        """
        tip_map = {
            "index": (self.INDEX_FINGER_TIP, self.INDEX_FINGER_MCP),
            "middle": (self.MIDDLE_FINGER_TIP, self.MIDDLE_FINGER_MCP),
            "ring": (self.RING_FINGER_TIP, self.RING_FINGER_MCP),
            "pinky": (self.PINKY_TIP, self.PINKY_MCP)
        }
        
        if finger not in tip_map:
            return False
        
        tip_id, mcp_id = tip_map[finger]
        tip_pos = hand_landmarks.get_pixel(tip_id)
        mcp_pos = hand_landmarks.get_pixel(mcp_id)
        
        # Dalam koordinat pixel, y lebih kecil = lebih tinggi
        return tip_pos[1] < mcp_pos[1]
    
    def get_finger_states(self, hand_landmarks):
        """Dapatkan status semua jari (terangkat atau tidak)."""
        return {
            "index": self.is_finger_up(hand_landmarks, "index"),
            "middle": self.is_finger_up(hand_landmarks, "middle"),
            "ring": self.is_finger_up(hand_landmarks, "ring"),
            "pinky": self.is_finger_up(hand_landmarks, "pinky")
        }
    
    def draw_landmarks(self, frame, hand_landmarks_list):
        """
        Gambar landmarks dan koneksi pada frame.
        Bekerja dengan kedua API (Tasks dan Solutions).
        
        Args:
            frame: BGR frame
            hand_landmarks_list: List of HandLandmarks objects
        """
        for hand_lm in hand_landmarks_list:
            # Gambar koneksi (garis antar landmark)
            for start_idx, end_idx in HAND_CONNECTIONS:
                start_pos = hand_lm.get_pixel(start_idx)
                end_pos = hand_lm.get_pixel(end_idx)
                cv2.line(frame, start_pos, end_pos, (0, 255, 0), 2)
            
            # Gambar landmark points
            for i in range(21):
                pos = hand_lm.get_pixel(i)
                
                # Warna berbeda untuk tip jari
                if i in [4, 8, 12, 16, 20]:
                    color = (0, 0, 255)  # Merah untuk tips
                    radius = 6
                elif i == 0:
                    color = (255, 0, 0)  # Biru untuk wrist
                    radius = 6
                else:
                    color = (0, 255, 0)  # Hijau untuk lainnya
                    radius = 4
                
                cv2.circle(frame, pos, radius, color, -1)
                cv2.circle(frame, pos, radius, (255, 255, 255), 1)
    
    def release(self):
        """Release MediaPipe resources."""
        if self._use_tasks:
            self._detector.close()
        else:
            self._detector.close()
