#ifndef SCREEN_PET_H
#define SCREEN_PET_H

#include "../config.h"
#include "../storage/settings.h"
#include <Arduino.h>
#include <Adafruit_SSD1306.h>

// ============================================================
//  Pet Screen - Animated Digital Pet
// ============================================================

enum PetMood {
  PET_MOOD_NEUTRAL,
  PET_MOOD_HAPPY,
  PET_MOOD_SLEEPY,
  PET_MOOD_HUNGRY
};

enum PetAnimation {
  PET_ANIM_IDLE,
  PET_ANIM_WALK_LEFT,
  PET_ANIM_WALK_RIGHT,
  PET_ANIM_EATING,
  PET_ANIM_SLEEPING,
  PET_ANIM_STUDYING
};

class ScreenPet {
public:
  ScreenPet(SettingsManager &settings);

  void render(Adafruit_SSD1306 &display);
  void update();

  void feed(); // Button interaction
  void pet();  // Button interaction
  PetMood getMood();

private:
  SettingsManager &_settings;

  uint8_t _currentFrame;
  unsigned long _lastFrameChange;
  unsigned long _lastMoodChange;
  PetMood _mood;
  bool _isActioning; // Doing special action
  unsigned long _actionStart;

  // Animation state
  PetAnimation _currentAnimation;
  unsigned long _lastAnimationChange;
  
  // Pet position (for movement and floating animation)
  int16_t _posX;
  int8_t _posY;
  bool _floatUp;
  unsigned long _lastFloat;

  void _drawPetSprite(Adafruit_SSD1306 &display);
  void _drawMoodIndicator(Adafruit_SSD1306 &display);
  void _drawHappinessBar(Adafruit_SSD1306 &display);
  void _drawFloatingParticles(Adafruit_SSD1306 &display);
  void _updateMood();
  void _updateAnimation();
  void _getAnimationFrame(const uint8_t **framePtr, int &frameCount);

  const uint8_t *_getIdleFrame(uint8_t petType, uint8_t frame);
  const uint8_t *_getHappyFrame(uint8_t petType);
  const uint8_t *_getSleepFrame(uint8_t petType);
};

#endif // SCREEN_PET_H
