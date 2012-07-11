#include <stdio.h>

#include "glyph.h"

FB_Surface *
gfx_glyph_load(GFX_Bitmap *bitmap, FB_Color *c) {
	int bpp = bitmap->bytes_per_pixel;
	FB_Surface *sfc;

	sfc = fb_create_surface(bitmap->width, bitmap->height, FB_FORMAT_BEST);

	for (int y=0; y<sfc->height; y++) {
		for (int x=0; x<sfc->width; x++) {
			int idx = y * sfc->width * bpp + x * bpp;

			switch (bpp) {
				case 1:
					// grayscale image
					c->a = bitmap->pixel_data[idx + 0];
				break;

				case 4:
					// RGBA image
					c->a = bitmap->pixel_data[idx + 3];
				break;
			}

			fb_set_pixel(sfc, x, y, c);
		}
	}

	return (sfc);
}

void
gfx_glyph_draw(FB_Surface *sfc, FB_Surface *font, int x, int y, unsigned char c) {
	int w = font->width / 95, h = font->height;
	FB_Rectangle d = { x, y, w, h };
	FB_Rectangle s = { x, 0, w, h };

	if (c > 32) {
		s.x = (c - 32) * w;
		fb_blit(font, &s, sfc, &d, 0);
	}
}

void
gfx_glyph_string(FB_Surface *sfc, FB_Surface *font, int x, int y, const char *fmt, ...) {
	char buffer[512];
	va_list ap;
	int n;

	va_start(ap, fmt);
	n = vsnprintf(buffer, sizeof (buffer), fmt, ap);
	va_end(ap);

	for (int i=0; i<n; i++) {
		gfx_glyph_draw(sfc, font, x, y, buffer[i]);
		x += font->width / 95;
	}
}
