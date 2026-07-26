#ifndef AUDIO_H
#define AUDIO_H

#include "pattern.h"

void audio_init(void);
void audio_stop_all(void);
void audio_trigger(uint8_t track, const StepData *step);
void audio_update(void);

#endif
