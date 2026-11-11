#include "settings_manager.h"
#include <FlashStorage.h>

// Flash identification
constexpr uint32_t SETTINGS_MAGIC = 0x434D494C;
constexpr uint16_t SETTINGS_VERSION = 1;

// Data stored in flash
struct settings_data {

    uint32_t magic;
    uint16_t version;

    uint32_t single_grind_time_ms;
    uint32_t double_grind_time_ms;

    uint8_t display_brightness;
    uint32_t standby_timeout_ms;

    language current_language;
};

// Flash storage instance
FlashStorage(settingsStorage, settings_data);

// Save settings to flash
void settings_manager::save_settings() {

    // Check if settings have changed
    if (!settings_dirty) {
        return;
    }

    // Prepare flash data
    settings_data data{};

    data.magic = SETTINGS_MAGIC;
    data.version = SETTINGS_VERSION;

    data.single_grind_time_ms = single_grind_time_ms;
    data.double_grind_time_ms = double_grind_time_ms;

    data.display_brightness = display_brightness;
    data.standby_timeout_ms = standby_timeout_ms;

    data.current_language = current_language;

    // Write settings to flash
    settingsStorage.write(data);

    // Reset dirty flag
    settings_dirty = false;

    if (ENABLE_DEBUG_MODE) {
        Serial.println("Settings saved to flash");
    }
}


void settings_manager::init() {
    // Load settings from flash
    load_settings();

    Serial.println("Settings manager initialized");
}

// Load settings from flash
void settings_manager::load_settings() {

    // Read data from flash
    settings_data saved = settingsStorage.read();

    // Validate flash identification
    if (saved.magic != SETTINGS_MAGIC ||
        saved.version != SETTINGS_VERSION) {

        Serial.println("No valid settings found. Using defaults.");
        return;
    }

    // Validate stored values
    if (saved.single_grind_time_ms > 300000 ||
        saved.double_grind_time_ms > 300000 ||
        saved.standby_timeout_ms > 3600000 ||
        saved.current_language > language::GERMAN) {

        Serial.println("Invalid settings data. Using defaults.");
        return;
    }

    // Apply stored settings
    single_grind_time_ms = saved.single_grind_time_ms;
    double_grind_time_ms = saved.double_grind_time_ms;

    display_brightness = saved.display_brightness;
    standby_timeout_ms = saved.standby_timeout_ms;

    current_language = saved.current_language;

    Serial.println("Settings loaded from flash");
}

uint32_t settings_manager::get_single_grind_time() const {
    return single_grind_time_ms;
}

uint32_t settings_manager::get_double_grind_time() const {
    return double_grind_time_ms;
}

uint32_t settings_manager::validate_grind_time(uint32_t time_ms) const {

    // Limit maximum time
    if (time_ms > MAX_GRIND_TIME_MS) {
        time_ms = MAX_GRIND_TIME_MS;
    }

    // Round to nearest 100 ms
    time_ms =
        ((time_ms + GRIND_TIME_STEP_MS / 2) /
         GRIND_TIME_STEP_MS) * GRIND_TIME_STEP_MS;

    return time_ms;
}

void settings_manager::set_single_grind_time(uint32_t time_ms) {

    uint32_t new_time = validate_grind_time(time_ms);

    if (new_time != single_grind_time_ms) {

        single_grind_time_ms = new_time;
        settings_dirty = true;
    }
}

void settings_manager::set_double_grind_time(uint32_t time_ms) {

    uint32_t new_time = validate_grind_time(time_ms);

    if (new_time != double_grind_time_ms) {

        double_grind_time_ms = new_time;
        settings_dirty = true;
    }
}

uint8_t settings_manager::get_display_brightness() const {
    return display_brightness;
}

void settings_manager::set_display_brightness(uint8_t brightness) {

    if (brightness != display_brightness) {

        display_brightness = brightness;
        settings_dirty = true;
    }
}


uint32_t settings_manager::get_standby_timeout() const {
    return standby_timeout_ms;
}

void settings_manager::set_standby_timeout(uint32_t timeout_ms) {

    uint32_t new_timeout = min(
        timeout_ms,
        MAX_STANDBY_TIMEOUT_MS
    );

    if (new_timeout != standby_timeout_ms) {

        standby_timeout_ms = new_timeout;
        settings_dirty = true;
    }
}

language settings_manager::get_language() const {
    return current_language;
}

void settings_manager::set_language(language new_language) {

    if (new_language != language::ENGLISH &&
        new_language != language::GERMAN) {
        return;
    }

    if (new_language != current_language) {

        current_language = new_language;
        settings_dirty = true;
    }
}