#include <stdio.h>
#include <stdlib.h>
#include "opengl.h"
#include <GL/freeglut.h>
#include "app.h"

static void display();
static void reshape(int x, int y);
static void keydown(unsigned char key, int x, int y);
static void keyup(unsigned char key, int x, int y);
static void skeydown(int key, int x, int y);
static void skeyup(int key, int x, int y);
static void mouse(int bn, int st, int x, int y);
static void motion(int x, int y);
static void passive_motion(int x, int y);
static void wheel(int wheel, int dir, int x, int y);


int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(1024, 640);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_MULTISAMPLE);
	glutCreateWindow("rototool");

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keydown);
	glutKeyboardUpFunc(keyup);
	glutSpecialFunc(skeydown);
	glutSpecialUpFunc(skeyup);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(passive_motion);
	glutMouseWheelFunc(wheel);

	if(!app_init(argc, argv)) {
		return 1;
	}
	atexit(app_shutdown);

	glutMainLoop();
	return 0;
}

void app_quit()
{
	exit(0);
}

void app_redraw()
{
	glutPostRedisplay();
}

void app_track_mouse(bool enable)
{
	glutPassiveMotionFunc(enable ? motion : 0);
}

static void display()
{
	app_display();
	glutSwapBuffers();
}

static void reshape(int x, int y)
{
	win_width = x;
	win_height = y;
	win_aspect = (float)x / (float)y;
	app_reshape(x, y);
}

static void keydown(unsigned char key, int x, int y)
{
	app_keyboard(key, true);
}

static void keyup(unsigned char key, int x, int y)
{
	app_keyboard(key, false);
}


static int conv_skey(int key)
{
	if(key >= GLUT_KEY_F1 && key <= GLUT_KEY_F12) {
		return KEY_F1 + (key - GLUT_KEY_F1);
	}

	switch(key) {
	case GLUT_KEY_LEFT:
		return KEY_LEFT;
	case GLUT_KEY_UP:
		return KEY_UP;
	case GLUT_KEY_RIGHT:
		return KEY_RIGHT;
	case GLUT_KEY_DOWN:
		return KEY_DOWN;
	case GLUT_KEY_PAGE_UP:
		return KEY_PGUP;
	case GLUT_KEY_PAGE_DOWN:
		return KEY_PGDOWN;
	case GLUT_KEY_HOME:
		return KEY_HOME;
	case GLUT_KEY_END:
		return KEY_END;
	case GLUT_KEY_INSERT:
		return KEY_INSERT;
	default:
		break;
	}

	return 0;
}

static void skeydown(int key, int x, int y)
{
	app_keyboard(conv_skey(key), true);
}

static void skeyup(int key, int x, int y)
{
	app_keyboard(conv_skey(key), false);
}

static void mouse(int bn, int st, int x, int y)
{
	int bidx = bn - GLUT_LEFT_BUTTON;

	if(bidx == 3) {
		wheel(0, 1, x, y);
		return;
	} else if(bidx == 4) {
		wheel(0, -1, x, y);
		return;
	}

	app_mouse_button(bidx, st == GLUT_DOWN, x, y);
}

static void motion(int x, int y)
{
	app_mouse_motion(x, y);
}

static void passive_motion(int x, int y)
{
	app_passive_mouse_motion(x, y);
}

static void wheel(int wheel, int dir, int x, int y)
{
	app_mouse_wheel(dir);
}
