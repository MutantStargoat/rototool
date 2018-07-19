#include "app.h"
#include "utk_preview.h"

PreviewImage::PreviewImage()
{
	upd = true;
	scale = 1.0f;
}

void PreviewImage::set_scale(float s)
{
	scale = s;
}

void PreviewImage::draw() const
{
	((PreviewImage*)this)->invalidate();
	Image::draw();
}

void PreviewImage::update()
{
	if(!vfchain.get_tap(VF_PREVIEW_TAP)) {
		VideoFilterNode *ctap = vfchain.get_tap(VF_COLOR_TAP);
		if(!ctap) {
			return;
		}
		vfchain.set_tap(VF_PREVIEW_TAP, ctap);
	}

	VideoFrame tmp = {0, 0, 0};
	VideoFrame *frm = vfchain.get_frame(VF_PREVIEW_TAP);
	if(scale != 1.0f) {
		scale_video_frame(&tmp, frm, scale);
		frm = &tmp;
	}

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

	memcpy(pixels, frm->pixels, img_w * img_h * 4);

	delete [] tmp.pixels;
}
