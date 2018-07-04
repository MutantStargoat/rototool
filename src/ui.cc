#include <vector>
#include "opengl.h"
#include "ui.h"
#include "app.h"
#include "ubertk.h"
#include "utk_callbacks.h"
#include "dtx/drawtext.h"
#include "pal.h"

struct PalCell {
	int x, y;
	int w, h;
};

static dtx_font *font;
static utk::Container *utkroot;
static utk::Window *win_colsel;
static utk::ColorBox *colbox;
static utk::HueBox *huebox;

static float palwin_width = 0.05;
static PalCell *pcells;
static int cur_pal_cell = -1;

static void init_palette_ui();
static void destroy_palette_ui();
static void draw_palette_ui();

static void hue_change(utk::Event *ev, void *cls);
static void col_change(utk::Event *ev, void *cls);
static void bn_close_click(utk::Event *ev, void *cls);

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

	utk::VBox *vbox = utk::create_vbox(win_colsel);
	colbox = utk::create_colorbox(vbox, col_change, 0);
	huebox = utk::create_huebox(vbox, hue_change, colbox);
	utk::create_button(vbox, "Close", bn_close_click, 0);

	win_colsel->set_size(vbox->get_size() + utk::IVec2(8, 8));

	init_palette_ui();
	return true;
}

void destroy_ui()
{
	dtx_close_font(font);
	destroy_palette_ui();
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

	draw_palette_ui();

	utk::draw(utkroot);

	glPopAttrib();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

static void init_palette_ui()
{
	static const float pad = 4.0f;

	float cell_width = win_width * palwin_width - 2.0f * pad;
	float cell_height = std::min(cell_width, (win_height - (2.0f + palette_size) * pad) / palette_size);
	float x = win_width * (1.0f - palwin_width) + pad;
	float y = pad;

	pcells = new PalCell[palette_size];
	for(int i=0; i<palette_size; i++) {
		pcells[i].x = x;
		pcells[i].y = y;
		pcells[i].w = cell_width;
		pcells[i].h = cell_height;
		y += cell_height + pad;
	}
}

static void destroy_palette_ui()
{
	delete [] pcells;
}

static void draw_palette_ui()
{
	glPushAttrib(GL_LINE_BIT);
	glLineWidth(2);

	float x = win_width * (1.0 - palwin_width);
	glBegin(GL_QUADS);
	glColor3f(0.3, 0.3, 0.3);
	glVertex2f(x, win_height);
	glVertex2f(win_width, win_height);
	glVertex2f(win_width, 0);
	glVertex2f(x, 0);
	glEnd();

	glBegin(GL_QUADS);
	for(int i=0; i<2; i++) {
		float offs = i == 0 ? 2.0f : 0.0f;
		for(int j=0; j<palette_size; j++) {
			if(i == 0) {
				glColor3f(0, 0, 0);
			} else {
				glColor3f(palette[j].x, palette[j].y, palette[j].z);
			}
			glVertex2f(pcells[j].x + offs, pcells[j].y + offs);
			glVertex2f(pcells[j].x + offs, pcells[j].y + pcells[j].h - offs);
			glVertex2f(pcells[j].x + pcells[j].w - offs, pcells[j].y + pcells[j].h - offs);
			glVertex2f(pcells[j].x + pcells[j].w - offs, pcells[j].y + offs);
		}
	}
	glEnd();

	glPopAttrib();
}

void ui_reshape(int x, int y)
{
	utkroot->set_size(x, y);

	destroy_palette_ui();
	init_palette_ui();
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

	if(x > win_width * (1.0 - palwin_width)) {
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

	if(x > win_width * (1.0 - palwin_width)) {
		if(!pressed) {
			for(int i=0; i<palette_size; i++) {
				if(x >= pcells[i].x && x < pcells[i].x + pcells[i].w &&
						y >= pcells[i].y && y < pcells[i].y + pcells[i].h) {
					cur_pal_cell = i;
					float h, s, v;
					utk::rgb_to_hsv(palette[i].x, palette[i].y, palette[i].z, &h, &s, &v);
					huebox->set_h(h);
					colbox->set_color_hsv(h, s, v);
					win_colsel->show();
					break;
				}
			}
		}
		return true;
	}
	return false;
}

static void hue_change(utk::Event *ev, void *cls)
{
	utk::ColorBox *cbox = (utk::ColorBox*)cls;
	utk::HueBox *huebox = (utk::HueBox*)ev->widget;
	cbox->set_h(huebox->get_h());
}

static void col_change(utk::Event *ev, void *cls)
{
	utk::ColorBox *cbox = (utk::ColorBox*)ev->widget;
	if(cur_pal_cell != -1) {
		const utk::Color &c = cbox->get_color();
		palette[cur_pal_cell].x = c.r / 255.0f;
		palette[cur_pal_cell].y = c.g / 255.0f;
		palette[cur_pal_cell].z = c.b / 255.0f;
	}
}

static void bn_close_click(utk::Event *ev, void *cls)
{
	win_colsel->hide();
}
