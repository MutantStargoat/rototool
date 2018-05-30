// ImgConvert.h

#ifndef _IMG_CONVERT_H_
#define _IMG_CONVERT_H_

bool FlattenYUV(unsigned char *dst, int dst_line_size, 
	const unsigned char *y, int y_line_size, 
	const unsigned char *u, int u_line_size,
	const unsigned char *v, int v_line_size,
	int width, int height);

bool ConvertYUV(unsigned char *dst, int dst_line_size, 
	const unsigned char *y, int y_line_size, 
	const unsigned char *u, int u_line_size,
	const unsigned char *v, int v_line_size,
	int width, int height);

#endif // ndef _IMG_CONVERT_H_
