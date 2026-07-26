#include "pattern.h"

PatternData current_pattern;
PatternData undo_pattern;
uint8_t undo_valid = 0;

static uint8_t clamp_u8(int16_t v, uint8_t lo, uint8_t hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return (uint8_t)v;
}

static int8_t clamp_i8(int16_t v, int8_t lo, int8_t hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return (int8_t)v;
}

void pattern_init_empty(PatternData *p) {
    uint8_t t, i;
    for (t = 0; t < NUM_TRACKS; ++t) {
        p->track[t].length = 16;
        p->track[t].direction = DIR_FORWARD;
        for (i = 0; i < NUM_STEPS; ++i) {
            StepData *s = &p->track[t].steps[i];
            s->trigger = 0;
            s->accent = 0;
            s->probability = 100;
            s->pitch_variation = 0;
            s->carrier_pitch = (t == TRACK_L) ? 28 : 55;
            s->mod_ratio = (t == TRACK_L) ? 2 : 6;
            s->fine_tune = 0;
            s->fm_depth = (t == TRACK_L) ? 10 : 18;
            s->pitch_env_amount = (t == TRACK_L) ? 18 : 8;
            s->pitch_env_decay = (t == TRACK_L) ? 14 : 6;
            s->pitch_env_direction = 0;
            s->amp_attack = 0;
            s->amp_decay = (t == TRACK_L) ? 12 : 6;
        }
    }
    p->bpm = 155;
    p->random_strength = 50;
}

void pattern_init_demo(PatternData *p) {
    uint8_t i;
    pattern_init_empty(p);
    for (i = 0; i < NUM_STEPS; ++i) {
        StepData *l = &p->track[TRACK_L].steps[i];
        StepData *r = &p->track[TRACK_R].steps[i];
        if ((i == 0) || (i == 4) || (i == 8) || (i == 12)) {
            l->trigger = 1;
            l->accent = (i == 0) ? 15 : 10;
            l->carrier_pitch = 24 + (i >> 2);
            l->fm_depth = 12;
            l->pitch_env_amount = 22;
            l->pitch_env_decay = 10;
            l->amp_decay = 13;
        }
        if ((i == 2) || (i == 6) || (i == 10) || (i == 14)) {
            r->trigger = 1;
            r->accent = (i == 6) ? 12 : 8;
            r->carrier_pitch = 58 + (i & 3);
            r->mod_ratio = 8;
            r->fm_depth = 24;
            r->pitch_env_amount = 6;
            r->pitch_env_decay = 5;
            r->amp_decay = 5;
        }
    }
    p->track[TRACK_R].length = 16;
    p->track[TRACK_R].direction = DIR_FORWARD;
}

void pattern_clamp(PatternData *p) {
    uint8_t t, i;
    p->bpm = (p->bpm < BPM_MIN) ? BPM_MIN : ((p->bpm > BPM_MAX) ? BPM_MAX : p->bpm);
    p->random_strength = clamp_u8(p->random_strength, 0, 100);
    for (t = 0; t < NUM_TRACKS; ++t) {
        if (p->track[t].length < 1) p->track[t].length = 1;
        if (p->track[t].length > 16) p->track[t].length = 16;
        if (p->track[t].direction > DIR_RANDOM) p->track[t].direction = DIR_FORWARD;
        for (i = 0; i < NUM_STEPS; ++i) {
            StepData *s = &p->track[t].steps[i];
            s->trigger = s->trigger ? 1 : 0;
            s->accent = clamp_u8(s->accent, 0, 15);
            s->probability = clamp_u8(s->probability, 0, 100);
            s->pitch_variation = clamp_u8(s->pitch_variation, 0, 15);
            s->carrier_pitch = clamp_u8(s->carrier_pitch, 0, 95);
            s->mod_ratio = clamp_u8(s->mod_ratio, 1, 16);
            s->fine_tune = clamp_i8(s->fine_tune, -16, 15);
            s->fm_depth = clamp_u8(s->fm_depth, 0, 31);
            s->pitch_env_amount = clamp_u8(s->pitch_env_amount, 0, 31);
            s->pitch_env_decay = clamp_u8(s->pitch_env_decay, 0, 31);
            s->pitch_env_direction = s->pitch_env_direction ? 1 : 0;
            s->amp_attack = clamp_u8(s->amp_attack, 0, 15);
            s->amp_decay = clamp_u8(s->amp_decay, 0, 31);
        }
    }
}

