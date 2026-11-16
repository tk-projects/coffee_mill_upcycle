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

    // Apply stored brightness
    apply_brightness();

    // Initialize display state
    displayed_state = state::STANDBY;
    display_dirty = true;
    last_display_update = 0;

    // Clear display
    oled.clearBuffer();
    oled.sendBuffer();

    Serial.println("Display manager initialized");
}

void display_manager::invalidate() {
    display_dirty = true;
}


/// Cyclic update
void display_manager::update(state current_state, grind_mode current_mode, uint32_t now, uint32_t manual_grind_time_ms, save_option_manuel_grind selected_option, bool save_prompt_confirmed) {
    // Check if brightness has changed
    if (settings->get_display_brightness() !=
        last_display_brightness) {

        apply_brightness();
    }

    // Detect state change
    if (current_state != displayed_state) {

        // Start animation when entering the corresponding state
        if (current_state == state::AUTO_GRINDING ||
            current_state == state::GRINDING_FINISHED) {

            animation_start_time = now;
        }

        displayed_state = current_state;
        display_dirty = true;
    }

    // Check if cyclic refresh is required
    bool cyclic_refresh =
        current_state == state::AUTO_GRINDING ||
        current_state == state::MANUAL_GRINDING ||
        current_state == state::GRINDING_FINISHED;

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

        case state::AUTO_GRINDING:{
            // Get current selected grinding duration --> Better implement function in grinder control and get grind duration from there
            uint32_t grind_duration_ms = 0;
            if (current_mode == grind_mode::SINGLE) {
                grind_duration_ms = settings->get_single_grind_time();
            } else {
                grind_duration_ms = settings->get_double_grind_time();
            }

            draw_auto_grinding(current_mode, now, grind_duration_ms);
            break;
        }
        case state::MANUAL_GRINDING:
            draw_manual_grinding(manual_grind_time_ms);
            break;

        case state::SAVE_PROMPT:
            draw_save_prompt(manual_grind_time_ms, selected_option, save_prompt_confirmed);
            break;

        case state::SETTINGS:
            draw_settings();
            break;
        case state::GRINDING_FINISHED:
            draw_grinding_finished(current_mode, now);
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
    // Draw confirmation dialog instead of settings menu
    if (settings->get_mode() == settings_mode::CONFIRM || settings->get_mode() == settings_mode::CONFIRMED) {
        draw_settings_confirmation();
        return;
    }

    // Menu layout
    constexpr int DISPLAY_WIDTH = 128;
    constexpr int ROW_HEIGHT = 17;
    constexpr int FIRST_ROW_Y = 12;
    constexpr int VISIBLE_ROWS = 3;
    constexpr int ITEM_COUNT = static_cast<int>(setting_item::COUNT);

    oled.setDrawColor(1);

    // Draw header
    oled.setFont(u8g2_font_5x7_tf);

    const char* title = settings->has_changes() ? "SETTINGS *" : "SETTINGS";

    oled.drawStr(
        (DISPLAY_WIDTH - oled.getStrWidth(title)) / 2,
        8,
        title
    );

    // Draw separator
    oled.drawHLine(0, 11, DISPLAY_WIDTH);

    // Get currently selected menu item
    int selected_index = static_cast<int>(
        settings->get_selected_item()
    );

    // Get current editing mode
    const bool editing =
        settings->get_mode() == settings_mode::EDIT;

    // Draw visible menu items
    for (int row = 0; row < VISIBLE_ROWS; row++) {

        // Keep selected item in center
        int item_index =
            (selected_index + row - 1 + ITEM_COUNT)
            % ITEM_COUNT;

        setting_item item =
            static_cast<setting_item>(item_index);

        // Get item name
        const char* name =
            settings->get_item_name(item);

        // Get formatted value
        char value[16];

        settings->get_item_value(
            item,
            value,
            sizeof(value)
        );

        // Individual row positions and heights
        int row_y;
        int row_height;
        int text_y;

        if (row == 0) {

            // Previous item
            row_y = 12;
            row_height = 15;
            text_y = 23;

        } else if (row == 1) {

            // Selected item
            row_y = 27;
            row_height = 22;
            text_y = 43;

        } else {

            // Next item
            row_y = 49;
            row_height = 15;
            text_y = 60;
        }

        // Select font depending on row and menu mode
        if (row == 1) {

            if (editing) {
                oled.setFont(u8g2_font_helvB10_tf);
            } else {
                oled.setFont(u8g2_font_helvR10_tf);
            }

        } else {

            oled.setFont(u8g2_font_helvR08_tf);;
        }

        // Right-align value
        int value_x =
            DISPLAY_WIDTH - 4 -
            oled.getStrWidth(value);

        // Highlight selected row
        if (row == 1) {

            // White background
            oled.setDrawColor(1);

            oled.drawBox(
                0,
                row_y,
                DISPLAY_WIDTH,
                row_height
            );

            // Black text
            oled.setDrawColor(0);

        } else {

            // White text
            oled.setDrawColor(1);
        }

        // Draw label and value
        oled.drawStr(4, text_y, name);
        oled.drawStr(value_x, text_y, value);

        // Restore default drawing color
        oled.setDrawColor(1);
    }
}


