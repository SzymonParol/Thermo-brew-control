#ifndef UI_H_
#define UI_H_

#include <stdint.h>

extern volatile uint8_t ui_needs_redraw;

void UI_RequestRedraw(void);
void UI_MoveCursor(uint8_t old_item, uint8_t new_item);
void System_Draw(void);

#endif /* UI_H_ */
