#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include "types.h"

// Pushbuttons
constexpr uint8_t PIN_BUTTON_SINGLE = D0;
constexpr uint8_t PIN_BUTTON_DOUBLE = D1;

// Rotary encoder
constexpr uint8_t PIN_ENCODER_CLK = D2;
constexpr uint8_t PIN_ENCODER_DT  = D3;
constexpr uint8_t PIN_ENCODER_SW  = D6;

// OLED display (I2C)
constexpr uint8_t PIN_OLED_SDA = D4;
constexpr uint8_t PIN_OLED_SCL = D5;
constexpr uint8_t OLED_ADDRESS = 0x3C;

// Motor control
constexpr uint8_t PIN_MOTOR = D7;
constexpr bool ENABLE_MOTOR_OUTPUT = true;

// Manual grinding detection
constexpr uint8_t PIN_MANUAL_GRIND = D8;
constexpr uint32_t SAVE_PROMPT_TIMEOUT_MS = 10000;

// Timing
constexpr uint32_t INPUT_DEBOUNCE_MS = 20;
constexpr uint32_t DISPLAY_INTERVAL_MS = 40;

// Settings
constexpr uint32_t SETTINGS_TIMEOUT_MS = 10000;
constexpr uint32_t SETTINGS_CONFIRMATION_DURATION_MS = 800;
constexpr coffee_animation_mode DEFAULT_COFFEE_ANIMATION_MODE = coffee_animation_mode::PROGRESS;

// Standard animation timing
constexpr uint32_t COFFEE_STANDARD_INTERVAL_MS = 130;

// Debug configuration
constexpr bool ENABLE_DEBUG_MODE = true;

// Grinding time configuration
constexpr uint32_t DEFAULT_SINGLE_GRIND_TIME_MS = 5000;
constexpr uint32_t DEFAULT_DOUBLE_GRIND_TIME_MS = 10000;

constexpr uint32_t MIN_GRIND_TIME_MS = 0;
constexpr uint32_t MAX_GRIND_TIME_MS = 30000;
constexpr uint32_t GRIND_TIME_STEP_MS = 100;

// Display configuration
constexpr uint8_t DEFAULT_DISPLAY_BRIGHTNESS = 80;
constexpr uint32_t DEFAULT_STANDBY_TIMEOUT_MS = 30000;
constexpr uint32_t MAX_STANDBY_TIMEOUT_MS = 3600000;
constexpr uint8_t MIN_DISPLAY_BRIGHTNESS = 0;
constexpr uint8_t MAX_DISPLAY_BRIGHTNESS = 100;
constexpr uint8_t DISPLAY_BRIGHTNESS_STEP = 10;

// Grinding completion display
constexpr uint32_t GRINDING_FINISHED_DURATION_MS = 2500;

// Animated checkmark timing
constexpr uint32_t CHECKMARK_FIRST_STROKE_MS = 150;
constexpr uint32_t CHECKMARK_SECOND_STROKE_MS = 300;

// Animated DONE text
constexpr uint32_t DONE_LETTER_INTERVAL_MS = 150;

#endif