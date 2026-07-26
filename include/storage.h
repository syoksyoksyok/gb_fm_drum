#ifndef STORAGE_H
#define STORAGE_H

#include <stdint.h>
#include "pattern.h"

#define SAVE_VERSION 1

void storage_init(void);
uint8_t storage_load_pattern(uint8_t index, PatternData *out);
void storage_save_pattern(uint8_t index, const PatternData *p);
uint8_t storage_last_pattern(void);
void storage_set_last_pattern(uint8_t index);
uint8_t storage_had_error(void);

#endif