uint8_t pattern_checksum(const PatternData *p) {
    const uint8_t *b = (const uint8_t *)p;
    uint16_t i;
    uint8_t c = 0x5a;
    for (i = 0; i < sizeof(PatternData); ++i) c = (uint8_t)((c << 1) | (c >> 7)) ^ b[i];
    return c;
}

const char *pattern_param_name(uint8_t param) {
    static const char *names[PARAM_COUNT] = {
        "TRG", "ACC", "PRB", "VAR", "CAR", "RAT", "FIN", "DEP", "PEA", "PED", "PDR", "ATK", "DEC"
    };
    return names[param % PARAM_COUNT];
}

uint8_t pattern_get_value(const StepData *s, uint8_t param) {
    switch (param) {
        case PARAM_TRG: return s->trigger;
        case PARAM_ACC: return s->accent;
        case PARAM_PRB: return s->probability;
        case PARAM_VAR: return s->pitch_variation;
        case PARAM_CAR: return s->carrier_pitch;
        case PARAM_RAT: return s->mod_ratio;
        case PARAM_FIN: return (uint8_t)s->fine_tune;
        case PARAM_DEP: return s->fm_depth;
        case PARAM_PEA: return s->pitch_env_amount;
        case PARAM_PED: return s->pitch_env_decay;
        case PARAM_PDR: return s->pitch_env_direction;
        case PARAM_ATK: return s->amp_attack;
        default: return s->amp_decay;
    }
}

void pattern_set_value(StepData *s, uint8_t param, int16_t value) {
    switch (param) {
        case PARAM_TRG: s->trigger = clamp_u8(value, 0, 1); break;
        case PARAM_ACC: s->accent = clamp_u8(value, 0, 15); break;
        case PARAM_PRB: s->probability = clamp_u8(value, 0, 100); break;
        case PARAM_VAR: s->pitch_variation = clamp_u8(value, 0, 15); break;
        case PARAM_CAR: s->carrier_pitch = clamp_u8(value, 0, 95); break;
        case PARAM_RAT: s->mod_ratio = clamp_u8(value, 1, 16); break;
        case PARAM_FIN: s->fine_tune = clamp_i8(value, -16, 15); break;
        case PARAM_DEP: s->fm_depth = clamp_u8(value, 0, 31); break;
        case PARAM_PEA: s->pitch_env_amount = clamp_u8(value, 0, 31); break;
        case PARAM_PED: s->pitch_env_decay = clamp_u8(value, 0, 31); break;
        case PARAM_PDR: s->pitch_env_direction = clamp_u8(value, 0, 1); break;
        case PARAM_ATK: s->amp_attack = clamp_u8(value, 0, 15); break;
        case PARAM_DEC: s->amp_decay = clamp_u8(value, 0, 31); break;
    }
}

int16_t pattern_min_value(uint8_t param) {
    return (param == PARAM_RAT) ? 1 : ((param == PARAM_FIN) ? -16 : 0);
}

int16_t pattern_max_value(uint8_t param) {
    switch (param) {
        case PARAM_TRG: return 1;
        case PARAM_ACC:
        case PARAM_VAR:
        case PARAM_ATK: return 15;
        case PARAM_PRB: return 100;
        case PARAM_CAR: return 95;
        case PARAM_RAT: return 16;
        case PARAM_FIN: return 15;
        case PARAM_PDR: return 1;
        default: return 31;
    }
}

uint8_t pattern_big_delta(uint8_t param) {
    if (param == PARAM_PRB) return 10;
    if (param == PARAM_CAR) return 12;
    return 4;
}
