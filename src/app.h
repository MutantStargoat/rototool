#ifndef APP_H_
#define APP_H_

#include "app/controller.h"

enum {
	KEY_ESC = 27,
	KEY_DEL = 127,

	KEY_F1 = 256,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12,
	KEY_LEFT,
	KEY_UP,
	KEY_RIGHT,
	KEY_DOWN,
	KEY_PGUP,
	KEY_PGDOWN,
	KEY_HOME,
	KEY_END,
	KEY_INSERT,

	KEY_SHIFT,
	KEY_CTRL,
	KEY_ALT
};

enum {
	MODKEY_SHIFT	= 1,
	MODKEY_CTRL		= 2,
	MODKEY_ALT		= 4
};

enum MouseCursor {
	CURSOR_DEFAULT,
	CURSOR_CROSS,
	CURSOR_WAIT,

	NUM_MOUSE_CURSORS
};

extern int win_width, win_height;
extern float win_aspect;
extern Controller controller;

// video filter taps
enum {
	VF_COLOR_TAP,
	VF_EDGES_TAP,
	VF_PREVIEW_TAP,

	NUM_VF_TAPS
};

bool app_init(int argc, char **argv);
void app_shutdown();

void app_display();
void app_reshape(int x, int y);
void app_keyboard(int key, bool pressed);
void app_mouse_button(int bn, bool pressed, int x, int y);
void app_mouse_motion(int x, int y);
void app_passive_mouse_motion(int x, int y);
void app_mouse_wheel(int delta);

// --- implemented in main_*.cc ---

// stop processing events and shut down. app_shutdown will be called.
void app_quit();

// multiple calls to app_redraw result in a single deferred app_display invocation
void app_redraw();

/* enable/disable generation of app_mouse_motion callbacks when no buttons are pressed
 * disabled by default to avoid a flood of mouse motion events
 */
void app_track_mouse(bool enable);

/* get time in milliseconds */
long app_get_msec();

int app_mouse_x();
int app_mouse_y();

/* get current modifier key bitmask */
unsigned int app_get_modifiers();

// set the mouse cursor (see MouseCursor enum)
void app_mouse_cursor(MouseCursor c);

#endif	// APP_H_
