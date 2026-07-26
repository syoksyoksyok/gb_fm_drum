#include <gb/gb.h>
#include <gbdk/console.h>
#include <stdio.h>
#include <string.h>
#include "tracker_ui.h"
#include "pattern.h"
#include "sequencer.h"

uint8_t ui_step = 0;
uint8_t ui_track = TRACK_L;
uint8_t ui_param = PARAM_TRG;
uint8_t ui_header_mode = 0;
uint8_t ui_header_item = 0;
uint8_t ui_pattern_index = 0;

static uint8_t full_redraw = 1;
static uint8_t header_redraw = 1;
static uint16_t step_redraw = 0xffffu;
static uint8_t last_seq_pos[NUM_TRACKS] = {0xffu, 0xffu};
static uint8_t flash_timer = 0;
static char flash_msg[9];

static const char dir_chars[4] = {'F', 'R', 'P', 'X'};
static const char digit_pairs[100][2] = {
    {'0','0'}, {'0','1'}, {'0','2'}, {'0','3'}, {'0','4'}, {'0','5'}, {'0','6'}, {'0','7'}, {'0','8'}, {'0','9'},
    {'1','0'}, {'1','1'}, {'1','2'}, {'1','3'}, {'1','4'}, {'1','5'}, {'1','6'}, {'1','7'}, {'1','8'}, {'1','9'},
    {'2','0'}, {'2','1'}, {'2','2'}, {'2','3'}, {'2','4'}, {'2','5'}, {'2','6'}, {'2','7'}, {'2','8'}, {'2','9'},
    {'3','0'}, {'3','1'}, {'3','2'}, {'3','3'}, {'3','4'}, {'3','5'}, {'3','6'}, {'3','7'}, {'3','8'}, {'3','9'},
    {'4','0'}, {'4','1'}, {'4','2'}, {'4','3'}, {'4','4'}, {'4','5'}, {'4','6'}, {'4','7'}, {'4','8'}, {'4','9'},
    {'5','0'}, {'5','1'}, {'5','2'}, {'5','3'}, {'5','4'}, {'5','5'}, {'5','6'}, {'5','7'}, {'5','8'}, {'5','9'},
    {'6','0'}, {'6','1'}, {'6','2'}, {'6','3'}, {'6','4'}, {'6','5'}, {'6','6'}, {'6','7'}, {'6','8'}, {'6','9'},
    {'7','0'}, {'7','1'}, {'7','2'}, {'7','3'}, {'7','4'}, {'7','5'}, {'7','6'}, {'7','7'}, {'7','8'}, {'7','9'},
    {'8','0'}, {'8','1'}, {'8','2'}, {'8','3'}, {'8','4'}, {'8','5'}, {'8','6'}, {'8','7'}, {'8','8'}, {'8','9'},
    {'9','0'}, {'9','1'}, {'9','2'}, {'9','3'}, {'9','4'}, {'9','5'}, {'9','6'}, {'9','7'}, {'9','8'}, {'9','9'}
};
static const char param_names[PARAM_COUNT][3] = {
    {'T','R','G'}, {'A','C','C'}, {'P','R','B'}, {'V','A','R'}, {'C','A','R'}, {'R','A','T'}, {'F','I','N'},
    {'D','E','P'}, {'P','E','A'}, {'P','E','D'}, {'P','D','R'}, {'A','T','K'}, {'D','E','C'}
};

static void print_lit(const char *s) {
    while (*s) putchar(*s++);
}

static void print_u8_2(uint8_t v) {
    putchar(digit_pairs[v][0]);
    putchar(digit_pairs[v][1]);
}

static void print_u16_3(uint16_t v) {
    if (v >= 300u) {
        putchar('3');
        v -= 300u;
    } else if (v >= 200u) {
        putchar('2');
        v -= 200u;
    } else if (v >= 100u) {
        putchar('1');
        v -= 100u;
    } else {
        putchar('0');
    }
    print_u8_2((uint8_t)v);
}

static void print_param_name(uint8_t param) {
    if (param >= PARAM_COUNT) param = 0;
    putchar(param_names[param][0]);
    putchar(param_names[param][1]);
    putchar(param_names[param][2]);
}

void ui_init(void) {
    DISPLAY_ON;
    SHOW_BKG;
    ui_request_full_redraw();
}

void ui_request_full_redraw(void) {
    full_redraw = 1;
    header_redraw = 1;
    step_redraw = 0xffffu;
}

