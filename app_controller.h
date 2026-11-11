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

private:
    // State machine
    state current_state = state::STANDBY;

    // Grind mode
    grind_mode current_grind_mode = grind_mode::SINGLE;

    // Modules
    input_manager inputs;
    display_manager display;
    grinder_controller grinder;
    settings_manager settings;

    // Settings timeout
    uint32_t last_settings_activity = 0;

    // Internal functions
    void handle_state(uint32_t now);
    void handle_event(event new_event);
    void change_state(state new_state);
};

#endif