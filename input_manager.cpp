#include "input_manager.h"
#include "config.h"


// Initialize all hardware inputs
void input_manager::init() {

    // Pushbuttons
    pinMode(PIN_BUTTON_SINGLE, INPUT_PULLUP);
    pinMode(PIN_BUTTON_DOUBLE, INPUT_PULLUP);

    // Rotary encoder
    pinMode(PIN_ENCODER_CLK, INPUT_PULLUP);
    pinMode(PIN_ENCODER_DT, INPUT_PULLUP);
    pinMode(PIN_ENCODER_SW, INPUT_PULLUP);

    // Original START signal
    //pinMode(PIN_MANUAL_GRIND, INPUT);   // (This input has already a pullup on the orignial pcb)
    pinMode(PIN_MANUAL_GRIND, INPUT_PULLUP);  // For testing only

    uint32_t now = millis();

    // Initialize button states
    single_button.raw_state = digitalRead(PIN_BUTTON_SINGLE);
    single_button.stable_state = single_button.raw_state;
    single_button.last_change = now;

    double_button.raw_state = digitalRead(PIN_BUTTON_DOUBLE);
    double_button.stable_state = double_button.raw_state;
    double_button.last_change = now;

    encoder_button.raw_state = digitalRead(PIN_ENCODER_SW);
    encoder_button.stable_state = encoder_button.raw_state;
    encoder_button.last_change = now;

    manual_input.raw_state = digitalRead(PIN_MANUAL_GRIND);
    manual_input.stable_state = manual_input.raw_state;
    manual_input.last_change = now;

    // Initialize encoder state
    encoder_state =
        (digitalRead(PIN_ENCODER_CLK) << 1) |
         digitalRead(PIN_ENCODER_DT);

    encoder_steps = 0;

    queue_head = 0;
    queue_tail = 0;
}

// Update all hardware inputs
void input_manager::update(uint32_t now) {

    // Single button
    if (update_button(single_button, PIN_BUTTON_SINGLE, now)) {

        if (single_button.stable_state == LOW) {
            push_event(event::SINGLE_PRESSED);
        }
    }

    // Double button
    if (update_button(double_button, PIN_BUTTON_DOUBLE, now)) {

        if (double_button.stable_state == LOW) {
            push_event(event::DOUBLE_PRESSED);
        }
    }

    // Rotary encoder
    update_encoder();
    update_encoder_button(now);

    // Manual grinding detection
    update_manual_input(now);
}

// Debounce function for all buttons
bool input_manager::update_button(button &btn, uint8_t pin, uint32_t now) {

    bool new_state = digitalRead(pin);

    // Detect raw signal change
    if (new_state != btn.raw_state) {
        btn.raw_state = new_state;
        btn.last_change = now;
    }

    // Check debounce time
    if ((now - btn.last_change) >= DEBOUNCE_MS) {

        if (btn.stable_state != btn.raw_state) {

            btn.stable_state = btn.raw_state;

            return true;
        }
    }

    return false;
}

void input_manager::update_encoder_button(uint32_t now) {

    if (update_button(encoder_button, PIN_ENCODER_SW, now)) {

        if (encoder_button.stable_state == LOW) {
            push_event(event::ENCODER_PRESSED);
        }
    }
}

// Read rotary encoder
void input_manager::update_encoder() {

    // Read both encoder signals and combine them into a 2 bit signal
    uint8_t new_state =
        (digitalRead(PIN_ENCODER_CLK) << 1) |
         digitalRead(PIN_ENCODER_DT);

    // No change
    if (new_state == encoder_state) {
        return;
    }

    // Lookup table for encoder transitions
    static const int8_t transitions[16] = {
         0, -1,  1,  0,
         1,  0,  0, -1,
        -1,  0,  0,  1,
         0,  1, -1,  0
    };

    // Previous and current state combined (4 bit)
    uint8_t index = (encoder_state << 2) | new_state;

    // Accumulate encoder transitions
    encoder_steps += transitions[index];  // Read step from table

    // Store current state
    encoder_state = new_state;

    // Generate event after four transitions
    if (encoder_steps >= 4) {

        push_event(event::ENCODER_RIGHT);
        encoder_steps = 0;

    } else if (encoder_steps <= -4) {

        push_event(event::ENCODER_LEFT);
        encoder_steps = 0;
    }
}


// Detect manual grinding via original START signal
void input_manager::update_manual_input(uint32_t now) {

    // Check for signal changes
    if (update_button(manual_input, PIN_MANUAL_GRIND, now)) {

        if (manual_input.stable_state == LOW) {

            // START signal activated
            push_event(event::MANUAL_GRIND_STARTED);

        } else {

            // START signal deactivated
            push_event(event::MANUAL_GRIND_STOPPED);
        }
    }
}

// Add event to queue
void input_manager::push_event(event new_event) {

    // Calculate next write position
    uint8_t next_head = (queue_head + 1) % QUEUE_SIZE;

    // Check if queue is full
    if (next_head == queue_tail) {
        return;
    }

    // Store event
    event_queue[queue_head] = new_event;

    // Update write position
    queue_head = next_head;
}

// Get oldest event from queue
event input_manager::get_event() {

    // Check if queue is empty
    if (queue_head == queue_tail) {
        return event::NONE;
    }

    // Read oldest event
    event new_event = event_queue[queue_tail];

    // Update read position
    queue_tail = (queue_tail + 1) % QUEUE_SIZE;

    return new_event;
}