void display_manager::draw_auto_grinding(grind_mode mode, uint32_t now, uint32_t grind_duration_ms) {

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
    uint8_t x_pos_grind_type = 43;

    // Draw selected coffee animation
    switch (settings->get_animation_mode()) {

        case coffee_animation_mode::OFF:
            break;

        case coffee_animation_mode::STANDARD:
            draw_coffee_standard_animation(now);
            x_pos_grind_type = 83;
            break;

        case coffee_animation_mode::PROGRESS:
            draw_coffee_animation(now, grind_duration_ms);
            x_pos_grind_type = 83;
            break;
    }

    // Display grinding mode
    oled.setFont(u8g2_font_helvR08_tf);

    if (mode == grind_mode::SINGLE) {
        //oled.drawStr(83, 12, "SINGLE");
        oled.drawStr(x_pos_grind_type, 12, "SINGLE");
    } else {
        //oled.drawStr(83, 12, "DOUBLE");
        oled.drawStr(x_pos_grind_type, 12, "DOUBLE");
    }

    // Format countdown
    char time_buffer[12];

    snprintf(time_buffer, sizeof(time_buffer), "%lu.%lu",
             (unsigned long)seconds,
             (unsigned long)decimals);

    // Select countdown font depending on animation mode
    const uint8_t* countdown_font;

    switch (settings->get_animation_mode()) {

        case coffee_animation_mode::OFF:
            countdown_font = u8g2_font_logisoso22_tf;
            break;

        case coffee_animation_mode::STANDARD:
            countdown_font = u8g2_font_logisoso20_tf;
            break;

        case coffee_animation_mode::PROGRESS:
        default:
            countdown_font = u8g2_font_logisoso16_tf;
            break;
    }

    // Calculate number width
    oled.setFont(countdown_font);
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
    oled.setFont(countdown_font);
    oled.drawStr(x, 43, time_buffer);

    // Draw unit
    oled.setFont(u8g2_font_helvR10_tf);
    oled.drawStr(x + number_width + spacing, 43, "s");

    // Progress bar dimensions
    const uint8_t bar_x = 5;
    const uint8_t bar_y = 51;
    const uint8_t bar_width = 118;
    const uint8_t bar_height = 11;

    // Corner radius
    const uint8_t bar_radius = 4;

    // Draw rounded progress bar frame
    oled.drawRFrame(
        bar_x,
        bar_y,
        bar_width,
        bar_height,
        bar_radius
    );

    // Calculate and draw progress
    if (duration > 0) {

        uint32_t progress_width =
            ((uint64_t)elapsed * (bar_width - 4)) / duration;

        if (progress_width > 0) {

            uint8_t fill_radius = min(
                (uint32_t)2,
                progress_width / 2
            );

            oled.drawRBox(
                bar_x + 2,
                bar_y + 2,
                progress_width,
                bar_height - 4,
                fill_radius
            );
        }
    }
}

void display_manager::draw_manual_grinding(uint32_t elapsed_ms) {
    // Convert elapsed time to tenths of seconds
    uint32_t elapsed_ds = elapsed_ms / 100;
    uint32_t seconds = elapsed_ds / 10;
    uint32_t decimals = elapsed_ds % 10;

    // Display grinding mode
    oled.setFont(u8g2_font_helvR08_tf);
    oled.drawStr(79, 12, "MANUAL");

    // Format elapsed time
    char time_buffer[12];

    snprintf(
        time_buffer,
        sizeof(time_buffer),
        "%lu.%lu",
        (unsigned long)seconds,
        (unsigned long)decimals
    );

    // Calculate number width
    oled.setFont(u8g2_font_helvR18_tf);
    uint8_t number_width = oled.getStrWidth(time_buffer);

    // Calculate unit width
    oled.setFont(u8g2_font_helvR10_tf);
    uint8_t unit_width = oled.getStrWidth("s");

    // Calculate combined width
    const uint8_t spacing = 4;

    uint8_t total_width =
        number_width + spacing + unit_width;

    // Center elapsed time
    const int8_t countdown_offset_x = 6;

    int16_t x =
        (128 - total_width) / 2 + countdown_offset_x;

    // Draw elapsed time
    oled.setFont(u8g2_font_helvR18_tf);
    oled.drawStr(x, 43, time_buffer);

    // Draw unit
    oled.setFont(u8g2_font_helvR10_tf);
    oled.drawStr(x + number_width + spacing, 43, "s");

    // Draw grinding label
    oled.setFont(u8g2_font_helvR08_tf);

    const char* label = "GRINDING";

    oled.drawStr(
        (128 - oled.getStrWidth(label)) / 2,
        59,
        label
    );
}


