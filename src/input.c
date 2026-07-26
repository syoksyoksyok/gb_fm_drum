#include <gb/gb.h>
#include "input.h"

InputState input;
static uint8_t hold_key = 0;
static uint8_t hold_frames = 0;

void input_update(void) {
    uint8_t dirs;
    input.prev = input.now;
    input.now = joypad();
    input.pressed = input.now & (uint8_t)~input.prev;
    input.released = input.prev & (uint8_t)~input.now;
    input.repeat = input.pressed;
    dirs = input.now & (J_UP | J_DOWN | J_LEFT | J_RIGHT);
    if (dirs && dirs == hold_key) {
        if (hold_frames < 24) hold_frames++;
        else {
            input.repeat |= dirs;
            hold_frames = 18;
        }
    } else {
        hold_key = dirs;
        hold_frames = 0;
    }
}
