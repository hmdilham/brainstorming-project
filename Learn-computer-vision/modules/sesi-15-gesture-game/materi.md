# Sesi 15 — Gesture-Controlled Game

## 🎯 Tujuan Pembelajaran
- Membuat game sederhana dengan Pygame
- Mengontrol karakter game dengan gesture tangan dari kamera
- 🔨 Mini Project: Gesture Dodge Game — hindari rintangan yang jatuh

---

## 📖 Teori

### Arsitektur Game + CV
```
Webcam  →  MediaPipe (Hand) → Position → Game Logic (Pygame)
                                              │
                                       ┌──────▼──────┐
                                       │  Game Loop  │
                                       │  - Update   │
                                       │  - Render   │
                                       │  - Collision│
                                       └─────────────┘
```

### Pygame Game Loop
```python
while running:
    1. Handle events (quit, keypress)
    2. Read camera → detect gesture → update player position
    3. Update game state (obstacles, score)
    4. Check collision
    5. Render everything
    6. Control FPS
```

---

## 🛠️ Praktik

### 0. Install Pygame

```bash
pip install pygame
```

### 1. 🔨 Mini Project: Gesture Dodge Game

```python
"""
sesi15_gesture_game.py
🔨 MINI PROJECT: Gesture Dodge Game
Gerakkan tangan ke kiri/kanan untuk menghindari rintangan yang jatuh

Controls:
- Tangan kiri layar  → karakter bergerak ke kiri
- Tangan kanan layar → karakter bergerak ke kanan
- Tangan tengah      → karakter diam
- Tidak ada tangan   → pause
"""
import pygame
import cv2
import mediapipe as mp
import numpy as np
import random
import time

# ====== Setup MediaPipe ======
mp_hands = mp.solutions.hands
hands = mp_hands.Hands(max_num_hands=1, min_detection_confidence=0.7)

# ====== Setup Kamera ======
cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

# ====== Setup Pygame ======
pygame.init()

GAME_WIDTH = 500
GAME_HEIGHT = 700
CAM_DISPLAY_W = 320
CAM_DISPLAY_H = 240
WINDOW_W = GAME_WIDTH + CAM_DISPLAY_W + 20

screen = pygame.display.set_mode((WINDOW_W, GAME_HEIGHT))
pygame.display.set_caption("Gesture Dodge Game")
clock = pygame.time.Clock()

# Fonts
font_big = pygame.font.SysFont("Arial", 48, bold=True)
font_med = pygame.font.SysFont("Arial", 24)
font_small = pygame.font.SysFont("Arial", 18)

# Colors
WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
RED = (255, 50, 50)
GREEN = (50, 255, 50)
BLUE = (50, 100, 255)
YELLOW = (255, 255, 50)
DARK_BG = (20, 20, 30)
PANEL_BG = (30, 30, 45)

# ====== Game Objects ======
PLAYER_W = 50
PLAYER_H = 50
OBSTACLE_W = 60
OBSTACLE_MIN_H = 30
OBSTACLE_MAX_H = 60

player_x = GAME_WIDTH // 2 - PLAYER_W // 2
player_y = GAME_HEIGHT - 100

obstacles = []
score = 0
high_score = 0
game_speed = 3
hand_x_ratio = 0.5  # 0.0 (kiri) → 1.0 (kanan)
game_state = "PLAYING"  # PLAYING, GAME_OVER, PAUSED
lives = 3
level = 1


def spawn_obstacle():
    x = random.randint(0, GAME_WIDTH - OBSTACLE_W)
    h = random.randint(OBSTACLE_MIN_H, OBSTACLE_MAX_H)
    color = random.choice([RED, YELLOW, (255, 128, 0)])
    return {"x": x, "y": -h, "w": OBSTACLE_W, "h": h, "color": color}


def reset_game():
    global obstacles, score, game_speed, lives, level, game_state
    obstacles = []
    score = 0
    game_speed = 3
    lives = 3
    level = 1
    game_state = "PLAYING"


# ====== Main Game Loop ======
running = True
spawn_timer = 0
SPAWN_INTERVAL = 30  # frames

while running:
    # 1. Events
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        elif event.type == pygame.KEYDOWN:
            if event.key == pygame.K_r:
                reset_game()
            elif event.key == pygame.K_q:
                running = False

    # 2. Read Camera & Detect Hand
    ret, frame = cap.read()
    if ret:
        frame = cv2.resize(frame, (CAM_DISPLAY_W, CAM_DISPLAY_H))
        frame = cv2.flip(frame, 1)
        rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        results = hands.process(rgb)

        hand_detected = False
        if results.multi_hand_landmarks:
            hand_lm = results.multi_hand_landmarks[0]
            hand_detected = True

            # Centroid x
            hand_x_ratio = sum([lm.x for lm in hand_lm.landmark]) / 21

            # Draw landmark pada frame kecil
            for lm in hand_lm.landmark:
                px = int(lm.x * CAM_DISPLAY_W)
                py = int(lm.y * CAM_DISPLAY_H)
                cv2.circle(frame, (px, py), 2, (0, 255, 0), -1)

            # Indicator
            indicator_x = int(hand_x_ratio * CAM_DISPLAY_W)
            cv2.line(frame, (indicator_x, 0), (indicator_x, CAM_DISPLAY_H),
                     (0, 0, 255), 2)

        # Convert frame untuk Pygame
        frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        frame_surface = pygame.surfarray.make_surface(
            np.transpose(frame_rgb, (1, 0, 2))
        )

    # 3. Update Game (jika PLAYING)
    if game_state == "PLAYING":
        # Update player position
        target_x = int(hand_x_ratio * (GAME_WIDTH - PLAYER_W))
        player_x += (target_x - player_x) * 0.2  # Smooth movement

        # Spawn obstacles
        spawn_timer += 1
        if spawn_timer >= SPAWN_INTERVAL:
            obstacles.append(spawn_obstacle())
            spawn_timer = 0

        # Update obstacles
        for obs in obstacles:
            obs["y"] += game_speed

        # Remove off-screen obstacles & add score
        new_obs = []
        for obs in obstacles:
            if obs["y"] > GAME_HEIGHT:
                score += 10
            else:
                new_obs.append(obs)
        obstacles = new_obs

        # Level up
        new_level = score // 200 + 1
        if new_level > level:
            level = new_level
            game_speed = 3 + level * 0.5
            SPAWN_INTERVAL = max(15, 30 - level * 2)

        # Collision detection
        player_rect = pygame.Rect(int(player_x), player_y, PLAYER_W, PLAYER_H)
        for obs in obstacles:
            obs_rect = pygame.Rect(obs["x"], obs["y"], obs["w"], obs["h"])
            if player_rect.colliderect(obs_rect):
                lives -= 1
                obstacles.remove(obs)
                if lives <= 0:
                    game_state = "GAME_OVER"
                    high_score = max(high_score, score)
                break

    # 4. Render
    screen.fill(DARK_BG)

    # Game area
    pygame.draw.rect(screen, PANEL_BG, (0, 0, GAME_WIDTH, GAME_HEIGHT))

    if game_state == "PLAYING":
        # Player
        pygame.draw.rect(screen, GREEN,
                          (int(player_x), player_y, PLAYER_W, PLAYER_H))
        pygame.draw.rect(screen, WHITE,
                          (int(player_x), player_y, PLAYER_W, PLAYER_H), 2)

        # Obstacles
        for obs in obstacles:
            pygame.draw.rect(screen, obs["color"],
                              (obs["x"], int(obs["y"]), obs["w"], obs["h"]))

        # HUD
        score_text = font_med.render(f"Score: {score}", True, WHITE)
        screen.blit(score_text, (10, 10))

        level_text = font_small.render(f"Level: {level} | Speed: {game_speed:.1f}",
                                        True, YELLOW)
        screen.blit(level_text, (10, 40))

        # Lives
        for i in range(lives):
            pygame.draw.circle(screen, RED, (GAME_WIDTH - 30 - i * 30, 25), 10)

    elif game_state == "GAME_OVER":
        go_text = font_big.render("GAME OVER", True, RED)
        screen.blit(go_text, (GAME_WIDTH // 2 - 130, GAME_HEIGHT // 2 - 60))

        score_text = font_med.render(f"Score: {score}  High: {high_score}",
                                      True, WHITE)
        screen.blit(score_text, (GAME_WIDTH // 2 - 100, GAME_HEIGHT // 2))

        restart_text = font_small.render("Press 'R' to restart", True, YELLOW)
        screen.blit(restart_text, (GAME_WIDTH // 2 - 70, GAME_HEIGHT // 2 + 40))

    # Camera panel (di sebelah kanan)
    cam_x = GAME_WIDTH + 10
    pygame.draw.rect(screen, PANEL_BG, (cam_x, 0, CAM_DISPLAY_W + 10, GAME_HEIGHT))

    # Camera title
    cam_title = font_small.render("Camera Feed", True, WHITE)
    screen.blit(cam_title, (cam_x + 5, 5))

    # Camera frame
    if ret:
        screen.blit(frame_surface, (cam_x + 5, 30))

    # Hand position indicator bar
    bar_y = 30 + CAM_DISPLAY_H + 10
    pygame.draw.rect(screen, (60, 60, 60),
                      (cam_x + 5, bar_y, CAM_DISPLAY_W, 20))
    indicator_pos = int(hand_x_ratio * CAM_DISPLAY_W)
    pygame.draw.rect(screen, GREEN,
                      (cam_x + 5 + indicator_pos - 5, bar_y, 10, 20))

    # Instructions
    inst_y = bar_y + 40
    instructions = [
        "CONTROLS:",
        "Move hand LEFT/RIGHT",
        "to dodge obstacles",
        "",
        "R = Restart",
        "Q = Quit",
        "",
        f"FPS: {clock.get_fps():.0f}",
    ]
    for i, text in enumerate(instructions):
        txt = font_small.render(text, True, (180, 180, 180))
        screen.blit(txt, (cam_x + 10, inst_y + i * 22))

    pygame.display.flip()
    clock.tick(60)

# Cleanup
hands.close()
cap.release()
pygame.quit()
```

