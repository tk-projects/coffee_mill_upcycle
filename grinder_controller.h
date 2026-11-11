#ifndef GRINDER_CONTROLLER_H
#define GRINDER_CONTROLLER_H

#include <Arduino.h>
#include "types.h"
#include "config.h"
#include "settings_manager.h"

class grinder_controller {

public:
    void init(const settings_manager &settings);

    void update(state current_state,
                grind_mode current_mode,
                uint32_t now);

    event get_event();

    bool is_grinding() const;

    uint32_t get_elapsed_time(uint32_t now) const;
    uint32_t get_remaining_time(uint32_t now) const;

private:
    const settings_manager* settings = nullptr;

    // Grinding status
    bool grinding_active = false;

    uint32_t grind_start_time = 0;
    uint32_t grind_duration = 0;

    // State monitoring
    state previous_state = state::STANDBY;

    // Pending event
    event pending_event = event::NONE;

    // Internal functions
    void start_grinding(grind_mode mode, uint32_t now);
    void stop_grinding();
};

#endif