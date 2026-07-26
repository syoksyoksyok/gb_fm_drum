#ifndef PATTERN_H
#define PATTERN_H

#include <stdint.h>

#define NUM_STEPS 16
#define NUM_TRACKS 2
#define NUM_PATTERNS 32

#define BPM_MIN 40
#define BPM_MAX 300

enum {
    TRACK_L = 0,
    TRACK_R = 1
};

typedef enum {
    DIR_FORWARD = 0,
    DIR_REVERSE = 1,
    DIR_PENDULUM = 2,
    DIR_RANDOM = 3
} Direction;

typedef enum {
    PARAM_TRG = 0,
    PARAM_ACC,
    PARAM_PRB,
    PARAM_VAR,
    PARAM_CAR,
    PARAM_RAT,
    PARAM_FIN,
    PARAM_DEP,
    PARAM_PEA,
    PARAM_PED,
    PARAM_PDR,
    PARAM_ATK,
    PARAM_DEC,
    PARAM_COUNT
} ParamId;

typedef struct {
    uint8_t trigger;
    uint8_t accent;
    uint8_t probability;
    uint8_t pitch_variation;
    uint8_t carrier_pitch;
    uint8_t mod_ratio;
    int8_t fine_tune;
    uint8_t fm_depth;
    uint8_t pitch_env_amount;
    uint8_t pitch_env_decay;
    uint8_t pitch_env_direction;
    uint8_t amp_attack;
    uint8_t amp_decay;
} StepData;

typedef struct {
    StepData steps[NUM_STEPS];
    uint8_t length;
    uint8_t direction;
} TrackData;

typedef struct {
    TrackData track[NUM_TRACKS];
    uint16_t bpm;
    uint8_t random_strength;
} PatternData;

extern PatternData current_pattern;
extern PatternData undo_pattern;
extern uint8_t undo_valid;

void pattern_init_demo(PatternData *p);
void pattern_init_empty(PatternData *p);
void pattern_clamp(PatternData *p);
uint8_t pattern_checksum(const PatternData *p);
const char *pattern_param_name(uint8_t param);
uint8_t pattern_get_value(const StepData *s, uint8_t param);
void pattern_set_value(StepData *s, uint8_t param, int16_t value);
int16_t pattern_min_value(uint8_t param);
int16_t pattern_max_value(uint8_t param);
uint8_t pattern_big_delta(uint8_t param);

#endif