void display_manager::draw_save_prompt(uint32_t duration_ms,save_option_manuel_grind selected_option, bool confirmed) {
    // Draw header
    oled.setFont(u8g2_font_helvR08_tf);

    const char* header = confirmed
        ? "GRIND TIME SAVED"
        : "SAVE GRIND TIME?";

    oled.drawStr(
        (128 - oled.getStrWidth(header)) / 2,
        11,
        header
    );

    // Convert grinding duration
    uint32_t duration_ds = duration_ms / 100;

    uint32_t seconds = duration_ds / 10;
    uint32_t decimals = duration_ds % 10;

    // Format measured time
    char time_buffer[12];

    snprintf(
        time_buffer,
        sizeof(time_buffer),
        "%lu.%lu",
        (unsigned long)seconds,
        (unsigned long)decimals
    );

    // Calculate number width
    oled.setFont(u8g2_font_helvR18_tf);
    uint8_t number_width = oled.getStrWidth(time_buffer);

    // Calculate unit width
    oled.setFont(u8g2_font_helvR10_tf);
    uint8_t unit_width = oled.getStrWidth("s");

    const uint8_t spacing = 4;

    uint8_t total_width =
        number_width + spacing + unit_width;

    int16_t x = (128 - total_width) / 2;

    // Draw measured time
    oled.setFont(u8g2_font_helvR18_tf);
    oled.drawStr(x, 36, time_buffer);

    // Draw unit
    oled.setFont(u8g2_font_helvR10_tf);
    oled.drawStr(x + number_width + spacing, 36, "s");

    // Save options
    const char* labels[] = {
        "NO",
        "SINGLE",
        "DOUBLE"
    };

    const uint8_t option_count = 3;

    // Button dimensions
    const uint8_t button_width = 40;
    const uint8_t button_height = 17;
    const uint8_t button_y = 44;
    const uint8_t button_spacing = 1;
    const uint8_t button_radius = 3;

    const uint8_t start_x = 3;

    // Draw all save options
    oled.setFont(u8g2_font_5x7_tf);

    for (uint8_t i = 0; i < option_count; i++) {

        uint8_t button_x =
            start_x +
            i * (button_width + button_spacing);

        bool selected =
            i == static_cast<uint8_t>(selected_option);

        // Draw button background or frame
        oled.setDrawColor(1);

        if (selected) {

            oled.drawRBox(
                button_x,
                button_y,
                button_width,
                button_height,
                button_radius
            );

        } else {

            oled.drawRFrame(
                button_x,
                button_y,
                button_width,
                button_height,
                button_radius
            );
        }

        // Center label inside button
        uint8_t text_width =
            oled.getStrWidth(labels[i]);

        int16_t text_x =
            button_x +
            (button_width - text_width) / 2;

        uint8_t text_y = button_y + 11;

        /// Invert text for selected option
        oled.setDrawColor(selected ? 0 : 1);

        // Draw confirmation checkmark or button label
        if (confirmed && selected && labels[i] != "NO") {

            draw_checkmark(
                button_x + button_width / 2,
                button_y + button_height / 2
            );

        } else {

            oled.drawStr(
                text_x,
                text_y,
                labels[i]
            );
        }
    }

    // Restore default drawing color
    oled.setDrawColor(1);
}


