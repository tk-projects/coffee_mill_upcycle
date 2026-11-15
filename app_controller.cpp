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
    const uint32_t now = millis();

    // 1. Update hardware inputs and get events from input manager
    inputs.update(now);

    // 2. Process events
    event new_event = inputs.get_event();  // Get oldest event from queue

    while (new_event != event::NONE) {
        handle_event(new_event, now);
        new_event = inputs.get_event();   // Get next event from queue until queue is empty
    }

    // process current state
    handle_state(now);

    // 4. Update grinder controller
    grinder.update(current_state, current_grind_mode, now);

    // 5. Process grinder events
    event grinder_event = grinder.get_event();

    if (grinder_event != event::NONE) {
        handle_event(grinder_event, now);
    }

    // 6. Update display
    uint32_t manual_grind_time_ms = 0;

    if (current_state == state::MANUAL_GRINDING) {
        manual_grind_time_ms = get_manual_grind_elapsed_time(now);

    } else if (current_state == state::SAVE_PROMPT) {
        manual_grind_time_ms = get_manual_grind_duration();
    }

    display.update(current_state,current_grind_mode, now, manual_grind_time_ms, selected_save_option, save_prompt_confirmed);
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
            // Leave save prompt after confirmation feedback
            if (save_prompt_confirmed) {
                if (now - save_prompt_last_activity_ms >= SETTINGS_CONFIRMATION_DURATION_MS) {
                    change_state(state::STANDBY, now);
                }

                break;
            }

            // Return to standby after inactivity
            if (now - save_prompt_last_activity_ms >= SAVE_PROMPT_TIMEOUT_MS) {
                change_state(state::STANDBY, now);
            }

            break;

        case state::SETTINGS:
            // Leave settings after confirmation feedback
            if (settings.get_mode() == settings_mode::CONFIRMED) {

                if (now - last_settings_activity >=
                    SETTINGS_CONFIRMATION_DURATION_MS) {

                    change_state(state::STANDBY, now);
                }

                break;
            }


            // Timeout after inaction
            if (now - last_settings_activity >= SETTINGS_TIMEOUT_MS) {
                settings.enter_confirmation();
                change_state(state::STANDBY, now);
            }
            break;
    }
}


