#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include <Arduino.h>
#include "types.h"
#include "config.h"

class settings_manager {

public:
    void init();

    // Grinding times
    uint32_t get_single_grind_time() const;
    uint32_t get_double_grind_time() const;

    void set_single_grind_time(uint32_t time_ms);
    void set_double_grind_time(uint32_t time_ms);

    // Display
    uint8_t get_display_brightness() const;
    void set_display_brightness(uint8_t brightness);

    uint32_t get_standby_timeout() const;
    void set_standby_timeout(uint32_t timeout_ms);

    // Language
    language get_language() const;
    void set_language(language new_language);
    void save_settings();

private:
    // Settings values (initialized with defaults)
    uint32_t single_grind_time_ms = DEFAULT_SINGLE_GRIND_TIME_MS;
    uint32_t double_grind_time_ms = DEFAULT_DOUBLE_GRIND_TIME_MS;

    uint8_t display_brightness = DEFAULT_DISPLAY_BRIGHTNESS;
    uint32_t standby_timeout_ms = DEFAULT_STANDBY_TIMEOUT_MS;

    language current_language = language::ENGLISH;

    // Settings status
    bool settings_dirty = false;

    // Internal functions
    void load_settings();
    uint32_t validate_grind_time(uint32_t time_ms) const;
};

#endif