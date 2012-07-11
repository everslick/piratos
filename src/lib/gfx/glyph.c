#include "glyph.h"

void
gfx_glyph(FB_Surface *sfc, int x, int y, unsigned char c, FB_Surface *font, FB_Color *bg) {
	int w = font->width / 95, h = font->height;
	FB_Rectangle d = { x, y, w, h };
	FB_Rectangle s = { x, 0, w, h };

	if (bg) fb_fill(sfc, &d, bg);

	if (c >= 32) {
		s.x = (c - 32) * w;
		fb_blit(font, &s, sfc, &d, 0);
	}
}
