#include "bitmap.h"

FB_Surface *
gfx_bitmap_load(GFX_Bitmap *bitmap) {
	int bpp = bitmap->bytes_per_pixel;
	FB_Surface *sfc;
	FB_Color c;

	sfc = fb_create_surface(bitmap->width, bitmap->height, FB_FORMAT_BEST);

	for (int y=0; y<sfc->height; y++) {
		for (int x=0; x<sfc->width; x++) {
			int idx = y * sfc->width * bpp + x * bpp;
			c.a = 0xff;

			switch (bpp) {
				case 4:
					c.a = bitmap->pixel_data[idx + 3];
				// fall through!

				case 3:
					c.b = bitmap->pixel_data[idx + 2];
					c.g = bitmap->pixel_data[idx + 1];
					c.r = bitmap->pixel_data[idx + 0];
				break;

				case 1:
					c.r = c.g = c.b = bitmap->pixel_data[idx];
				break;
			}

			fb_set_pixel(sfc, x, y, &c);
		}
	}

	return (sfc);
}