// Process events:
void app_controller::handle_event(event new_event, uint32_t now) {
    Serial.print("Event received: ");
    Serial.println(event_to_string(new_event));

    switch (current_state) {
        case state::STANDBY:
            // Transition into AUTO_GRINDING
            if (new_event == event::SINGLE_PRESSED) {

                current_grind_mode = grind_mode::SINGLE;
                change_state(state::AUTO_GRINDING, now);
            }

            else if (new_event == event::DOUBLE_PRESSED) {

                current_grind_mode = grind_mode::DOUBLE;
                change_state(state::AUTO_GRINDING, now);
            }

            // Transition into MANUAL_GRINDING
            else if (new_event == event::MANUAL_GRIND_STARTED) {

                change_state(state::MANUAL_GRINDING, now);
            }

            // Transition into SETTINGS
            else if (new_event == event::ENCODER_LEFT ||
                    new_event == event::ENCODER_RIGHT ||
                    new_event == event::ENCODER_PRESSED) {

                change_state(state::SETTINGS, now);
            }
            break;

        case state::AUTO_GRINDING:
            // If the grinder finishes or the single or double button is pressed again during auto grind, AUTO_GRINDING will be exited
            if (new_event == event::SINGLE_PRESSED ||
                new_event == event::DOUBLE_PRESSED ||
                new_event == event::GRIND_TIMER_FINISHED) {

                change_state(state::STANDBY, now);
            }
            break;

        case state::MANUAL_GRINDING:
            // Finish manual grinding and store duration
            if (new_event == event::MANUAL_GRIND_STOPPED) {
                manual_grind_duration_ms = now - manual_grind_start_ms;
                change_state(state::SAVE_PROMPT, now);
            }

            break;

        case state::SAVE_PROMPT:
            // Allow manual grinding at any time
            if (new_event == event::MANUAL_GRIND_STARTED) {
                change_state(state::MANUAL_GRINDING, now);
                break;
            }

            // Ignore other events during confirmation feedback
            if (save_prompt_confirmed) {
                break;
            }

            switch (new_event) {
                case event::ENCODER_LEFT:
                    // Select previous save option
                    selected_save_option = static_cast<save_option_manuel_grind>((static_cast<uint8_t>(selected_save_option) + 2) % 3);
                    save_prompt_last_activity_ms = now;
                    display.invalidate();
                    break;

                case event::ENCODER_RIGHT:
                    // Select next save option
                    selected_save_option =static_cast<save_option_manuel_grind>((static_cast<uint8_t>(selected_save_option) + 1) % 3);
                    save_prompt_last_activity_ms = now;
                    display.invalidate();
                    break;

                case event::ENCODER_PRESSED:
                    switch (selected_save_option) {
                        case save_option_manuel_grind::NO:
                            change_state(state::STANDBY, now);
                            break;

                        case save_option_manuel_grind::SINGLE:
                            settings.set_single_grind_time(manual_grind_duration_ms);
                            settings.save_settings();
                            break;

                        case save_option_manuel_grind::DOUBLE:
                            settings.set_double_grind_time(manual_grind_duration_ms);
                            settings.save_settings();
                            break;
                    }

                    // Show confirmation feedback after saving
                    if (selected_save_option != save_option_manuel_grind::NO) {
                        save_prompt_confirmed = true;
                        save_prompt_last_activity_ms = now;
                        display.invalidate();
                    }
                    break;

                // Restart manual grinding mode, when button is pressed again
                case event::MANUAL_GRIND_STARTED:
                    // Discard previous measurement and restart
                    change_state(state::MANUAL_GRINDING, now);
                    break;

                default:
                    break;
            }

            break;

        case state::SETTINGS:
            // Ignore encoder events during confirmation feedback
            if (settings.get_mode() == settings_mode::CONFIRMED) {
                break;
            }

            switch (new_event) {
                case event::ENCODER_LEFT:
                    last_settings_activity = now;

                    if (settings.get_mode() == settings_mode::NAVIGATE) {
                        settings.previous();

                    } else if (settings.get_mode() == settings_mode::EDIT) {
                        settings.decrease();

                    } else if (settings.get_mode() == settings_mode::CONFIRM) {
                        settings.toggle_confirmation_choice();
                    }

                    display.invalidate();
                    break;

                case event::ENCODER_RIGHT:

                    last_settings_activity = now;

                    if (settings.get_mode() == settings_mode::NAVIGATE) {
                        settings.next();

                    } else if (settings.get_mode() == settings_mode::EDIT) {
                        settings.increase();

                    } else if (settings.get_mode() == settings_mode::CONFIRM) {
                        settings.toggle_confirmation_choice();
                    }

                    display.invalidate();
                    break;

                case event::ENCODER_PRESSED:

                    last_settings_activity = now;

                    // Handle confirmation dialog
                    if (settings.get_mode() == settings_mode::CONFIRM) {
                        if (settings.get_confirmation_choice() == confirmation_choice::SAVE) {
                            settings.save_settings();

                        } else {
                            settings.discard_changes();
                        }
                        settings.enter_confirmed();
                        //change_state(state::STANDBY, now);
                    }

                    // Handle EXIT menu item
                    else if (settings.get_selected_item() == setting_item::EXIT) {
                        if (settings.has_changes()) {
                            settings.enter_confirmation();

                        } else {
                            change_state(state::STANDBY, now);
                        }
                    }

                    // Normal setting: enter or leave edit mode
                    else {
                        settings.toggle_edit();
                    }

                    display.invalidate();
                    break;

                case event::SINGLE_PRESSED:
                    settings.discard_changes();
                    change_state(state::STANDBY, now);
                    display.invalidate();
                    break;

                case event::DOUBLE_PRESSED:
                    settings.discard_changes();
                    change_state(state::STANDBY, now);
                    display.invalidate();
                    break;

                default:
                    break;
            }

            break;
    }
}


// Change into new state
void app_controller::change_state(state new_state, uint32_t now) {

    if (current_state == new_state) {
        return;
    }

    state old_state = current_state;
    current_state = new_state;

    // Initialize new state
    switch (new_state) {

        case state::SETTINGS:

            last_settings_activity = now;
            settings.begin_session();

            break;

        case state::MANUAL_GRINDING:

            manual_grind_start_ms = now;
            manual_grind_duration_ms = 0;

            break;

        case state::SAVE_PROMPT:
            selected_save_option = save_option_manuel_grind::NO;
            save_prompt_last_activity_ms = now;
            save_prompt_confirmed = false;

            break;

        default:
            break;
    }

    // Force display update
    display.invalidate();

    Serial.print("State changed: ");
    Serial.print(state_to_string(old_state));
    Serial.print(" --> ");
    Serial.println(state_to_string(new_state));
}


uint32_t app_controller::get_manual_grind_elapsed_time(uint32_t now) const {
    return now - manual_grind_start_ms;
}

uint32_t app_controller::get_manual_grind_duration() const {
    return manual_grind_duration_ms;
}

save_option_manuel_grind app_controller::get_selected_save_option() const {
    return selected_save_option;
}