#include "display_manager.h"
#include "display_assets.h"

// Initialize display manager
void display_manager::init(const settings_manager &settings,
                           const grinder_controller &grinder) {

    // Store references to other modules
    this->settings = &settings;
    this->grinder = &grinder;

    // Initialize OLED
    oled.begin();

    // Set display brightness
    oled.setContrast(settings.get_display_brightness());

    // Initialize display state
    displayed_state = state::STANDBY;
    display_dirty = true;
    last_display_update = 0;

    // Clear display
    oled.clearBuffer();
    oled.sendBuffer();

    Serial.println("Display manager initialized");
}

/// Cyclic update
void display_manager::update(state current_state, grind_mode current_mode, uint32_t now) {

    // Detect state change
    if (current_state != displayed_state) {

        // Start coffee animation when entering AUTO_GRINDING
        if (current_state == state::AUTO_GRINDING) {
            animation_start_time = now;
        }

        displayed_state = current_state;
        display_dirty = true;
    }

    // Check if cyclic refresh is required
    bool cyclic_refresh =
        current_state == state::AUTO_GRINDING ||
        current_state == state::MANUAL_GRINDING;

    // Nothing changed and no cyclic refresh required
    if (!display_dirty && !cyclic_refresh) {
        return;
    }

    // Limit display refresh rate
    if (now - last_display_update < DISPLAY_INTERVAL_MS) {
        return;
    }

    // Clear display buffer
    oled.clearBuffer();

    // Draw current screen
    switch (current_state) {

        case state::STANDBY:
            draw_standby();
            break;

        case state::AUTO_GRINDING:
            draw_auto_grinding(current_mode, now);
            break;

        case state::MANUAL_GRINDING:
            draw_manual_grinding(now);
            break;

        case state::SAVE_PROMPT:
            draw_save_prompt();
            break;

        case state::SETTINGS:
            draw_settings();
            break;
    }

    // Send buffer to OLED
    oled.sendBuffer();

    // Update refresh information
    last_display_update = now;
    display_dirty = false;
}


void display_manager::draw_standby() {

    // Read grinding times
    uint32_t single_time = settings->get_single_grind_time();
    uint32_t double_time = settings->get_double_grind_time();

    // Convert milliseconds to tenths of seconds
    uint32_t single_ds = single_time / 100;
    uint32_t double_ds = double_time / 100;

    // Draw coffee beans
    oled.drawXBMP(54, -2,
                 COFFEE_ICON_WIDTH,
                 COFFEE_ICON_HEIGHT,
                 coffee_frame_0);

    // Draw READY
    oled.setFont(u8g2_font_helvR10_tf);
    //oled.drawStr(45, 14, "READY");

    // Use same font for labels and values
    oled.setFont(u8g2_font_helvR12_tf);

    // Single
    oled.drawStr(8, 37, "Single");

    char time_buffer[12];

    // Format single time
    snprintf(time_buffer, sizeof(time_buffer), "%lu.%lu",
            (unsigned long)(single_ds / 10),
            (unsigned long)(single_ds % 10));

    
    oled.setFont(u8g2_font_helvR14_tf);

    // Right-align number
    oled.drawStr(110 - oled.getStrWidth(time_buffer), 37, time_buffer);

    // Draw unit at fixed position
    oled.setFont(u8g2_font_helvR12_tf);
    oled.drawStr(114, 37, "s");

    // Double
    oled.setFont(u8g2_font_helvR12_tf);
    oled.drawStr(8, 60, "Double");
    oled.setFont(u8g2_font_helvR14_tf);

    // Format double time
    snprintf(time_buffer, sizeof(time_buffer), "%lu.%lu",
            (unsigned long)(double_ds / 10),
            (unsigned long)(double_ds % 10));

    // Right-align number
    oled.drawStr(110 - oled.getStrWidth(time_buffer), 60, time_buffer);

    // Draw unit at fixed position
    oled.setFont(u8g2_font_helvR12_tf);
    oled.drawStr(114, 60, "s");
}


void display_manager::draw_settings() {

    oled.setFont(u8g2_font_helvB14_tr);
    oled.drawStr(20, 38, "SETTINGS");
}


void display_manager::draw_auto_grinding(grind_mode mode, uint32_t now) {

    // Read grinding times
    uint32_t remaining = grinder->get_remaining_time(now);
    uint32_t elapsed = grinder->get_elapsed_time(now);

    // Calculate total duration
    uint32_t duration = elapsed + remaining;

    // Convert remaining time to tenths of seconds
    uint32_t remaining_ds = (remaining + 99) / 100;

    uint32_t seconds = remaining_ds / 10;
    uint32_t decimals = remaining_ds % 10;

    // Draw coffee beans at the same position as standby
    // Draw animated coffee beans
    draw_coffee_animation(now);

    // Display grinding mode
    oled.setFont(u8g2_font_helvR08_tf);

    if (mode == grind_mode::SINGLE) {
        oled.drawStr(83, 12, "SINGLE");
    } else {
        oled.drawStr(83, 12, "DOUBLE");
    }

    // Format countdown
    char time_buffer[12];

    snprintf(time_buffer, sizeof(time_buffer), "%lu.%lu",
             (unsigned long)seconds,
             (unsigned long)decimals);

    // Calculate number width
    oled.setFont(u8g2_font_helvR18_tf);
    uint8_t number_width = oled.getStrWidth(time_buffer);

    // Calculate unit width
    oled.setFont(u8g2_font_helvR10_tf);
    uint8_t unit_width = oled.getStrWidth("s");

    // Calculate combined width
    const uint8_t spacing = 4;
    uint8_t total_width = number_width + spacing + unit_width;

 
    // Center countdown with optical correction
    const int8_t countdown_offset_x = 6;

    int16_t x = (128 - total_width) / 2 + countdown_offset_x;

    // Draw number
    oled.setFont(u8g2_font_helvR18_tf);
    oled.drawStr(x, 43, time_buffer);

    // Draw unit
    oled.setFont(u8g2_font_helvR10_tf);
    oled.drawStr(x + number_width + spacing, 43, "s");

    // Progress bar dimensions
    const uint8_t bar_x = 5;
    const uint8_t bar_y = 51;
    const uint8_t bar_width = 118;
    const uint8_t bar_height = 11;

    // Draw progress bar frame
    oled.drawFrame(bar_x, bar_y, bar_width, bar_height);

    // Calculate and draw progress
    if (duration > 0) {

        uint32_t progress_width =
            ((uint64_t)elapsed * (bar_width - 4)) / duration;

        oled.drawBox(
            bar_x + 2,
            bar_y + 2,
            progress_width,
            bar_height - 4
        );
    }
}

void display_manager::draw_manual_grinding(uint32_t now) {

    oled.setFont(u8g2_font_6x12_tf);
    oled.drawStr(15, 20, "MANUAL GRINDING");
}


void display_manager::draw_save_prompt() {

    oled.setFont(u8g2_font_helvB12_tr);
    oled.drawStr(12, 38, "SAVE TIME?");
}


// Draw animated coffee beans
void display_manager::draw_coffee_animation(uint32_t now) {

    // Calculate animation time
    uint32_t elapsed = now - animation_start_time;

    // Calculate current frame
    uint8_t frame =
        (elapsed / COFFEE_ANIMATION_INTERVAL_MS) %
        COFFEE_ANIMATION_FRAMES;

    // Select bitmap
    const uint8_t* bitmap = coffee_animation_frames[frame];

    // Draw animation
    oled.drawXBMP(
        54,
        -2,
        COFFEE_ICON_WIDTH,
        COFFEE_ICON_HEIGHT,
        bitmap
    );
}