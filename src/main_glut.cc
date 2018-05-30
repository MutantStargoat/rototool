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
}

static void keydown(unsigned char key, int x, int y)
{
	app_keyboard(key, true);
}

static void keyup(unsigned char key, int x, int y)
{
	app_keyboard(key, false);
}

static void skeydown(int key, int x, int y)
{
	app_keyboard(key, true);
}

static void skeyup(int key, int x, int y)
{
	app_keyboard(key, false);
}

static void mouse(int bn, int st, int x, int y)
{
	app_mouse_button(bn - GLUT_LEFT_BUTTON, st == GLUT_DOWN, x, y);
}

static void motion(int x, int y)
{
	app_mouse_motion(x, y);
}
