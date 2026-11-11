#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <Arduino.h>
#include "types.h"

class input_manager {

public:
    void init();
    void update(uint32_t now);
    event get_event();

private:

    // Button debounce
    static constexpr uint32_t DEBOUNCE_MS = 20;

    struct button {
        bool raw_state = HIGH;
        bool stable_state = HIGH;
        uint32_t last_change = 0;
    };

    button single_button;
    button double_button;
    button encoder_button;
    button manual_input;

    // Encoder
    uint8_t encoder_state = 0;
    int8_t encoder_steps = 0;

    // Event queue
    static constexpr uint8_t QUEUE_SIZE = 16;

    event event_queue[QUEUE_SIZE];
    uint8_t queue_head = 0;
    uint8_t queue_tail = 0;

    // Internal functions
    bool update_button(button &btn, uint8_t pin, uint32_t now);
    void update_encoder();
    void update_encoder_button(uint32_t now);
    void update_manual_input(uint32_t now);
    void push_event(event new_event);
};

#endif