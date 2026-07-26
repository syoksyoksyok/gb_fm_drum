#include <gb/gb.h>
#include "pattern.h"
#include "sequencer.h"
#include "audio.h"
#include "randomizer.h"

uint8_t seq_playing = 0;
uint8_t seq_pos[NUM_TRACKS] = {0, 0};
static uint8_t pend_dir[NUM_TRACKS] = {1, 1};
static uint16_t tick_accum = 0;

void sequencer_init(void) {
    seq_playing = 0;
    sequencer_reset_positions();
}

void sequencer_reset_positions(void) {
    uint8_t t;
    for (t = 0; t < NUM_TRACKS; ++t) {
        seq_pos[t] = 0;
        pend_dir[t] = 1;
    }
    tick_accum = 0;
}

void sequencer_start(void) {
    sequencer_reset_positions();
    seq_playing = 1;
}

void sequencer_stop(void) {
    seq_playing = 0;
    audio_stop_all();
}

static void advance_track(uint8_t t) {
    TrackData *tr = &current_pattern.track[t];
    uint8_t len = tr->length;
    if (len <= 1) {
        seq_pos[t] = 0;
        return;
    }
    switch (tr->direction) {
        case DIR_REVERSE:
            seq_pos[t] = (seq_pos[t] == 0) ? (len - 1) : (seq_pos[t] - 1);
            break;
        case DIR_PENDULUM:
            if (pend_dir[t]) {
                if (seq_pos[t] >= len - 1) {
                    pend_dir[t] = 0;
                    seq_pos[t] = len - 2;
                } else {
                    seq_pos[t]++;
                }
            } else {
                if (seq_pos[t] == 0) {
                    pend_dir[t] = 1;
                    seq_pos[t] = 1;
                } else {
                    seq_pos[t]--;
                }
            }
            break;
        case DIR_RANDOM:
            seq_pos[t] = rng_range(len - 1);
            break;
        default:
            seq_pos[t]++;
            if (seq_pos[t] >= len) seq_pos[t] = 0;
            break;
    }
}

static void play_step(uint8_t t) {
    StepData *s = &current_pattern.track[t].steps[seq_pos[t]];
    if (s->trigger && s->probability && (rng_range(99) < s->probability)) {
        audio_trigger(t, s);
    }
}

void sequencer_update(void) {
    if (!seq_playing) return;
    if (current_pattern.bpm < BPM_MIN) current_pattern.bpm = BPM_MIN;
    if (current_pattern.bpm > BPM_MAX) current_pattern.bpm = BPM_MAX;
    tick_accum += (uint16_t)(current_pattern.bpm << 2);
    if (tick_accum >= 3600u) {
        tick_accum -= 3600u;
        play_step(TRACK_L);
        play_step(TRACK_R);
        advance_track(TRACK_L);
        advance_track(TRACK_R);
    }
}
