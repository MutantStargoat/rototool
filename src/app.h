#ifndef APP_H_
#define APP_H_

enum {
	KEY_ESC = 27
};

extern int win_width, win_height;
extern float win_aspect;

bool app_init(int argc, char **argv);
void app_shutdown();

void app_display();
void app_reshape(int x, int y);
void app_keyboard(int key, bool pressed);
void app_mouse_button(int bn, bool pressed, int x, int y);
void app_mouse_motion(int x, int y);

// --- implemented in main_*.cc ---

// stop processing events and shut down. app_shutdown will be called.
void app_quit();

// multiple calls to app_redraw result in a single deferred app_display invocation
void app_redraw();

/* enable/disable generation of app_mouse_motion callbacks when no buttons are pressed
 * disabled by default to avoid a flood of mouse motion events
 */
void app_track_mouse(bool enable);

#endif	// APP_H_
