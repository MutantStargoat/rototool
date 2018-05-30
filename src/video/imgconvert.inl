(unsigned char *dst, int dst_line_size, 
	const unsigned char *y, int y_line_size, 
	const unsigned char *u, int u_line_size,
	const unsigned char *v, int v_line_size,
	int width, int height) 
{

	if (width % 2) width--;
	if (height % 2) height--;

	const unsigned char *y_line_odd = y;
	const unsigned char *y_line_even = y + y_line_size;

	const unsigned char *u_line = u;
	const unsigned char *v_line = v;

	unsigned char *dst_line_odd = dst;
	unsigned char *dst_line_even = dst + dst_line_size;

	for (int j=0; j<height; j+=2) {
		// fetch pixel pointers
		const unsigned char *y_pixel_odd = y_line_odd;
		const unsigned char *y_pixel_even = y_line_even;
		const unsigned char *u_pixel = u_line;
		const unsigned char *v_pixel = v_line;
		unsigned char *dst_pixel_odd = dst_line_odd;
		unsigned char *dst_pixel_even = dst_line_even;

		for (int i=0; i<width; i+=2) {
			unsigned char u_val = *u_pixel++;
			unsigned char v_val = *v_pixel++;

			// top left
			ConvertYUV(dst_pixel_odd, *y_pixel_odd++, u_val, v_val);
			dst_pixel_odd += 4;

			// top right
			ConvertYUV(dst_pixel_odd, *y_pixel_odd++, u_val, v_val);
			dst_pixel_odd += 4;

			// bottom left
			ConvertYUV(dst_pixel_even, *y_pixel_even++, u_val, v_val);
			dst_pixel_even += 4;

			// bottom right
			ConvertYUV(dst_pixel_even, *y_pixel_even++, u_val, v_val);
			dst_pixel_even += 4;
		}

		// update line pointers
		y_line_odd += 2 * y_line_size;
		y_line_even += 2 * y_line_size;
		u_line += u_line_size;
		v_line += v_line_size;
		dst_line_odd += 2 * dst_line_size;
		dst_line_even += 2 * dst_line_size;
	}
		
	return true;
}