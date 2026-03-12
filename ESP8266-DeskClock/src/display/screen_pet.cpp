#include "screen_pet.h"
#include "../sprites/sprites.h"

// ============================================================
//  Pet Screen Implementation - Adafruit SSD1306
// ============================================================

ScreenPet::ScreenPet(SettingsManager &settings)
    : _settings(settings), _currentFrame(0), _lastFrameChange(0),
      _lastMoodChange(0), _mood(PET_MOOD_NEUTRAL), _isActioning(false),
      _actionStart(0), _currentAnimation(PET_ANIM_IDLE), _lastAnimationChange(0),
      _posX(48), _posY(0), _floatUp(true), _lastFloat(0) {}

void ScreenPet::render(Adafruit_SSD1306 &display) {
  // Top status bar
  display.drawFastHLine(0, 12, 128, SSD1306_WHITE);
  display.setTextSize(1);

  // Pet type name
  const char *petNames[] = {"Kucing", "Bintang", "Robot"};
  uint8_t petType = _settings.get().petType;
  if (petType > PET_ROBOT)
    petType = PET_CAT;

  display.setCursor(2, 2);
  display.print(F(" "));
  display.print(petNames[petType]);

  // Draw the animated pet sprite (centered)
  _drawPetSprite(display);

  // Draw happiness bar
  _drawHappinessBar(display);

  // Draw mood indicator text
  _drawMoodIndicator(display);
}

void ScreenPet::update() {
  unsigned long now = millis();

  // Update animation state
  _updateAnimation();

  // Animate frames based on current animation
  int frameCount = PET_IDLE_FRAMES;
  switch (_currentAnimation) {
    case PET_ANIM_WALK_LEFT:
    case PET_ANIM_WALK_RIGHT:
      frameCount = 2; // walk has 2 frames
      break;
    case PET_ANIM_EATING:
    case PET_ANIM_SLEEPING:
    case PET_ANIM_STUDYING:
      frameCount = 2; // these have 2 frames
      break;
    default:
      frameCount = PET_IDLE_FRAMES;
      break;
  }

  if (now - _lastFrameChange >= PET_FRAME_RATE) {
    _currentFrame = (_currentFrame + 1) % frameCount;
    _lastFrameChange = now;
  }

  // Movement for walking animations
  if (_currentAnimation == PET_ANIM_WALK_LEFT) {
    _posX -= 2;
    if (_posX < -32) _posX = 128; // wrap around
  } else if (_currentAnimation == PET_ANIM_WALK_RIGHT) {
    _posX += 2;
    if (_posX > 128) _posX = -32; // wrap around
  }

  // Floating effect (only for idle and studying)
  if (_currentAnimation == PET_ANIM_IDLE || _currentAnimation == PET_ANIM_STUDYING) {
    if (now - _lastFloat >= 300) {
      if (_floatUp) {
        _posY--;
        if (_posY <= -2)
          _floatUp = false;
      } else {
        _posY++;
        if (_posY >= 2)
          _floatUp = true;
      }
      _lastFloat = now;
    }
  } else {
    _posY = 0; // reset Y for other animations
  }

  // Update mood periodically
  if (now - _lastMoodChange >= PET_MOOD_INTERVAL) {
    _updateMood();
    _lastMoodChange = now;
  }

  // End special action after 2 seconds
  if (_isActioning && (now - _actionStart > 2000)) {
    _isActioning = false;
  }
}

void ScreenPet::feed() {
  GadgetSettings &cfg = _settings.get();
  cfg.petHunger = min((uint8_t)(cfg.petHunger + 20), (uint8_t)100);
  cfg.petHappiness = min((uint8_t)(cfg.petHappiness + 10), (uint8_t)100);
  _isActioning = true;
  _actionStart = millis();
  _currentAnimation = PET_ANIM_EATING;
  _currentFrame = 0;
  _settings.save();
}

void ScreenPet::pet() {
  GadgetSettings &cfg = _settings.get();
  cfg.petHappiness = min((uint8_t)(cfg.petHappiness + 15), (uint8_t)100);
  _isActioning = true;
  _actionStart = millis();
  _settings.save();
}

PetMood ScreenPet::getMood() { return _mood; }

// --- Private Methods ---

void ScreenPet::_drawPetSprite(Adafruit_SSD1306 &display) {
  uint8_t petType = _settings.get().petType;
  
  // Position with movement and floating effect
  int x = _posX;
  int y = 22 + _posY;
  
  const uint8_t *sprite = nullptr;
  
  // Get current frame based on animation state
  switch(petType) {
    case PET_CAT:
      switch (_currentAnimation) {
        case PET_ANIM_WALK_LEFT:
          sprite = (const uint8_t*)pgm_read_ptr(&cat_walk_left_frames[_currentFrame]);
          break;
        case PET_ANIM_WALK_RIGHT:
          sprite = (const uint8_t*)pgm_read_ptr(&cat_walk_right_frames[_currentFrame]);
          break;
        case PET_ANIM_EATING:
          sprite = (const uint8_t*)pgm_read_ptr(&cat_eating_frames[_currentFrame]);
          break;
        case PET_ANIM_SLEEPING:
          sprite = (const uint8_t*)pgm_read_ptr(&cat_sleeping_frames[_currentFrame]);
          break;
        case PET_ANIM_STUDYING:
          sprite = (const uint8_t*)pgm_read_ptr(&cat_studying_frames[_currentFrame]);
          break;
        default: // IDLE
          sprite = (const uint8_t*)pgm_read_ptr(&cat_idle_frames[_currentFrame]);
          break;
      }
      break;
      
    case PET_STAR:
      // For now, star only has idle animation
      sprite = (const uint8_t*)pgm_read_ptr(&star_idle_frames[_currentFrame % STAR_IDLE_COUNT]);
      break;
      
    case PET_ROBOT:
      // For now, robot only has idle animation
      sprite = (const uint8_t*)pgm_read_ptr(&robot_idle_frames[_currentFrame % ROBOT_IDLE_COUNT]);
      break;
      
    default:
      sprite = (const uint8_t*)pgm_read_ptr(&cat_idle_frames[0]);
      break;
  }
  
  if (sprite != nullptr) {
    // Draw the 32x32 sprite
    display.drawBitmap(x, y, sprite, 32, 32, SSD1306_WHITE);
  }
}

