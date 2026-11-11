#include "grinder_controller.h"

// Controller init
void grinder_controller::init(const settings_manager &settings) {

    // Store settings reference
    this->settings = &settings;

    grinding_active = false;

    grind_start_time = 0;
    grind_duration = 0;

    previous_state = state::STANDBY;
    pending_event = event::NONE;

    // Initialize motor output (default OFF)
    digitalWrite(PIN_MOTOR, LOW);
    pinMode(PIN_MOTOR, OUTPUT);


    // Initialize built-in LED (active LOW)
    if (ENABLE_DEBUG_MODE) {
        digitalWrite(LED_BUILTIN, HIGH);
        pinMode(LED_BUILTIN, OUTPUT);
    }

     Serial.println("Grinder controller initialized");
}

// Cyclicl control
void grinder_controller::update(state current_state, grind_mode current_mode, uint32_t now) {

    // Detect state transition
    if (current_state != previous_state) {

        // Stop grinding when leaving AUTO_GRINDING
        if (previous_state == state::AUTO_GRINDING) {
            stop_grinding();
        }

        // Start grinding when entering AUTO_GRINDING
        if (current_state == state::AUTO_GRINDING) {
            start_grinding(current_mode, now);
        }

        // Store current state
        previous_state = current_state;
    }

    // Monitor automatic grinding
    if (current_state == state::AUTO_GRINDING && grinding_active) {
        if (now - grind_start_time >= grind_duration) {
            stop_grinding();

            pending_event = event::GRIND_TIMER_FINISHED;
        }
    }
}


void grinder_controller::start_grinding(grind_mode mode, uint32_t now) {

    // Select grinding duration
    if (mode == grind_mode::SINGLE) {
        grind_duration = settings->get_single_grind_time();
    }
    else {
        grind_duration = settings->get_double_grind_time();
    }

    // Initialize timer
    grind_start_time = now;
    grinding_active = true;

    // Enable motor output
    if (ENABLE_MOTOR_OUTPUT) {
        digitalWrite(PIN_MOTOR, HIGH);
    }

    // Indicate active grinding
    if (ENABLE_DEBUG_MODE) {
        digitalWrite(LED_BUILTIN, LOW);
    }

    Serial.print("Grinding started: ");
    Serial.print(grind_duration);
    Serial.println(" ms");
}

void grinder_controller::stop_grinding() {
    // Always disable motor output
    digitalWrite(PIN_MOTOR, LOW);

    if (!grinding_active) {
        return;
    }

    grinding_active = false;

    // Turn off built-in LED
    if (ENABLE_DEBUG_MODE) {
        digitalWrite(LED_BUILTIN, HIGH);
    }

    Serial.println("Grinding stopped");
}

event grinder_controller::get_event() {
    event new_event = pending_event;
    pending_event = event::NONE;  // Reset event
    return new_event;
}

bool grinder_controller::is_grinding() const {
    return grinding_active;
}

uint32_t grinder_controller::get_elapsed_time(uint32_t now) const {

    if (!grinding_active) {
        return 0;
    }

    uint32_t elapsed = now - grind_start_time;

    return min(elapsed, grind_duration);
}


// Function used by display later in order to show remaining grinding duration
uint32_t grinder_controller::get_remaining_time(uint32_t now) const {

    if (!grinding_active) {
        return 0;
    }

    uint32_t elapsed = now - grind_start_time;

    if (elapsed >= grind_duration) {
        return 0;
    }

    return grind_duration - elapsed;
}