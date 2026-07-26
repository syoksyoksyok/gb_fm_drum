#include <gb/gb.h>
#include "audio.h"
#include "randomizer.h"

typedef struct {
    uint8_t active;
    uint8_t age;
    uint8_t base_pitch;
    uint8_t mod_ratio;
    uint8_t fm_depth;
    uint8_t pitch_env_amount;
    uint8_t pitch_env_decay;
    uint8_t pitch_env_direction;
    uint8_t amp_attack;
    uint8_t amp_decay;
    int8_t fine_tune;
    uint8_t phase;
    uint8_t env_amt;
} Voice;

static Voice voices[NUM_TRACKS];

#define AUDIO_MAX_VOL 15u
#define AUDIO_ENV_DEN_MAX 176u

static const uint16_t pitch_table[96] = {
    44,88,132,176,220,264,308,352,396,440,484,528,
    572,616,660,704,748,792,836,880,924,968,1012,1056,
    1100,1144,1188,1232,1276,1320,1364,1408,1452,1496,1540,1584,
    1620,1640,1660,1680,1700,1720,1740,1760,1780,1800,1816,1832,
    1848,1864,1880,1896,1912,1928,1940,1952,1964,1976,1988,2000,
    2010,2020,2030,2040,2050,2060,2070,2080,2090,2100,2110,2120,
    2130,2140,2150,2160,2170,2180,2190,2200,2210,2220,2230,2240,
    2250,2260,2270,2280,2290,2300,2310,2320,2330,2340,2350,2360
};

static const uint8_t recip_q7_table[AUDIO_ENV_DEN_MAX + 1u] = {
    0,0,64,43,32,26,21,18,16,14,13,12,11,10,9,9,
    8,8,7,7,6,6,6,6,5,5,5,5,5,4,4,4,
    4,4,4,4,4,3,3,3,3,3,3,3,3,3,3,3,
    3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1
};

static uint16_t clamp_freq(int16_t v) {
    if (v < 32) return 32;
    if (v > 2047) return 2047;
    return (uint16_t)v;
}

static void write_freq(uint8_t track, uint16_t f, uint8_t trigger, uint8_t vol) {
    uint8_t env = (uint8_t)((vol & 0x0f) << 4);
    if (track == TRACK_L) {
        NR12_REG = env;
        NR13_REG = (uint8_t)f;
        NR14_REG = (uint8_t)(0x40 | ((f >> 8) & 0x07) | (trigger ? 0x80 : 0));
    } else {
        NR22_REG = env;
        NR23_REG = (uint8_t)f;
        NR24_REG = (uint8_t)(0x40 | ((f >> 8) & 0x07) | (trigger ? 0x80 : 0));
    }
}

static uint8_t pitch_env_amount_at(const Voice *v) {
    uint8_t den;
    if (v->pitch_env_decay == 0) return 0;
    den = v->age + v->pitch_env_decay + 1u;
    if (den > AUDIO_ENV_DEN_MAX) den = AUDIO_ENV_DEN_MAX;
    return (uint8_t)(((uint16_t)v->pitch_env_amount * (v->pitch_env_decay + 1u) * recip_q7_table[den]) >> 7);
}

void audio_init(void) {
    NR52_REG = 0x80;
    NR50_REG = 0x77;
    NR51_REG = 0x12;
    NR10_REG = 0x00;
    NR11_REG = 0xc0;
    NR21_REG = 0xc0;
    audio_stop_all();
}

void audio_stop_all(void) {
    voices[0].active = 0;
    voices[1].active = 0;
    NR12_REG = 0;
    NR22_REG = 0;
    NR14_REG = 0x40;
    NR24_REG = 0x40;
}

void audio_trigger(uint8_t track, const StepData *s) {
    Voice *v = &voices[track];
    int8_t pv = 0;
    if (s->pitch_variation) pv = (int8_t)(rng_range((uint8_t)(s->pitch_variation * 2u)) - s->pitch_variation);
    v->active = 1;
    v->age = 0;
    v->base_pitch = (uint8_t)((int16_t)s->carrier_pitch + pv < 0 ? 0 : ((int16_t)s->carrier_pitch + pv > 95 ? 95 : (int16_t)s->carrier_pitch + pv));
    v->mod_ratio = s->mod_ratio;
    v->fm_depth = s->fm_depth + (s->accent >> 2);
    if (v->fm_depth > 31) v->fm_depth = 31;
    v->pitch_env_amount = s->pitch_env_amount + (s->accent >> 3);
    if (v->pitch_env_amount > 31) v->pitch_env_amount = 31;
    v->pitch_env_decay = s->pitch_env_decay;
    v->pitch_env_direction = s->pitch_env_direction;
    v->amp_attack = s->amp_attack;
    v->amp_decay = s->amp_decay;
    v->fine_tune = s->fine_tune;
    v->phase = 0;
    v->env_amt = v->pitch_env_amount;
    write_freq(track, pitch_table[v->base_pitch], 1, AUDIO_MAX_VOL);
}

void audio_update(void) {
    uint8_t t;
    for (t = 0; t < NUM_TRACKS; ++t) {
        Voice *v = &voices[t];
        uint8_t fm_phase;
        uint8_t decay_age, decay_len;
        int16_t f;
        if (!v->active) continue;
        v->age++;
        decay_age = (v->age > v->amp_attack) ? (v->age - v->amp_attack) : 0;
        decay_len = 6 + (v->amp_decay << 2);
        if (decay_age >= decay_len || v->age > 144) {
            v->active = 0;
            write_freq(t, pitch_table[v->base_pitch], 0, 0);
            continue;
        }
        if ((v->age & 1u) == 0) {
            v->env_amt = pitch_env_amount_at(v);
        }
        f = (int16_t)pitch_table[v->base_pitch] + v->fine_tune;
        f += v->pitch_env_direction ? -(int16_t)(v->env_amt << 2) : (int16_t)(v->env_amt << 2);
        v->phase += v->mod_ratio;
        fm_phase = (v->phase & 0x10) ? 1 : 0;
        f += fm_phase ? (int16_t)v->fm_depth : -(int16_t)v->fm_depth;
        write_freq(t, clamp_freq(f), 0, AUDIO_MAX_VOL);
    }
}
