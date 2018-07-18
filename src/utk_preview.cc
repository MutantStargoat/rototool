#include "app.h"
#include "utk_preview.h"

#define PREVIEW_TAP		42

PreviewImage::PreviewImage()
{
	upd = true;
}

void PreviewImage::update()
{
	if(!vfchain.get_tap(PREVIEW_TAP)) {
		VideoFilterNode *ctap = vfchain.get_tap(VF_COLOR_TAP);
		if(!ctap) {
			return;
		}
		vfchain.set_tap(PREVIEW_TAP, ctap);
	}

	VideoFrame *frm = vfchain.get_frame(PREVIEW_TAP);
	if(img_w != frm->width || img_h != frm->height) {
		delete [] pixels;
		img_w = frm->width;
		img_h = frm->height;
		pixels = new uint32_t[img_w * img_h];

		set_size(img_w, img_h);

		utk::Window *win = get_window();
		if(win) {
			utk::Widget *c = win->get_child();
			if(c && dynamic_cast<utk::Container*>(c)) {
				utk::Container *cont = (utk::Container*)c;
				cont->layout();

				win->set_size(cont->get_width() + 8, cont->get_height() + 8);
			}
		}
		win->set_pos(win_width - win->get_frame_width(), 0);
	}

	memcpy(pixels, frm, img_w * img_h * 4);
}
