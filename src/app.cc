#include <assert.h>
#include "app.h"
#include "opengl.h"

// updated by main_*.cc
int win_width, win_height;
float win_aspect;

bool app_init(int argc, char **argv)
{
	if(init_opengl() == -1) {
		return false;
	}
	return true;
}

void app_shutdown()
{
}

void app_display()
{
	glClearColor(0.1, 0.1, 0.1, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	assert(glGetError() == GL_NO_ERROR);
}

void app_reshape(int x, int y)
{
	glViewport(0, 0, x, y);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, x, y, 0, -1, 1);
}

void app_keyboard(int key, bool pressed)
{
	if(pressed) {
		switch(key) {
		case KEY_ESC:
			app_quit();
			break;

		default:
			break;
		}
	}
}

void app_mouse_button(int bn, bool pressed, int x, int y)
{
}

void app_mouse_motion(int x, int y)
{
}
