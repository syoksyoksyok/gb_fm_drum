#ifndef TRACKER_UI_H
#define TRACKER_UI_H

#include <stdint.h>

extern uint8_t ui_step;
extern uint8_t ui_track;
extern uint8_t ui_param;
extern uint8_t ui_header_mode;
extern uint8_t ui_header_item;
extern uint8_t ui_pattern_index;

void ui_init(void);
void ui_request_full_redraw(void);
void ui_request_header_redraw(void);
void ui_request_step_redraw(uint8_t step);
void ui_draw(void);
void ui_flash(const char *msg, uint8_t frames);

#endif
