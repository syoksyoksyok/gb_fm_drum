#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>

typedef struct {
    uint8_t now;
    uint8_t prev;
    uint8_t pressed;
    uint8_t released;
    uint8_t repeat;
} InputState;

extern InputState input;

void input_update(void);

#endif
