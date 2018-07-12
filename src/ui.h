#ifndef UI_H_
#define UI_H_

#include "utk/ubertk.h"

bool init_ui();
void destroy_ui();
void draw_ui();

utk::Container *ui_root();

void ui_reshape(int x, int y);
bool ui_keyboard(int key, bool pressed);
bool ui_mouse_motion(int x, int y);
bool ui_mouse_button(int bn, bool pressed, int x, int y);

#endif	// UI_H_
