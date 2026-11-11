#ifndef TYPES_H
#define TYPES_H

enum class state {
    STANDBY,
    AUTO_GRINDING,
    MANUAL_GRINDING,
    SAVE_PROMPT,
    SETTINGS
};

enum class event {
    NONE,
    SINGLE_PRESSED,
    DOUBLE_PRESSED,
    ENCODER_LEFT,
    ENCODER_RIGHT,
    ENCODER_PRESSED,
    MANUAL_GRIND_STARTED,
    MANUAL_GRIND_STOPPED,
    GRIND_TIMER_FINISHED
};

enum class grind_mode {
    SINGLE,
    DOUBLE
};

enum class language {
    ENGLISH,
    GERMAN
};


// Convert state to string for debugging
inline const char* state_to_string(state value) {

    switch (value) {
        case state::STANDBY:         return "STANDBY";
        case state::AUTO_GRINDING:   return "AUTO_GRINDING";
        case state::MANUAL_GRINDING: return "MANUAL_GRINDING";
        case state::SAVE_PROMPT:     return "SAVE_PROMPT";
        case state::SETTINGS:        return "SETTINGS";
        default:                     return "UNKNOWN";
    }
}


// Convert event to string for debugging
inline const char* event_to_string(event value) {

    switch (value) {

        case event::NONE:                 return "NONE";
        case event::SINGLE_PRESSED:       return "SINGLE_PRESSED";
        case event::DOUBLE_PRESSED:       return "DOUBLE_PRESSED";
        case event::ENCODER_LEFT:         return "ENCODER_LEFT";
        case event::ENCODER_RIGHT:        return "ENCODER_RIGHT";
        case event::ENCODER_PRESSED:      return "ENCODER_PRESSED";
        case event::MANUAL_GRIND_STARTED: return "MANUAL_GRIND_STARTED";
        case event::MANUAL_GRIND_STOPPED: return "MANUAL_GRIND_STOPPED";
        case event::GRIND_TIMER_FINISHED: return "GRIND_TIMER_FINISHED";

        default: return "UNKNOWN";
    }
}

#endif