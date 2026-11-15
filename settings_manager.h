
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

    // Flash storage
    void save_settings();

    // Menu navigation
    void next();
    void previous();

    // Menu editing
    void increase();
    void decrease();
    void toggle_edit();

    // Settings session
    void begin_session();
    bool has_changes() const;
    void discard_changes();

    // Save confirmation
    void enter_confirmation();
    void toggle_confirmation_choice();
    confirmation_choice get_confirmation_choice() const;
    void enter_confirmed();

    // Menu status
    setting_item get_selected_item() const;
    settings_mode get_mode() const;

    // Menu display
    const char* get_item_name(setting_item item) const;
    void get_item_value(setting_item item, char* buffer, size_t size) const;

    coffee_animation_mode get_animation_mode() const;
    void set_animation_mode(coffee_animation_mode mode);

private:
    // Settings values (initialized with defaults)
    uint32_t single_grind_time_ms = DEFAULT_SINGLE_GRIND_TIME_MS;
    uint32_t double_grind_time_ms = DEFAULT_DOUBLE_GRIND_TIME_MS;

    uint8_t display_brightness = DEFAULT_DISPLAY_BRIGHTNESS;
    uint32_t standby_timeout_ms = DEFAULT_STANDBY_TIMEOUT_MS;

    language current_language = language::ENGLISH;

    // Original settings before editing
    uint32_t original_single_grind_time_ms;
    uint32_t original_double_grind_time_ms;

    uint8_t original_display_brightness;
    uint32_t original_standby_timeout_ms;

    coffee_animation_mode animation_mode = DEFAULT_COFFEE_ANIMATION_MODE;
    coffee_animation_mode original_animation_mode = DEFAULT_COFFEE_ANIMATION_MODE;

    // Currently selected confirmation option
    confirmation_choice selected_confirmation = confirmation_choice::SAVE;

    // Settings status
    bool settings_dirty = false;

    // Menu status
    static constexpr uint8_t SETTINGS_COUNT =
        static_cast<uint8_t>(setting_item::COUNT);

    uint8_t selected_index = 0;
    settings_mode mode = settings_mode::NAVIGATE;

    // Internal functions
    void load_settings();
    uint32_t validate_grind_time(uint32_t time_ms) const;
};

#endif
