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

static uint8_t redraw = 1;
static uint8_t flash_timer = 0;
static char flash_msg[9];

static const char dir_chars[4] = {'F', 'R', 'P', 'X'};

static void print_lit(const char *s) {
    while (*s) putchar(*s++);
}

static void print_u8_2(uint8_t v) {
    putchar((char)('0' + (v / 10u)));
    putchar((char)('0' + (v % 10u)));
}

static void print_u16_3(uint16_t v) {
    putchar((char)('0' + ((v / 100u) % 10u)));
    putchar((char)('0' + ((v / 10u) % 10u)));
    putchar((char)('0' + (v % 10u)));
}

static void print_param_name(uint8_t param) {
    switch (param) {
        case PARAM_ACC: print_lit("ACC"); break;
        case PARAM_PRB: print_lit("PRB"); break;
        case PARAM_VAR: print_lit("VAR"); break;
        case PARAM_CAR: print_lit("CAR"); break;
        case PARAM_RAT: print_lit("RAT"); break;
        case PARAM_FIN: print_lit("FIN"); break;
        case PARAM_DEP: print_lit("DEP"); break;
        case PARAM_PEA: print_lit("PEA"); break;
        case PARAM_PED: print_lit("PED"); break;
        case PARAM_PDR: print_lit("PDR"); break;
        case PARAM_ATK: print_lit("ATK"); break;
        case PARAM_DEC: print_lit("DEC"); break;
        default: print_lit("TRG"); break;
    }
}

void ui_init(void) {
    DISPLAY_ON;
    SHOW_BKG;
    redraw = 1;
}

void ui_request_full_redraw(void) {
    redraw = 1;
}

void ui_flash(const char *msg, uint8_t frames) {
    strncpy(flash_msg, msg, 8);
    flash_msg[8] = 0;
    flash_timer = frames;
    redraw = 1;
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

void ui_draw(void) {
    uint8_t i, next_param;
    if (!redraw && !flash_timer) return;
    next_param = (ui_param + 1u) % PARAM_COUNT;
    gotoxy(0, 0);
    if (flash_timer) {
        print_flash_line();
        flash_timer--;
        return;
    }
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

    for (i = 0; i < NUM_STEPS; ++i) {
        StepData *l = &current_pattern.track[TRACK_L].steps[i];
        StepData *r = &current_pattern.track[TRACK_R].steps[i];
        uint8_t selected = !ui_header_mode && ui_step == i;
        uint8_t pos_mark = ' ';
        if (selected) pos_mark = (ui_track == TRACK_L) ? 'L' : 'R';
        else if (seq_playing && seq_pos[TRACK_L] == i && seq_pos[TRACK_R] == i) pos_mark = '*';
        else if (seq_playing && seq_pos[TRACK_L] == i) pos_mark = 'L';
        else if (seq_playing && seq_pos[TRACK_R] == i) pos_mark = 'R';
        gotoxy(0, (uint8_t)(i + 2));
        print_u8_2(i + 1u);
        putchar(pos_mark);
        putchar('|');
        print_lr_values(l, r, ui_param);
        putchar('|');
        print_lr_values(l, r, next_param);
        putchar(' ');
    }
    redraw = 0;
}
