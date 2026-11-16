#ifndef APP_CONTROLLER_H
#define APP_CONTROLLER_H

#include <Arduino.h>
#include "types.h"
#include "config.h"
#include "input_manager.h"
#include "display_manager.h"
#include "grinder_controller.h"
#include "settings_manager.h"

class app_controller {

public:
    void init();
    void update();

    uint32_t get_manual_grind_elapsed_time(uint32_t now) const;
    uint32_t get_manual_grind_duration() const;
    save_option_manuel_grind get_selected_save_option() const;

private:
    // Modules
    input_manager inputs;
    display_manager display;
    grinder_controller grinder;
    settings_manager settings;

    // State machine
    state current_state = state::STANDBY;

    // Grind mode
    grind_mode current_grind_mode = grind_mode::SINGLE;
    uint32_t grinding_finished_start_time = 0;

    // Manual grinding
    uint32_t manual_grind_start_ms = 0;
    uint32_t manual_grind_duration_ms = 0;
    bool save_prompt_confirmed = false;

    // Save prompt
    save_option_manuel_grind selected_save_option = save_option_manuel_grind::NO;
    uint32_t save_prompt_last_activity_ms = 0;

    // Settings timeout
    uint32_t last_settings_activity = 0;

    // Internal functions
    void handle_event(event new_event, uint32_t now);
    void handle_state(uint32_t now);
    void change_state(state new_state, uint32_t now);
};

#endif