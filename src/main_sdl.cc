#include <stdio.h>
#include <SDL2/SDL.h>
#include "opengl.h"
#include "app.h"

static void process_event(SDL_Event *ev);
static void proc_modkeys();
static int translate_keysym(SDL_Keycode sym);


static SDL_Window *win;
static SDL_GLContext ctx;
static bool quit;

static unsigned int modkeys;
static bool redisp_pending = true;
static bool track_mouse = false;
static int scale_factor = 1;

static long start_time;


int main(int argc, char **argv)
{
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		fprintf(stderr, "failed to initialize SDL\n");
		return 1;
	}

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);
#ifndef NDEBUG
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

	int defpos = SDL_WINDOWPOS_UNDEFINED;
	unsigned int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;

	if(!(win = SDL_CreateWindow("rototool", defpos, defpos, 1024, 640, flags))) {
		fprintf(stderr, "failed to create window\n");
		SDL_Quit();
		return 1;
	}

	if(!(ctx = SDL_GL_CreateContext(win))) {
		fprintf(stderr, "failed to create OpenGL context\n");
		SDL_Quit();
		return 1;
	}
	SDL_GL_GetDrawableSize(win, &win_width, &win_height);
	win_aspect = (float)win_width / (float)win_height;

	start_time = SDL_GetTicks();

	if(!app_init(argc, argv)) {
		return 1;
	}
	app_reshape(win_width, win_height);

	while(!quit) {
		SDL_Event ev;

		if(redisp_pending) {
			while(SDL_PollEvent(&ev)) {
				process_event(&ev);
				if(quit) goto break_evloop;
			}
		} else {
			SDL_WaitEvent(&ev);
			do {
				process_event(&ev);
				if(quit) goto break_evloop;
			} while(SDL_PollEvent(&ev));
		}

		if(redisp_pending) {
			redisp_pending = false;
			app_display();
			SDL_GL_SwapWindow(win);
		}
	}
break_evloop:

	app_shutdown();
	SDL_Quit();
	return 0;
}

void app_quit()
{
	quit = true;
}

void app_redraw()
{
	redisp_pending = true;
}

void app_track_mouse(bool enable)
{
	track_mouse = enable;
}

long app_get_msec()
{
	return SDL_GetTicks() - start_time;
}

int app_mouse_x() {
	int ret;
	SDL_GetMouseState(&ret, nullptr);
	return ret;
}

int app_mouse_y() {
	int ret;
	SDL_GetMouseState(nullptr, &ret);
	return ret;
}

unsigned int app_get_modifiers()
{
	return modkeys;
}


static void process_event(SDL_Event *ev)
{
	int key, bidx;
	bool press;

	switch(ev->type) {
	case SDL_QUIT:
		quit = true;
		break;

	case SDL_KEYDOWN:
	case SDL_KEYUP:
		proc_modkeys();
		if((key = translate_keysym(ev->key.keysym.sym)) != -1) {
			app_keyboard(key, ev->key.state == SDL_PRESSED);
		}
		break;

	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		proc_modkeys();
		bidx = ev->button.button - SDL_BUTTON_LEFT;
		press = ev->button.state == SDL_PRESSED;
		app_mouse_button(bidx, press, ev->button.x * scale_factor, ev->button.y * scale_factor);
		break;

	case SDL_MOUSEMOTION:
		if(ev->motion.state || track_mouse) {
			app_mouse_motion(ev->motion.x * scale_factor, ev->motion.y * scale_factor);
		} else {
			app_passive_mouse_motion(ev->motion.x * scale_factor, ev->motion.y * scale_factor);
		}
		break;

	case SDL_MOUSEWHEEL:
		app_mouse_wheel(ev->wheel.y);
		break;

	case SDL_WINDOWEVENT:
		if(ev->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
			SDL_GL_GetDrawableSize(win, &win_width, &win_height);
			win_aspect = (float)win_width / (float)win_height;
			scale_factor = win_width / ev->window.data1;
			app_reshape(win_width, win_height);
			redisp_pending = true;
		}
		break;
	}
}

static void proc_modkeys()
{
	modkeys = 0;
	SDL_Keymod sdlmod = SDL_GetModState();
	if(sdlmod & KMOD_SHIFT) {
		modkeys |= MODKEY_SHIFT;
	}
	if(sdlmod & KMOD_ALT) {
		modkeys |= MODKEY_ALT;
	}
	if(sdlmod & KMOD_CTRL) {
		modkeys |= MODKEY_CTRL;
	}
}

static int translate_keysym(SDL_Keycode sym)
{
	switch(sym) {
	case SDLK_RETURN:
		return '\n';
	case SDLK_DELETE:
		return KEY_DEL;
	case SDLK_LEFT:
		return KEY_LEFT;
	case SDLK_RIGHT:
		return KEY_RIGHT;
	case SDLK_UP:
		return KEY_UP;
	case SDLK_DOWN:
		return KEY_DOWN;
	case SDLK_PAGEUP:
		return KEY_PGUP;
	case SDLK_PAGEDOWN:
		return KEY_PGDOWN;
	case SDLK_HOME:
		return KEY_HOME;
	case SDLK_END:
		return KEY_END;
	case SDLK_INSERT:
		return KEY_INSERT;
	case SDLK_LSHIFT:
	case SDLK_RSHIFT:
		return KEY_SHIFT;
	case SDLK_LCTRL:
	case SDLK_RCTRL:
		return KEY_CTRL;
	case SDLK_LALT:
	case SDLK_RALT:
		return KEY_ALT;
	default:
		break;
	}

	if(sym < 127) {
		return sym;
	}
	if(sym >= SDLK_F1 && sym <= SDLK_F12) {
		return KEY_F1 + sym - SDLK_F1;
	}
	return -1;
}