void ui_request_header_redraw(void) {
    header_redraw = 1;
}

void ui_request_step_redraw(uint8_t step) {
    step_redraw |= (uint16_t)(1u << step);
}

void ui_flash(const char *msg, uint8_t frames) {
    strncpy(flash_msg, msg, 8);
    flash_msg[8] = 0;
    flash_timer = frames;
    ui_request_full_redraw();
}

static void print_signed(int8_t v) {
    if (v >= 0) {
        putchar('+');
        print_u8_2((uint8_t)v);
    } else {
        putchar('-');
        print_u8_2((uint8_t)-v);
    }
}

static void print_cell_value(StepData *s, uint8_t param) {
    uint8_t v = pattern_get_value(s, param);
    if (param == PARAM_FIN) print_signed((int8_t)v);
    else if (param == PARAM_PDR) {
        putchar(' ');
        putchar(v ? 'U' : 'D');
        putchar(' ');
    } else {
        print_u16_3((uint16_t)v);
    }
}

static void print_lr_values(StepData *l, StepData *r, uint8_t param) {
    print_cell_value(l, param);
    putchar('|');
    print_cell_value(r, param);
}

static void print_flash_line(void) {
    uint8_t i;
    for (i = 0; i < 20u; ++i) {
        putchar((i < 8u && flash_msg[i]) ? flash_msg[i] : ' ');
    }
}

static void request_seq_pos_redraw(void) {
    uint8_t t, pos;
    for (t = 0; t < NUM_TRACKS; ++t) {
        pos = seq_playing ? seq_pos[t] : 0xffu;
        if (last_seq_pos[t] != pos) {
            if (last_seq_pos[t] < NUM_STEPS) ui_request_step_redraw(last_seq_pos[t]);
            if (pos < NUM_STEPS) ui_request_step_redraw(pos);
            last_seq_pos[t] = pos;
        }
    }
}

static void draw_header(uint8_t next_param) {
    gotoxy(0, 0);
    putchar('P');
    print_u8_2(ui_pattern_index + 1u);
    print_lit("|B");
    print_u16_3(current_pattern.bpm);
    print_lit("|L");
    print_u8_2(current_pattern.track[0].length);
    putchar(dir_chars[current_pattern.track[0].direction]);
    print_lit("|R");
    print_u8_2(current_pattern.track[1].length);
    putchar(dir_chars[current_pattern.track[1].direction]);
    putchar('|');
    putchar(seq_playing ? '>' : 'S');

    gotoxy(0, 1);
    putchar(ui_header_mode ? 'H' : 'S');
    putchar(ui_header_mode ? 'D' : 'T');
    print_lit(" |");
    print_param_name(ui_param);
    print_lit("|   |");
    print_param_name(next_param);
    print_lit("|    ");
}

static void draw_step_row(uint8_t i, uint8_t next_param) {
    StepData *l = &current_pattern.track[TRACK_L].steps[i];
    StepData *r = &current_pattern.track[TRACK_R].steps[i];
    uint8_t selected = !ui_header_mode && ui_step == i;
    uint8_t playing = seq_playing && (seq_pos[TRACK_L] == i || seq_pos[TRACK_R] == i);
    uint8_t pos_mark = ' ';
    if (selected && playing) pos_mark = '*';
    else if (selected) pos_mark = (ui_track == TRACK_L) ? 'L' : 'R';
    else if (playing) pos_mark = '>';
    gotoxy(0, (uint8_t)(i + 2u));
    print_u8_2(i + 1u);
    putchar(pos_mark);
    putchar('|');
    print_lr_values(l, r, ui_param);
    putchar('|');
    print_lr_values(l, r, next_param);
    putchar(' ');
}

void ui_draw(void) {
    uint8_t i, next_param;
    request_seq_pos_redraw();
    if (!full_redraw && !header_redraw && !step_redraw && !flash_timer) return;
    next_param = ui_param + 1u;
    if (next_param >= PARAM_COUNT) next_param = 0;
    if (flash_timer) {
        gotoxy(0, 0);
        print_flash_line();
        flash_timer--;
        return;
    }
    if (full_redraw || header_redraw) {
        draw_header(next_param);
        header_redraw = 0;
    }
    for (i = 0; i < NUM_STEPS; ++i) {
        if (full_redraw || (step_redraw & (uint16_t)(1u << i))) draw_step_row(i, next_param);
    }
    full_redraw = 0;
    step_redraw = 0;
}
