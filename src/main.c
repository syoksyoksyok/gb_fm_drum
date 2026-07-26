#include <gb/gb.h>
#include <stdint.h>
#include <string.h>
#include "audio.h"
#include "input.h"
#include "pattern.h"
#include "randomizer.h"
#include "sequencer.h"
#include "storage.h"
#include "tracker_ui.h"

static uint16_t frame_counter = 0;
static uint8_t select_a_hold = 0;
static uint8_t select_a_long_done = 0;

static void copy_pattern(PatternData *dst, const PatternData *src) {
    memcpy(dst, src, sizeof(PatternData));
}

static void edit_step_value(int8_t delta) {
    StepData *s = &current_pattern.track[ui_track].steps[ui_step];
    int16_t v = (ui_param == PARAM_FIN) ? s->fine_tune : pattern_get_value(s, ui_param);
    pattern_set_value(s, ui_param, v + delta);
    ui_request_full_redraw();
}

static void edit_header_value(int8_t delta, uint8_t big) {
    int16_t d = delta;
    if (big) d *= (ui_header_item == 1) ? 10 : 4;
    switch (ui_header_item) {
        case 0:
            ui_pattern_index = (uint8_t)((int16_t)ui_pattern_index + d < 0 ? 0 : ((int16_t)ui_pattern_index + d > 31 ? 31 : (int16_t)ui_pattern_index + d));
            break;
        case 1:
            current_pattern.bpm = (uint16_t)((int16_t)current_pattern.bpm + d < BPM_MIN ? BPM_MIN : ((int16_t)current_pattern.bpm + d > BPM_MAX ? BPM_MAX : (int16_t)current_pattern.bpm + d));
            break;
        case 2:
        case 3: {
            TrackData *tr = &current_pattern.track[ui_header_item - 2u];
            tr->length = (uint8_t)((int16_t)tr->length + d < 1 ? 1 : ((int16_t)tr->length + d > 16 ? 16 : (int16_t)tr->length + d));
            if (seq_pos[ui_header_item - 2u] >= tr->length) seq_pos[ui_header_item - 2u] = tr->length - 1u;
            break;
        }
        case 4:
        case 5: {
            TrackData *tr = &current_pattern.track[ui_header_item - 4u];
            int16_t v = (int16_t)tr->direction + delta;
            if (v < 0) v = 3;
            if (v > 3) v = 0;
            tr->direction = (uint8_t)v;
            break;
        }
        default:
            current_pattern.random_strength = (uint8_t)((int16_t)current_pattern.random_strength + (d * 5) < 0 ? 0 : ((int16_t)current_pattern.random_strength + (d * 5) > 100 ? 100 : (int16_t)current_pattern.random_strength + (d * 5)));
            break;
    }
    ui_request_full_redraw();
}

static void do_randomize(uint8_t full) {
    copy_pattern(&undo_pattern, &current_pattern);
    undo_valid = 1;
    if (full) {
        randomizer_full(&current_pattern);
        ui_flash("FULLRND", 45);
    } else {
        randomizer_musical(&current_pattern);
        ui_flash("RANDOM", 35);
    }
}

static void handle_input(void) {
    uint8_t sel = input.now & J_SELECT;
    uint8_t a = input.now & J_A;
    rng_mix((uint8_t)(input.now ^ frame_counter ^ seq_pos[0] ^ (seq_pos[1] << 4)));

    if (sel && a) {
        if (select_a_hold < 255) select_a_hold++;
        if (select_a_hold >= 60 && !select_a_long_done) {
            do_randomize(1);
            select_a_long_done = 1;
        }
    }
    if ((input.released & J_A) && select_a_hold && !select_a_long_done) do_randomize(0);
    if (!(sel && a)) {
        select_a_hold = 0;
        select_a_long_done = 0;
    }

    if ((input.pressed & J_START) && !sel) {
        if (seq_playing) sequencer_stop(); else sequencer_start();
        ui_request_full_redraw();
    }
    if (sel && (input.pressed & J_START)) {
        storage_save_pattern(ui_pattern_index, &current_pattern);
        storage_set_last_pattern(ui_pattern_index);
        ui_flash("SAVE", 35);
    }
    if (sel && (input.pressed & J_B)) {
        if (undo_valid) {
            copy_pattern(&current_pattern, &undo_pattern);
            undo_valid = 0;
            ui_flash("UNDO", 35);
        }
    }
    if ((input.pressed & J_B) && !(sel && (input.now & J_B))) {
        StepData *s = &current_pattern.track[ui_track].steps[ui_step];
        s->trigger ^= 1u;
        ui_request_full_redraw();
    }

    if (sel && (input.pressed & J_UP)) {
        ui_header_mode ^= 1u;
        ui_request_full_redraw();
        return;
    }
    if (sel && (input.pressed & J_DOWN)) {
        ui_header_mode = 0;
        ui_request_full_redraw();
        return;
    }
    if (sel && (input.pressed & J_RIGHT)) {
        ui_param = (ui_param + 1u) % PARAM_COUNT;
        ui_request_full_redraw();
        return;
    }
    if (sel && (input.pressed & J_LEFT)) {
        ui_param = (ui_param == 0) ? (PARAM_COUNT - 1u) : (ui_param - 1u);
        ui_request_full_redraw();
        return;
    }

    if (ui_header_mode) {
        if ((input.pressed & J_A) && ui_header_item == 0 && !(input.now & (J_UP | J_DOWN | J_LEFT | J_RIGHT))) {
            storage_load_pattern(ui_pattern_index, &current_pattern);
            storage_set_last_pattern(ui_pattern_index);
            ui_flash("LOAD", 35);
        }
        if (input.now & J_A) {
            if (input.repeat & J_UP) edit_header_value(1, 0);
            if (input.repeat & J_DOWN) edit_header_value(-1, 0);
            if (input.repeat & J_RIGHT) edit_header_value(1, 1);
            if (input.repeat & J_LEFT) edit_header_value(-1, 1);
        } else {
            if (input.repeat & J_LEFT) { ui_header_item = (ui_header_item == 0) ? 6 : (ui_header_item - 1u); ui_request_full_redraw(); }
            if (input.repeat & J_RIGHT) { ui_header_item = (ui_header_item + 1u) % 7u; ui_request_full_redraw(); }
        }
        return;
    }

    if (input.now & J_A) {
        if (input.repeat & J_UP) edit_step_value(1);
        if (input.repeat & J_DOWN) edit_step_value(-1);
        if (input.repeat & J_RIGHT) edit_step_value(pattern_big_delta(ui_param));
        if (input.repeat & J_LEFT) edit_step_value(-(int8_t)pattern_big_delta(ui_param));
    } else {
        if (input.repeat & J_UP) { ui_step = (ui_step == 0) ? 15 : (ui_step - 1u); ui_request_full_redraw(); }
        if (input.repeat & J_DOWN) { ui_step = (ui_step + 1u) & 15u; ui_request_full_redraw(); }
        if (input.repeat & (J_LEFT | J_RIGHT)) { ui_track ^= 1u; ui_request_full_redraw(); }
    }
}

void main(void) {
    audio_init();
    storage_init();
    ui_pattern_index = storage_last_pattern();
    storage_load_pattern(ui_pattern_index, &current_pattern);
    rng_seed(0x5147u);
    sequencer_init();
    ui_init();
    if (storage_had_error()) ui_flash("BAD SAVE", 80);
    while (1) {
        wait_vbl_done();
        frame_counter++;
        input_update();
        handle_input();
        sequencer_update();
        audio_update();
        ui_draw();
    }
}
