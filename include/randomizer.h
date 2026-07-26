#ifndef RANDOMIZER_H
#define RANDOMIZER_H

#include <stdint.h>
#include "pattern.h"

void rng_seed(uint16_t seed);
uint8_t rng_u8(void);
uint16_t rng_u16(void);
uint8_t rng_range(uint8_t max_inclusive);
void rng_mix(uint8_t value);
void randomizer_musical(PatternData *p);
void randomizer_full(PatternData *p);

#endif
