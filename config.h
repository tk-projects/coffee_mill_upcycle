#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

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

// Timing
constexpr uint32_t INPUT_DEBOUNCE_MS = 20;
constexpr uint32_t DISPLAY_INTERVAL_MS = 40;

// Settings
constexpr uint32_t SETTINGS_TIMEOUT_MS = 5000;

// Debug configuration
constexpr bool ENABLE_DEBUG_MODE = true;

// Grinding time configuration
constexpr uint32_t DEFAULT_SINGLE_GRIND_TIME_MS = 5000;
constexpr uint32_t DEFAULT_DOUBLE_GRIND_TIME_MS = 18000;

constexpr uint32_t MIN_GRIND_TIME_MS = 0;
constexpr uint32_t MAX_GRIND_TIME_MS = 300000;
constexpr uint32_t GRIND_TIME_STEP_MS = 100;

// Display configuration
constexpr uint8_t DEFAULT_DISPLAY_BRIGHTNESS = 128;

constexpr uint32_t DEFAULT_STANDBY_TIMEOUT_MS = 30000;
constexpr uint32_t MAX_STANDBY_TIMEOUT_MS = 3600000;

#endif