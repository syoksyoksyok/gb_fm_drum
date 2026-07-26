#ifndef SEQUENCER_H
#define SEQUENCER_H

#include <stdint.h>
#include "pattern.h"

extern uint8_t seq_playing;
extern uint8_t seq_pos[NUM_TRACKS];

void sequencer_init(void);
void sequencer_start(void);
void sequencer_stop(void);
void sequencer_update(void);
void sequencer_reset_positions(void);

#endif