void ScreenPet::_drawMoodIndicator(Adafruit_SSD1306 &display) {
  display.setTextSize(1);
  display.setCursor(2, 56);
  
  switch (_mood) {
    case PET_MOOD_HAPPY:
      display.print(F("Happy!"));
      break;
    case PET_MOOD_SLEEPY:
      display.print(F("Sleepy"));
      break;
    case PET_MOOD_HUNGRY:
      display.print(F("Hungry"));
      break;
    default:
      display.print(F("OK"));
      break;
  }
}

void ScreenPet::_drawHappinessBar(Adafruit_SSD1306 &display) {
  GadgetSettings &cfg = _settings.get();
  
  // Happiness bar
  display.setTextSize(1);
  display.setCursor(70, 56);
  display.print(F("Joy:"));
  
  int barWidth = (cfg.petHappiness * 40) / 100;
  display.drawRect(96, 55, 30, 6, SSD1306_WHITE);
  display.fillRect(97, 56, barWidth * 28 / 40, 4, SSD1306_WHITE);
}

void ScreenPet::_drawFloatingParticles(Adafruit_SSD1306 &display) {
  // Draw simple hearts when happy
  display.setTextSize(1);
  display.setCursor(10, 20 + _posY);
  display.print(F("<3"));
  display.setCursor(100, 30 - _posY);
  display.print(F("<3"));
}

void ScreenPet::_updateMood() {
  GadgetSettings &cfg = _settings.get();
  
  if (cfg.petHunger < 30) {
    _mood = PET_MOOD_HUNGRY;
  } else if (cfg.petHappiness > 80) {
    _mood = PET_MOOD_HAPPY;
  } else if (cfg.petHappiness < 30) {
    _mood = PET_MOOD_SLEEPY;
  } else {
    _mood = PET_MOOD_NEUTRAL;
  }
  
  // Decrease pet hunger and happiness over time
  if (cfg.petHunger > 0) {
    cfg.petHunger = max((uint8_t)(cfg.petHunger - 1), (uint8_t)0);
  }
  if (cfg.petHappiness > 0) {
    cfg.petHappiness = max((uint8_t)(cfg.petHappiness - 1), (uint8_t)0);
  }
}

void ScreenPet::_updateAnimation() {
  // Don't change animation if doing special action
  if (_isActioning) {
    return;
  }
  
  unsigned long now = millis();
  
  // Change animation every 5-8 seconds
  if (now - _lastAnimationChange >= 5000 + (random(3000))) {
    _lastAnimationChange = now;
    _currentFrame = 0;
    
    // Reset position for new animation
    if (_currentAnimation == PET_ANIM_WALK_LEFT || _currentAnimation == PET_ANIM_WALK_RIGHT) {
      _posX = 48; // return to center
    }
    
    // Choose random animation based on mood
    uint8_t petType = _settings.get().petType;
    
    // Only cat has all animations for now
    if (petType == PET_CAT) {
      int randAnim = random(100);
      
      if (_mood == PET_MOOD_SLEEPY) {
        // More likely to sleep when sleepy
        if (randAnim < 50) {
          _currentAnimation = PET_ANIM_SLEEPING;
        } else if (randAnim < 70) {
          _currentAnimation = PET_ANIM_WALK_LEFT;
        } else {
          _currentAnimation = PET_ANIM_IDLE;
        }
      } else if (_mood == PET_MOOD_HUNGRY) {
        // More likely to look around when hungry
        if (randAnim < 40) {
          _currentAnimation = PET_ANIM_WALK_LEFT;
        } else if (randAnim < 60) {
          _currentAnimation = PET_ANIM_WALK_RIGHT;
        } else {
          _currentAnimation = PET_ANIM_IDLE;
        }
      } else if (_mood == PET_MOOD_HAPPY) {
        // More active when happy
        if (randAnim < 30) {
          _currentAnimation = PET_ANIM_WALK_LEFT;
        } else if (randAnim < 50) {
          _currentAnimation = PET_ANIM_WALK_RIGHT;
        } else if (randAnim < 70) {
          _currentAnimation = PET_ANIM_STUDYING;
        } else {
          _currentAnimation = PET_ANIM_IDLE;
        }
      } else {
        // Neutral mood - balanced variety
        if (randAnim < 20) {
          _currentAnimation = PET_ANIM_WALK_LEFT;
        } else if (randAnim < 40) {
          _currentAnimation = PET_ANIM_WALK_RIGHT;
        } else if (randAnim < 55) {
          _currentAnimation = PET_ANIM_STUDYING;
        } else if (randAnim < 70) {
          _currentAnimation = PET_ANIM_SLEEPING;
        } else {
          _currentAnimation = PET_ANIM_IDLE;
        }
      }
    } else {
      // Star and Robot only have idle for now
      _currentAnimation = PET_ANIM_IDLE;
    }
  }
}
