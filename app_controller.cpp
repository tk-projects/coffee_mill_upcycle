#include "app_controller.h"

void app_controller::init() {

    // Initialize serial interface
    Serial.begin(115200);
    Serial.println("Coffee grinder controller starting...");

    // Initialize modules
    settings.init();
    inputs.init();
    grinder.init(settings);
    display.init(settings, grinder);

    // Set initial state
    current_state = state::STANDBY;
}


// High level controller update function and hierarchy
void app_controller::update() {
    // Get time
    uint32_t now = millis();

    // 1. Update hardware inputs and get events from input manager
    inputs.update(now);

    // 2. Process events
    event new_event = inputs.get_event();  // Get oldest event from queue

    while (new_event != event::NONE) {
        handle_event(new_event);
        new_event = inputs.get_event();   // Get next event from queue until queue is empty
    }

    // process current state
    handle_state(now);

    // 4. Update grinder controller
    grinder.update(current_state, current_grind_mode, now);

    // 5. Process grinder events
    event grinder_event = grinder.get_event();

    if (grinder_event != event::NONE) {
        handle_event(grinder_event);
    }

    // 6. Update display
    display.update(current_state, current_grind_mode, now);
}


// Process current state implications
void app_controller::handle_state(uint32_t now) {

    switch (current_state) {

        case state::STANDBY:
            break;

        case state::AUTO_GRINDING:
            break;

        case state::MANUAL_GRINDING:
            break;

        case state::SAVE_PROMPT:
            break;

        case state::SETTINGS:

            // Timeout to leave SETTINGS and go back into STANDBY
            if (now - last_settings_activity >= SETTINGS_TIMEOUT_MS) {
                change_state(state::STANDBY);
            }
            break;
    }
}


// Process events:
void app_controller::handle_event(event new_event) {
    Serial.print("Event received: ");
    Serial.println(event_to_string(new_event));

    switch (current_state) {

        case state::STANDBY:
            // Transition into AUTO_GRINDING
            if (new_event == event::SINGLE_PRESSED) {

                current_grind_mode = grind_mode::SINGLE;
                change_state(state::AUTO_GRINDING);
            }

            else if (new_event == event::DOUBLE_PRESSED) {

                current_grind_mode = grind_mode::DOUBLE;
                change_state(state::AUTO_GRINDING);
            }

            // Transition into MANUAL_GRINDING
            else if (new_event == event::MANUAL_GRIND_STARTED) {

                change_state(state::MANUAL_GRINDING);
            }

            // Transition into SETTINGS
            else if (new_event == event::ENCODER_LEFT ||
                    new_event == event::ENCODER_RIGHT ||
                    new_event == event::ENCODER_PRESSED) {

                change_state(state::SETTINGS);
            }
            break;

        case state::AUTO_GRINDING:
            // If the grinder finishes or the single or double button is pressed again during auto grind, AUTO_GRINDING will be exited
            if (new_event == event::SINGLE_PRESSED ||
                new_event == event::DOUBLE_PRESSED ||
                new_event == event::GRIND_TIMER_FINISHED) {

                change_state(state::STANDBY);
            }
            break;

        case state::MANUAL_GRINDING:
            // Transistion into SETTINGS
            if (new_event == event::MANUAL_GRIND_STOPPED) {

                change_state(state::SAVE_PROMPT);
            }
            break;

        case state::SAVE_PROMPT:
            if (new_event == event::ENCODER_PRESSED) {
                change_state(state::STANDBY);
            }
            break;

        case state::SETTINGS:
            if (new_event == event::ENCODER_LEFT ||
                new_event == event::ENCODER_RIGHT ||
                new_event == event::ENCODER_PRESSED) {

                last_settings_activity = millis();
            }
            break;
    }
}


// Change into new state
void app_controller::change_state(state new_state) {
    if (current_state == new_state) {
        return;
    }

    // Save settings when leaving SETTINGS
    if (current_state == state::SETTINGS) {
        // Temporary FlashStorage test
        settings.save_settings();
    }

    state old_state = current_state;
    current_state = new_state;

    // Initialize settings timeout on state entry
    if (new_state == state::SETTINGS) {
        last_settings_activity = millis();
    }

    Serial.print("State changed: ");
    Serial.print(state_to_string(old_state));
    Serial.print(" --> ");
    Serial.println(state_to_string(new_state));
}