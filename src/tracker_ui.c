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

static void print_value(StepData *s) {
    uint8_t v = pattern_get_value(s, ui_param);
    if (ui_param == PARAM_FIN) print_signed((int8_t)v);
    else if (ui_param == PARAM_PDR) printf(" %c ", v ? 'U' : 'D');
    else if (ui_param == PARAM_PRB) printf("%03u", (uint16_t)v);
    else printf("%02u ", (uint16_t)v);
}

void ui_draw(void) {
    uint8_t i;
    if (!redraw && !flash_timer) return;
    gotoxy(0, 0);
    if (flash_timer) {
        printf("%-8s            ", flash_msg);
        flash_timer--;
    } else {
        printf("P%02uB%03u L%02uR%02u %c ",
               (uint16_t)ui_pattern_index + 1u, current_pattern.bpm,
               (uint16_t)current_pattern.track[0].length,
               (uint16_t)current_pattern.track[1].length,
               seq_playing ? '>' : 'S');
    }

    gotoxy(0, 1);
    printf("%c", ui_header_mode ? '^' : ' ');
    print_param_name(ui_param);
    printf(" L%c R%c R%03u    ",
           dir_chars[current_pattern.track[0].direction],
           dir_chars[current_pattern.track[1].direction],
           (uint16_t)current_pattern.random_strength);

    for (i = 0; i < NUM_STEPS; ++i) {
        StepData *l = &current_pattern.track[TRACK_L].steps[i];
        StepData *r = &current_pattern.track[TRACK_R].steps[i];
        gotoxy(0, (uint8_t)(i + 2));
        printf("%c%c%02u ", seq_playing && seq_pos[TRACK_L] == i ? 'L' : ' ',
               seq_playing && seq_pos[TRACK_R] == i ? 'R' : ' ', (uint16_t)i + 1u);
        printf("%c", (!ui_header_mode && ui_step == i && ui_track == TRACK_L) ? '[' : ' ');
        print_value(l);
        printf("%c ", (!ui_header_mode && ui_step == i && ui_track == TRACK_L) ? ']' : ' ');
        printf("%c", (!ui_header_mode && ui_step == i && ui_track == TRACK_R) ? '[' : ' ');
        print_value(r);
        printf("%c ", (!ui_header_mode && ui_step == i && ui_track == TRACK_R) ? ']' : ' ');
    }
    redraw = 0;
}
