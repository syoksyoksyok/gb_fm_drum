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

static void print_param_name(uint8_t param) {
    switch (param) {
        case PARAM_ACC: printf("ACC"); break;
        case PARAM_PRB: printf("PRB"); break;
        case PARAM_VAR: printf("VAR"); break;
        case PARAM_CAR: printf("CAR"); break;
        case PARAM_RAT: printf("RAT"); break;
        case PARAM_FIN: printf("FIN"); break;
        case PARAM_DEP: printf("DEP"); break;
        case PARAM_PEA: printf("PEA"); break;
        case PARAM_PED: printf("PED"); break;
        case PARAM_PDR: printf("PDR"); break;
        case PARAM_ATK: printf("ATK"); break;
        case PARAM_DEC: printf("DEC"); break;
        default: printf("TRG"); break;
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
    if (v >= 0) printf("+%02d", (int16_t)v);
    else printf("-%02d", (int16_t)-v);
}

static void print_cell_value(StepData *s, uint8_t param) {
    uint8_t v = pattern_get_value(s, param);
    if (param == PARAM_FIN) print_signed((int8_t)v);
    else if (param == PARAM_PDR) printf(" %c ", v ? 'U' : 'D');
    else if (param == PARAM_PRB) printf("%03u", (uint16_t)v);
    else printf("%03u", (uint16_t)v);
}

static void print_lr_values(StepData *l, StepData *r, uint8_t param) {
    print_cell_value(l, param);
    printf("|");
    print_cell_value(r, param);
}

void ui_draw(void) {
    uint8_t i, next_param;
    if (!redraw && !flash_timer) return;
    next_param = (ui_param + 1u) % PARAM_COUNT;
    gotoxy(0, 0);
    if (flash_timer) {
        printf("%-8s            ", flash_msg);
        flash_timer--;
    } else {
        printf("P%02u|B%03u|L%02u%c|R%02u%c|%c",
               (uint16_t)ui_pattern_index + 1u, current_pattern.bpm,
               (uint16_t)current_pattern.track[0].length,
               dir_chars[current_pattern.track[0].direction],
               (uint16_t)current_pattern.track[1].length,
               dir_chars[current_pattern.track[1].direction],
               seq_playing ? '>' : 'S');
    }

    gotoxy(0, 1);
    printf("%c%c |", ui_header_mode ? 'H' : 'S', ui_header_mode ? 'D' : 'T');
    print_param_name(ui_param);
    printf("|   |");
    print_param_name(next_param);
    printf("|    ");

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
        printf("                    ");
        gotoxy(0, (uint8_t)(i + 2));
        printf("%02u%c|", (uint16_t)i + 1u, pos_mark);
        print_lr_values(l, r, ui_param);
        printf("|");
        print_lr_values(l, r, next_param);
    }
    redraw = 0;
}
