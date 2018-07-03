#ifndef UTK_CALLBACKS_H_
#define UTK_CALLBACKS_H_

void utk_color(int r, int g, int b, int a);
void utk_clip(int x1, int y1, int x2, int y2);
void utk_image(int x, int y, const void *pixels, int xsz, int ysz);
void utk_rect(int x1, int y1, int x2, int y2);
void utk_line(int x1, int y1, int x2, int y2, int width);
void utk_text(int x, int y, const char *txt, int sz);
int utk_text_spacing();
int utk_text_width(const char *txt, int sz);

#endif	// UTK_CALLBACKS_H_