// Draw animated coffee beans
void display_manager::draw_coffee_animation(uint32_t now,uint32_t grind_duration_ms) {
    // Calculate elapsed grinding time
    uint32_t elapsed = now - animation_start_time;

    // Prevent division by zero
    if (grind_duration_ms == 0) {
        return;
    }

    // Calculate current frame based on grinding progress
    uint32_t frame =
        (elapsed * COFFEE_ANIMATION_FRAMES) /
        grind_duration_ms;

    // Clamp to the last frame
    if (frame >= COFFEE_ANIMATION_FRAMES) {
        frame = COFFEE_ANIMATION_FRAMES - 1;
    }

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

void display_manager::draw_coffee_standard_animation(
    uint32_t now
) {
    // Calculate elapsed animation time
    uint32_t elapsed = now - animation_start_time;

    // Calculate frame at constant speed
    uint32_t frame =
        elapsed / COFFEE_STANDARD_INTERVAL_MS;

    // Stop animation when finished
    if (frame >= COFFEE_STANDARD_ANIMATION_FRAMES) {
        return;
    }

    // Select bitmap
    const uint8_t* bitmap =
        coffee_standard_animation_frames[frame];

    // Draw animation
    oled.drawXBMP(
        54,
        -2,
        COFFEE_ICON_WIDTH,
        COFFEE_ICON_HEIGHT,
        bitmap
    );
}


void display_manager::draw_settings_confirmation() {
    const bool confirmed = settings->get_mode() == settings_mode::CONFIRMED;
    const bool save_selected = settings->get_confirmation_choice() == confirmation_choice::SAVE;
    constexpr int DISPLAY_WIDTH = 128;

    oled.setDrawColor(1);

    // Draw title
    oled.setFont(u8g2_font_helvR10_tf);
    const char* title = "Save changes?";

    if (confirmed) {

        if (save_selected) {
            title = "Settings saved";
        }
        else{
            title = "Changes discarded";
        }
    }

    oled.drawStr(
        (DISPLAY_WIDTH - oled.getStrWidth(title)) / 2,
        18,
        title
    );

    // Button dimensions
    constexpr int BUTTON_Y = 30;
    constexpr int DISCARD_BUTTON_WIDTH = 72;
    constexpr int SAVE_BUTTON_WIDTH = 54;
    constexpr int BUTTON_HEIGHT = 24;

    constexpr int SAVE_X = 0;
    constexpr int DISCARD_X = 56;

    oled.setFont(u8g2_font_helvB10_tf);

    // SAVE button
    if (save_selected) {
        oled.setDrawColor(1);

        oled.drawBox(
            SAVE_X,
            BUTTON_Y,
            SAVE_BUTTON_WIDTH,
            BUTTON_HEIGHT
        );

        oled.setDrawColor(0);

    } else {
        oled.setDrawColor(1);

        oled.drawFrame(
            SAVE_X,
            BUTTON_Y,
            SAVE_BUTTON_WIDTH,
            BUTTON_HEIGHT
        );
    }

    if (confirmed && save_selected) {
        draw_checkmark(
            SAVE_X + SAVE_BUTTON_WIDTH / 2,
            BUTTON_Y + BUTTON_HEIGHT / 2
        );

    } else {
        oled.drawStr(
            SAVE_X +
            (SAVE_BUTTON_WIDTH - oled.getStrWidth("SAVE")) / 2,
            47,
            "SAVE"
        );
    }

    // DISCARD button
    oled.setDrawColor(1);

    if (!save_selected) {
        oled.drawBox(
            DISCARD_X,
            BUTTON_Y,
            DISCARD_BUTTON_WIDTH,
            BUTTON_HEIGHT
        );

        oled.setDrawColor(0);

    } else {
        oled.drawFrame(
            DISCARD_X,
            BUTTON_Y,
            DISCARD_BUTTON_WIDTH,
            BUTTON_HEIGHT
        );
    }

    // Draw DISCARD label
    oled.drawStr(
        DISCARD_X +
        (DISCARD_BUTTON_WIDTH -
        oled.getStrWidth("DISCARD")) / 2,
        47,
        "DISCARD"
    );

    // Restore default drawing color
    oled.setDrawColor(1);
}

void display_manager::draw_checkmark(
    int center_x,
    int center_y) {

    // Draw thick checkmark
    for (int offset = -1; offset <= 2; offset++) {

        oled.drawLine(
            center_x - 7,
            center_y + offset,
            center_x - 2,
            center_y + 5 + offset
        );

        oled.drawLine(
            center_x - 2,
            center_y + 5 + offset,
            center_x + 8,
            center_y - 5 + offset
        );
    }
}

void display_manager::apply_brightness() {

    // Read brightness setting
    uint8_t brightness =
        settings->get_display_brightness();

    // Convert percentage to OLED contrast
    uint8_t contrast =
        (static_cast<uint32_t>(brightness) * 255 + 50)
        / 100;

    // Apply brightness to display
    oled.setContrast(contrast);

    // Remember applied brightness
    last_display_brightness = brightness;
}

void display_manager::draw_animated_checkmark(
    int center_x,
    int center_y,
    uint32_t elapsed
) {
    // Checkmark coordinates
    const int x1 = center_x - 7;
    const int y1 = center_y;

    const int x2 = center_x - 2;
    const int y2 = center_y + 5;

    const int x3 = center_x + 8;
    const int y3 = center_y - 5;

    // Calculate progress of first stroke
    uint32_t first_elapsed = min(
        elapsed,
        CHECKMARK_FIRST_STROKE_MS
    );

    int first_x = x1 +
        ((x2 - x1) * (int32_t)first_elapsed) /
        (int32_t)CHECKMARK_FIRST_STROKE_MS;

    int first_y = y1 +
        ((y2 - y1) * (int32_t)first_elapsed) /
        (int32_t)CHECKMARK_FIRST_STROKE_MS;

    // Draw first stroke
    if (elapsed > 0) {

        for (int offset = -1; offset <= 2; offset++) {

            oled.drawLine(
                x1,
                y1 + offset,
                first_x,
                first_y + offset
            );
        }
    }

    // Draw second stroke after first stroke is finished
    if (elapsed >= CHECKMARK_FIRST_STROKE_MS) {

        uint32_t second_elapsed = min(
            elapsed - CHECKMARK_FIRST_STROKE_MS,
            CHECKMARK_SECOND_STROKE_MS
        );

        int second_x = x2 +
            ((x3 - x2) * (int32_t)second_elapsed) /
            (int32_t)CHECKMARK_SECOND_STROKE_MS;

        int second_y = y2 +
            ((y3 - y2) * (int32_t)second_elapsed) /
            (int32_t)CHECKMARK_SECOND_STROKE_MS;

        for (int offset = -1; offset <= 2; offset++) {

            oled.drawLine(
                x2,
                y2 + offset,
                second_x,
                second_y + offset
            );
        }
    }
}

void display_manager::draw_grinding_finished(grind_mode mode, uint32_t now) {
    // Calculate elapsed animation time
    uint32_t elapsed = now - animation_start_time;

    constexpr int DISPLAY_WIDTH = 128;

    // Draw grinding mode
    oled.setFont(u8g2_font_helvR08_tf);

    const char* mode_text =
        (mode == grind_mode::SINGLE)
        ? "SINGLE"
        : "DOUBLE";

    uint8_t mode_width = oled.getStrWidth(mode_text);
    uint8_t x_pos_grind_type = 42;

    // Adjust position if animation is enabled
    if (settings->get_animation_mode() == coffee_animation_mode::PROGRESS) {
        x_pos_grind_type = 83;
        const uint8_t* bitmap = coffee_frame_29;

        // Draw last frame
        oled.drawXBMP(
            54,
            -2,
            COFFEE_ICON_WIDTH,
            COFFEE_ICON_HEIGHT,
            bitmap
        );
    }
    else if (settings->get_animation_mode() == coffee_animation_mode::STANDARD) {
        x_pos_grind_type = 83;    
    }

    oled.drawStr(
        x_pos_grind_type,
        12,
        mode_text
    );

    // Draw animated DONE text
    oled.setFont(u8g2_font_helvR12_tf);

    const char* done_text = "Done";

    // Calculate width of complete text
    uint8_t text_width =
        oled.getStrWidth(done_text);

    // Checkmark and text layout
    constexpr uint8_t checkmark_width = 16;
    constexpr uint8_t spacing = 8;

    uint8_t total_width =
        checkmark_width + spacing + text_width;

    int start_x =
        (DISPLAY_WIDTH - total_width) / 2;

    // Draw animated checkmark
    draw_animated_checkmark(
        start_x + 7,
        33,
        elapsed
    );

    // Calculate visible characters
    uint8_t visible_chars = min(
        (uint32_t)4,
        1 + elapsed / DONE_LETTER_INTERVAL_MS
    );

    // Prepare animated text
    char text_buffer[5] = {};

    for (uint8_t i = 0; i < visible_chars; i++) {
        text_buffer[i] = done_text[i];
    }

    // Draw text at fixed position
    oled.drawStr(
        start_x + checkmark_width + spacing,
        43,
        text_buffer
    );

    // Draw fully filled progress bar
    constexpr uint8_t bar_x = 5;
    constexpr uint8_t bar_y = 51;
    constexpr uint8_t bar_width = 118;
    constexpr uint8_t bar_height = 11;
    constexpr uint8_t bar_radius = 4;

    oled.drawRFrame(
        bar_x,
        bar_y,
        bar_width,
        bar_height,
        bar_radius
    );

    oled.drawRBox(
        bar_x + 2,
        bar_y + 2,
        bar_width - 4,
        bar_height - 4,
        2
    );
}