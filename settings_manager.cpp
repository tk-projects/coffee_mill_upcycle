#include "settings_manager.h"
#include <FlashStorage.h>
#include <stdio.h>

// Flash identification
constexpr uint32_t SETTINGS_MAGIC = 0x434D494C;
constexpr uint16_t SETTINGS_VERSION = 2;

// Data stored in flash
struct settings_data {

    uint32_t magic;
    uint16_t version;

    uint32_t single_grind_time_ms;
    uint32_t double_grind_time_ms;

    uint8_t display_brightness;
    uint32_t standby_timeout_ms;

    language current_language;

    coffee_animation_mode animation_mode;
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

    data.animation_mode = animation_mode;

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

    // Store original settings
    original_single_grind_time_ms = single_grind_time_ms;
    original_double_grind_time_ms = double_grind_time_ms;
    original_display_brightness = display_brightness;
    original_standby_timeout_ms = standby_timeout_ms;
    original_animation_mode = animation_mode;

    selected_index = 0;
    mode = settings_mode::NAVIGATE;
    
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

    if (saved.animation_mode != coffee_animation_mode::OFF &&
        saved.animation_mode != coffee_animation_mode::STANDARD &&
        saved.animation_mode != coffee_animation_mode::PROGRESS) {

        Serial.println("Invalid animation mode. Using defaults.");
        return;
    }
    animation_mode = saved.animation_mode;
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

