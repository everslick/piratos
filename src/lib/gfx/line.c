#include "line.h"

#define ABS(_A_) (((_A_)<0) ? -(_A_) : (_A_))
#define SWAP(_A_,_B_) { _A_^=_B_; _B_^=_A_; _A_^=_B_; }

void
gfx_line_draw(FB_Surface *sfc, int x0, int y0, int x1, int y1, FB_Color *c) {
	int dx = x1 - x0;
	int dy = y1 - y0;
	int stepx, stepy;
	int fraction;

	if (dx < 0) {
		dx = -dx;
		stepx = -1;
	} else
		stepx = 1;

	if (dy < 0) {
		dy = -dy;
		stepy = -1;
	} else
		stepy = 1;

	dy <<= 1;
	dx <<= 1;

	fb_set_pixel(sfc, x0, y0, c);

	if (dx > dy) {
		fraction = dy - (dx >> 1);

		while (x0 != x1) {
			if (fraction >= 0) {
				y0 += stepy;
				fraction -= dx;
			}

			x0 += stepx;
			fraction += dy;

			fb_set_pixel(sfc, x0, y0, c);
		}
	} else {
		fraction = dx - (dy >> 1);

		while (y0 != y1) {
			if (fraction >= 0) {
				x0 += stepx;
				fraction -= dy;
			}

			y0 += stepy;
			fraction += dx;

			fb_set_pixel(sfc, x0, y0, c);
		}
	}
}
