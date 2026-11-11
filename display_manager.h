#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <U8g2lib.h>
#include "types.h"
#include "config.h"
#include "settings_manager.h"
#include "grinder_controller.h"

class display_manager {

public:
    void init(const settings_manager &settings, const grinder_controller &grinder);
    void update(state current_state, grind_mode current_mode, uint32_t now);

private:
    // Display driver
    U8G2_SH1106_128X64_NONAME_F_HW_I2C oled{
        U8G2_R2,
        U8X8_PIN_NONE
    };

    // Display state
    state displayed_state = state::STANDBY;
    bool display_dirty = true;
    uint32_t last_display_update = 0;

    // Coffee animation
    uint32_t animation_start_time = 0;

    // Internal functions
    void draw_standby();
    void draw_auto_grinding(grind_mode mode, uint32_t now);
    void draw_manual_grinding(uint32_t now);
    void draw_save_prompt();
    void draw_settings();
    void draw_coffee_animation(uint32_t now);

    // Instances 
    const settings_manager* settings = nullptr;
    const grinder_controller* grinder = nullptr;
};

#endif