### 2. Variasi: Gesture Jump Game

```python
"""
sesi15_jump_game.py
Variasi: Angkat tangan ke atas untuk lompat
Tangan di bawah = jalan biasa
Tangan di atas  = lompat menghindari rintangan dari samping
"""
import pygame
import cv2
import mediapipe as mp
import numpy as np
import random

mp_hands = mp.solutions.hands
hands = mp_hands.Hands(max_num_hands=1, min_detection_confidence=0.7)
cap = cv2.VideoCapture(0)

pygame.init()
W, H = 800, 400
screen = pygame.display.set_mode((W, H))
pygame.display.set_caption("Gesture Jump!")
clock = pygame.time.Clock()
font = pygame.font.SysFont("Arial", 24)

# Player
player_x = 100
player_y = H - 80
player_w, player_h = 40, 40
jumping = False
jump_height = 0
target_jump = 0

# Obstacles
obstacles = []
spawn_timer = 0
score = 0
speed = 5

running = True
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    # Camera
    ret, frame = cap.read()
    hand_y_ratio = 0.7  # Default: bawah

    if ret:
        frame = cv2.resize(frame, (200, 150))
        frame = cv2.flip(frame, 1)
        rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        results = hands.process(rgb)

        if results.multi_hand_landmarks:
            hand_lm = results.multi_hand_landmarks[0]
            hand_y_ratio = sum([lm.y for lm in hand_lm.landmark]) / 21

    # Jump based on hand height
    target_jump = max(0, (0.5 - hand_y_ratio) * 300)  # Tangan atas = jump tinggi

    # Smooth jump
    jump_height += (target_jump - jump_height) * 0.15
    current_player_y = H - 80 - int(jump_height)

    # Spawn obstacles
    spawn_timer += 1
    if spawn_timer > 60:
        obs_h = random.randint(20, 50)
        obstacles.append({"x": W, "y": H - 60 - obs_h, "w": 30, "h": obs_h + 20})
        spawn_timer = 0

    # Update obstacles
    for obs in obstacles:
        obs["x"] -= speed

    obstacles = [o for o in obstacles if o["x"] > -50]

    # Collision
    player_rect = pygame.Rect(player_x, current_player_y, player_w, player_h)
    hit = False
    for obs in obstacles:
        if player_rect.colliderect(pygame.Rect(obs["x"], obs["y"], obs["w"], obs["h"])):
            hit = True

    if not hit:
        score += 1

    # Render
    screen.fill((135, 206, 235))  # Sky blue

    # Ground
    pygame.draw.rect(screen, (100, 200, 100), (0, H - 40, W, 40))

    # Player
    color = (255, 50, 50) if hit else (50, 255, 50)
    pygame.draw.rect(screen, color,
                      (player_x, current_player_y, player_w, player_h))

    # Obstacles
    for obs in obstacles:
        pygame.draw.rect(screen, (200, 50, 50),
                          (obs["x"], obs["y"], obs["w"], obs["h"]))

    # Camera preview
    if ret:
        frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        surf = pygame.surfarray.make_surface(np.transpose(frame_rgb, (1, 0, 2)))
        screen.blit(surf, (W - 210, 10))

    # Score
    txt = font.render(f"Score: {score // 10}", True, (0, 0, 0))
    screen.blit(txt, (10, 10))

    txt2 = font.render("Hand UP = Jump!", True, (0, 0, 0))
    screen.blit(txt2, (10, 40))

    pygame.display.flip()
    clock.tick(60)

    speed = 5 + score // 500  # Accelerate

hands.close()
cap.release()
pygame.quit()
```

---

## 📝 Latihan Mandiri

1. **Tambahkan power-up** (item hijau yang menambah nyawa)
2. **Buat leaderboard** sederhana yang menyimpan high score ke file
3. **Tambahkan sound effects** menggunakan `pygame.mixer`
4. **Challenge**: Buat game Fruit Ninja — jari memotong buah yang jatuh

---

## 📚 Referensi
- [Pygame Documentation](https://www.pygame.org/docs/)
- [Pygame Tutorial](https://realpython.com/pygame-a-primer/)
- [MediaPipe + Pygame](https://google.github.io/mediapipe/solutions/hands.html)
