#include "randomizer.h"

static uint16_t rng_state = 0xace1u;

void rng_seed(uint16_t seed) {
    rng_state ^= seed ? seed : 0x1d3bu;
}

uint8_t rng_u8(void) {
    uint16_t x = rng_state;
    x ^= x << 7;
    x ^= x >> 9;
    x ^= x << 8;
    rng_state = x;
    return (uint8_t)(x ^ (x >> 8));
}

uint16_t rng_u16(void) {
    return ((uint16_t)rng_u8() << 8) | rng_u8();
}

uint8_t rng_range(uint8_t max_inclusive) {
    return (uint8_t)(rng_u8() % (max_inclusive + 1u));
}

void rng_mix(uint8_t value) {
    rng_state ^= ((uint16_t)value << 8) | (rng_state >> 8);
    rng_u8();
}

static uint8_t choose_from(const uint8_t *v, uint8_t n) {
    return v[rng_u8() % n];
}

static int16_t near_value(int16_t cur, int16_t lo, int16_t hi, uint8_t strength, uint8_t spread) {
    int16_t span = (int16_t)((uint16_t)spread * strength / 100u);
    int16_t v = cur + (int16_t)(rng_range((uint8_t)(span * 2u)) - span);
    if (v < lo) v = lo;
    if (v > hi) v = hi;
    return v;
}

static void ensure_trigger(TrackData *t) {
    uint8_t i;
    for (i = 0; i < t->length; ++i) if (t->steps[i].trigger) return;
    t->steps[rng_range(t->length - 1)].trigger = 1;
}

void randomizer_musical(PatternData *p) {
    static const uint8_t probs[] = {50, 60, 70, 75, 80, 90, 100};
    static const uint8_t ratios[] = {1, 2, 3, 4, 6, 8};
    static const uint8_t lengths[] = {4, 6, 7, 8, 12, 16};
    uint8_t t, i, strength = p->random_strength;
    for (t = 0; t < NUM_TRACKS; ++t) {
        TrackData *tr = &p->track[t];
        if (rng_range(99) < strength) tr->length = choose_from(lengths, sizeof(lengths));
        if (rng_range(99) < (strength >> 1)) tr->direction = rng_range(3);
        for (i = 0; i < NUM_STEPS; ++i) {
            StepData *s = &tr->steps[i];
            uint8_t strong = ((i & 3) == 0);
            uint8_t chance = strong ? 52 : ((i & 1) ? 18 : 30);
            chance = (uint8_t)((chance * (40u + strength)) / 90u);
            s->trigger = (rng_range(99) < chance);
            s->accent = s->trigger ? (strong ? (10 + rng_range(5)) : rng_range(12)) : rng_range(4);
            s->probability = strong ? 100 : choose_from(probs, sizeof(probs));
            s->pitch_variation = rng_range((uint8_t)(strength / 8u));
            s->carrier_pitch = (uint8_t)near_value(s->carrier_pitch, 0, 95, strength, t == TRACK_L ? 18 : 24);
            if (t == TRACK_L && s->carrier_pitch > 52) s->carrier_pitch = 28 + rng_range(20);
            if (t == TRACK_R && s->carrier_pitch < 35) s->carrier_pitch = 42 + rng_range(34);
            s->mod_ratio = choose_from(ratios, sizeof(ratios));
            s->fine_tune = (int8_t)near_value(s->fine_tune, -16, 15, strength, 8);
            s->fm_depth = (uint8_t)near_value(s->fm_depth, 0, 31, strength, 12);
            s->pitch_env_amount = (uint8_t)near_value(s->pitch_env_amount, 0, 31, strength, 12);
            s->pitch_env_decay = (uint8_t)near_value(s->pitch_env_decay, 0, 31, strength, 10);
            s->pitch_env_direction = (rng_range(99) < 75) ? 0 : 1;
            s->amp_attack = rng_range(4);
            s->amp_decay = 3 + rng_range(14);
        }
        ensure_trigger(tr);
    }
    p->bpm = (uint16_t)near_value(p->bpm, BPM_MIN, BPM_MAX, strength, strength <= 50 ? 20 : 60);
    pattern_clamp(p);
}

void randomizer_full(PatternData *p) {
    uint8_t t, i;
    for (t = 0; t < NUM_TRACKS; ++t) {
        TrackData *tr = &p->track[t];
        tr->length = 1 + rng_range(15);
        tr->direction = rng_range(3);
        for (i = 0; i < NUM_STEPS; ++i) {
            StepData *s = &tr->steps[i];
            s->trigger = rng_range(1);
            s->accent = rng_range(15);
            s->probability = rng_range(100);
            s->pitch_variation = rng_range(15);
            s->carrier_pitch = rng_range(95);
            s->mod_ratio = 1 + rng_range(15);
            s->fine_tune = (int8_t)(rng_range(31) - 16);
            s->fm_depth = rng_range(31);
            s->pitch_env_amount = rng_range(31);
            s->pitch_env_decay = rng_range(31);
            s->pitch_env_direction = rng_range(1);
            s->amp_attack = rng_range(15);
            s->amp_decay = rng_range(31);
        }
        ensure_trigger(tr);
    }
    p->bpm = BPM_MIN + (rng_u16() % (BPM_MAX - BPM_MIN + 1u));
    pattern_clamp(p);
}
