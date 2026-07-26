#include <gb/gb.h>
#include <stdint.h>
#include "storage.h"

#define SRAM_BASE ((volatile uint8_t *)0xA000)
#define SAVE_MAGIC0 'G'
#define SAVE_MAGIC1 'F'
#define SAVE_MAGIC2 'M'
#define SAVE_MAGIC3 'D'
#define HEADER_SIZE 16
#define SLOT_SIZE (sizeof(PatternData) + 2u)

static uint8_t last_pat = 0;
static uint8_t had_error = 0;

static uint16_t slot_offset(uint8_t index) {
    return HEADER_SIZE + ((uint16_t)index * SLOT_SIZE);
}

static void sram_write(uint16_t off, uint8_t value) {
    SRAM_BASE[off] = value;
}

static uint8_t sram_read(uint16_t off) {
    return SRAM_BASE[off];
}

static void save_header(void) {
    sram_write(0, SAVE_MAGIC0);
    sram_write(1, SAVE_MAGIC1);
    sram_write(2, SAVE_MAGIC2);
    sram_write(3, SAVE_MAGIC3);
    sram_write(4, SAVE_VERSION);
    sram_write(5, last_pat);
    sram_write(6, 0x55);
    sram_write(7, (uint8_t)(SAVE_MAGIC0 ^ SAVE_MAGIC1 ^ SAVE_MAGIC2 ^ SAVE_MAGIC3 ^ SAVE_VERSION ^ last_pat ^ 0x55));
}

static uint8_t valid_header(void) {
    uint8_t c = (uint8_t)(sram_read(0) ^ sram_read(1) ^ sram_read(2) ^ sram_read(3) ^ sram_read(4) ^ sram_read(5) ^ sram_read(6));
    return sram_read(0) == SAVE_MAGIC0 && sram_read(1) == SAVE_MAGIC1 &&
           sram_read(2) == SAVE_MAGIC2 && sram_read(3) == SAVE_MAGIC3 &&
           sram_read(4) == SAVE_VERSION && sram_read(7) == c;
}

void storage_save_pattern(uint8_t index, const PatternData *p) {
    uint16_t off = slot_offset(index);
    const uint8_t *b = (const uint8_t *)p;
    uint16_t i;
    uint8_t c = pattern_checksum(p);
    ENABLE_RAM;
    for (i = 0; i < sizeof(PatternData); ++i) sram_write(off + i, b[i]);
    sram_write(off + sizeof(PatternData), c);
    sram_write(off + sizeof(PatternData) + 1u, (uint8_t)~c);
    last_pat = index;
    save_header();
    DISABLE_RAM;
}

uint8_t storage_load_pattern(uint8_t index, PatternData *out) {
    uint16_t off = slot_offset(index);
    uint8_t *b = (uint8_t *)out;
    uint16_t i;
    uint8_t c, nc;
    ENABLE_RAM;
    for (i = 0; i < sizeof(PatternData); ++i) b[i] = sram_read(off + i);
    c = sram_read(off + sizeof(PatternData));
    nc = sram_read(off + sizeof(PatternData) + 1u);
    DISABLE_RAM;
    if (c != (uint8_t)~nc || c != pattern_checksum(out)) {
        if (index == 0) pattern_init_demo(out); else pattern_init_empty(out);
        had_error = 1;
        storage_save_pattern(index, out);
        return 0;
    }
    pattern_clamp(out);
    return 1;
}

void storage_init(void) {
    uint8_t i;
    ENABLE_RAM;
    if (!valid_header()) {
        PatternData p;
        had_error = 1;
        last_pat = 0;
        save_header();
        DISABLE_RAM;
        for (i = 0; i < NUM_PATTERNS; ++i) {
            if (i == 0) pattern_init_demo(&p); else pattern_init_empty(&p);
            storage_save_pattern(i, &p);
        }
        return;
    }
    last_pat = sram_read(5);
    if (last_pat >= NUM_PATTERNS) last_pat = 0;
    DISABLE_RAM;
}

uint8_t storage_last_pattern(void) {
    return last_pat;
}

void storage_set_last_pattern(uint8_t index) {
    last_pat = index;
    ENABLE_RAM;
    save_header();
    DISABLE_RAM;
}

uint8_t storage_had_error(void) {
    return had_error;
}
