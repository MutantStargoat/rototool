#include "opengl.h"
#include "ui.h"
#include "app.h"
#include "ubertk.h"
#include "utk_callbacks.h"
#include "dtx/drawtext.h"

static dtx_font *font;
static utk::Container *utkroot;
static utk::Window *win_colsel;
static utk::ColorBox *colbox;
static utk::HueBox *huebox;

static void hue_change_callback(utk::Event *ev, void *cls);

bool init_ui()
{
	if(!(font = dtx_open_font_glyphmap("data/font.glyphmap"))) {
		fprintf(stderr, "failed to load glyphmap\n");
		return false;
	}

	utk::gfx::color = utk_color;
	utk::gfx::clip = utk_clip;
	utk::gfx::image = utk_image;
	utk::gfx::rect = utk_rect;
	utk::gfx::line = utk_line;
	utk::gfx::text = utk_text;
	utk::gfx::text_spacing = utk_text_spacing;
	utk::gfx::text_width = utk_text_width;

	utkroot = utk::init(0, 0);	// dummy size, will change later

	win_colsel = utk::create_window(utkroot, 5, 5, 10, 10, "Color");
	win_colsel->show();

	utk::VBox *vbox = utk::create_vbox(win_colsel);
	colbox = utk::create_colorbox(vbox);
	huebox = utk::create_huebox(vbox, hue_change_callback, colbox);

	win_colsel->set_size(vbox->get_size() + utk::IVec2(8, 8));

	return true;
}

void destroy_ui()
{
	dtx_close_font(font);
}

void draw_ui()
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, win_width, win_height, 0, -1, 1);

	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	utk::draw(utkroot);

	glPopAttrib();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void ui_reshape(int x, int y)
{
	utkroot->set_size(x, y);
}

bool ui_keyboard(int key, bool pressed)
{
	/*
	utk::KeyboardEvent ev;
	ev.key = key;
	ev.pressed = pressed;
	utk::event(&ev);
	app_redraw();
	*/
	return false;
}

bool ui_mouse_motion(int x, int y)
{
	static int prev_x, prev_y;

	utk::MMotionEvent ev;
	ev.x = x;
	ev.y = y;
	utk::event(&ev);

	if(utkroot->get_child_at(x, y) != utkroot ||
			utkroot->get_child_at(prev_x, prev_y) != utkroot) {
		prev_x = x;
		prev_y = y;
		app_redraw();
		return true;
	}
	prev_x = x;
	prev_y = y;

	return false;
}

bool ui_mouse_button(int bn, bool pressed, int x, int y)
{
	utk::MButtonEvent ev;
	ev.button = bn;
	ev.pressed = pressed;
	ev.x = x;
	ev.y = y;
	utk::event(&ev);
	app_redraw();

	if(utkroot->get_child_at(x, y) != utkroot) {
		return true;
	}
	return false;
}

static void hue_change_callback(utk::Event *ev, void *cls)
{
	utk::ColorBox *cbox = (utk::ColorBox*)cls;
	utk::HueBox *huebox = (utk::HueBox*)ev->widget;
	cbox->set_h(huebox->get_h());
}