    // Limit brightness to valid range
    brightness = constrain(
        brightness,
        MIN_DISPLAY_BRIGHTNESS,
        MAX_DISPLAY_BRIGHTNESS
    );

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


// Navigate to next menu item
void settings_manager::next() {
    selected_index = (selected_index + 1) % SETTINGS_COUNT;
}

// Navigate to previous menu item
void settings_manager::previous() {
    selected_index =
        (selected_index + SETTINGS_COUNT - 1) % SETTINGS_COUNT;
}

// Switch between navigation and editing
void settings_manager::toggle_edit() {
    mode = (mode == settings_mode::NAVIGATE)
        ? settings_mode::EDIT
        : settings_mode::NAVIGATE;
}

// Get currently selected menu item
setting_item settings_manager::get_selected_item() const {
    return static_cast<setting_item>(selected_index);
}

// Get current menu mode
settings_mode settings_manager::get_mode() const {
    return mode;
}

// Increase selected setting
void settings_manager::increase() {

    switch (get_selected_item()) {

        case setting_item::SINGLE:
            if (single_grind_time_ms < MAX_GRIND_TIME_MS) {
                set_single_grind_time(
                    single_grind_time_ms + GRIND_TIME_STEP_MS);
            }
            break;

        case setting_item::DOUBLE:
            if (double_grind_time_ms < MAX_GRIND_TIME_MS) {
                set_double_grind_time(
                    double_grind_time_ms + GRIND_TIME_STEP_MS);
            }
            break;

        case setting_item::BRIGHTNESS:
            set_display_brightness(
                min(static_cast<int>(display_brightness) + DISPLAY_BRIGHTNESS_STEP, static_cast<int>(MAX_DISPLAY_BRIGHTNESS))
            );
            break;


        case setting_item::STANDBY_TIMEOUT:
            if (standby_timeout_ms <=
                MAX_STANDBY_TIMEOUT_MS - 5000) {
                set_standby_timeout(standby_timeout_ms + 5000);
            }
            break;

        case setting_item::ANIMATION: {
            uint8_t next_mode = (static_cast<uint8_t>(animation_mode) + 1) % 3;
            set_animation_mode(static_cast<coffee_animation_mode>(next_mode));

            break;
        }

        default:
            break;
    }
}

// Decrease selected setting
void settings_manager::decrease() {

    switch (get_selected_item()) {

        case setting_item::SINGLE:
            if (single_grind_time_ms >= GRIND_TIME_STEP_MS) {
                set_single_grind_time(
                    single_grind_time_ms - GRIND_TIME_STEP_MS);
            }
            break;

        case setting_item::DOUBLE:
            if (double_grind_time_ms >= GRIND_TIME_STEP_MS) {
                set_double_grind_time(
                    double_grind_time_ms - GRIND_TIME_STEP_MS);
            }
            break;

        case setting_item::BRIGHTNESS:
            set_display_brightness(
                max(
                    static_cast<int>(display_brightness) - DISPLAY_BRIGHTNESS_STEP,
                    static_cast<int>(MIN_DISPLAY_BRIGHTNESS)
                )
            );
            break;

        case setting_item::STANDBY_TIMEOUT:
            if (standby_timeout_ms >= 5000) {
                set_standby_timeout(standby_timeout_ms - 5000);
            }
            break;

        case setting_item::ANIMATION: {
            uint8_t previous_mode = (static_cast<uint8_t>(animation_mode) + 3 - 1) % 3;
            set_animation_mode(static_cast<coffee_animation_mode>(previous_mode));
            break;
        }

        default:
            break;
    }
}

// Get menu item name
const char* settings_manager::get_item_name(setting_item item) const {

    switch (item) {
        case setting_item::SINGLE:          return "Single";
        case setting_item::DOUBLE:          return "Double";
        case setting_item::BRIGHTNESS:      return "Brightness";
        case setting_item::STANDBY_TIMEOUT: return "Standby";
        case setting_item::ANIMATION:       return "Animation";
        case setting_item::EXIT:            return "Exit";
        default:                            return "";
    }
}

// Format menu item value for display
void settings_manager::get_item_value(
    setting_item item,
    char* buffer,
    size_t size) const {

    switch (item) {

        case setting_item::SINGLE:
            snprintf(buffer, size, "%lu.%lu s",
                (unsigned long)(single_grind_time_ms / 1000),
                (unsigned long)((single_grind_time_ms % 1000) / 100));
            break;

        case setting_item::DOUBLE:
            snprintf(buffer, size, "%lu.%lu s",
                (unsigned long)(double_grind_time_ms / 1000),
                (unsigned long)((double_grind_time_ms % 1000) / 100));
            break;

        case setting_item::BRIGHTNESS:
            snprintf(buffer, size, "%u%%", display_brightness);
            break;

        case setting_item::STANDBY_TIMEOUT:
            snprintf(buffer, size, "%lu s",
                (unsigned long)(standby_timeout_ms / 1000));
            break;
        
        case setting_item::ANIMATION: {
            switch (animation_mode) {
                case coffee_animation_mode::OFF:
                    snprintf(buffer, size, "Off");
                    break;

                case coffee_animation_mode::STANDARD:
                    snprintf(buffer, size, "Short");
                    break;

                case coffee_animation_mode::PROGRESS:
                    snprintf(buffer, size, "Long");
                    break;

                default:
                    snprintf(buffer, size, "?");
                    break;
            }
            break;
        }

        default:
            if (size > 0) buffer[0] = '\0';
            break;
    }
}

bool settings_manager::has_changes() const {
    return
        single_grind_time_ms != original_single_grind_time_ms ||
        double_grind_time_ms !=  original_double_grind_time_ms ||
        display_brightness != original_display_brightness ||
        standby_timeout_ms != original_standby_timeout_ms ||
        animation_mode != original_animation_mode;
}

void settings_manager::discard_changes() {
    // Restore original settings
    single_grind_time_ms = original_single_grind_time_ms;
    double_grind_time_ms = original_double_grind_time_ms;
    display_brightness = original_display_brightness;
    standby_timeout_ms = original_standby_timeout_ms;
    animation_mode =  original_animation_mode;
    mode = settings_mode::NAVIGATE;
}

void settings_manager::enter_confirmation() {

    mode = settings_mode::CONFIRM;

    selected_confirmation = confirmation_choice::SAVE;
}

void settings_manager::toggle_confirmation_choice() {

    if (selected_confirmation ==
        confirmation_choice::SAVE) {

        selected_confirmation =
            confirmation_choice::DISCARD;

    } else {

        selected_confirmation =
            confirmation_choice::SAVE;
    }
}

confirmation_choice settings_manager::get_confirmation_choice() const {
    return selected_confirmation;
}

coffee_animation_mode settings_manager::get_animation_mode() const {
    return animation_mode;
}

void settings_manager::set_animation_mode(coffee_animation_mode mode) {
    // Validate animation mode
    if (mode != coffee_animation_mode::OFF &&
        mode != coffee_animation_mode::STANDARD &&
        mode != coffee_animation_mode::PROGRESS) {
        return;
    }

    // Update setting
    if (mode != animation_mode) {

        animation_mode = mode;
        settings_dirty = true;
    }
}

void settings_manager::begin_session() {

    // Store original settings
    original_single_grind_time_ms = single_grind_time_ms;
    original_double_grind_time_ms = double_grind_time_ms;
    original_display_brightness = display_brightness;
    original_standby_timeout_ms = standby_timeout_ms;
    original_animation_mode = animation_mode;

    // Initialize menu
    selected_index = 0;
    mode = settings_mode::NAVIGATE;

    selected_confirmation = confirmation_choice::SAVE;
}

void settings_manager::enter_confirmed() {
    mode = settings_mode::CONFIRMED;
}