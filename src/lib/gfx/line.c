#include "line.h"

#define ABS(a) (((a) < 0) ? -(a) : (a))

static void
swap(int *a, int *b) {
	int temp = *a;

	*a = *b;
	*b = temp;
}

void
gfx_line_draw(FB_Surface *sfc, int x1, int y1, int x2, int y2, FB_Color *c) {
	int tag, dx, dy, tx, ty, inc1, inc2, d, curx, cury;

	fb_set_pixel(sfc, x1, y1, c);

	if ((x1 == x2) && (y1 == y2)) return;

	tag = 0;
	dx  = ABS(x2 - x1);
	dy  = ABS(y2 - y1);

	if (dx < dy) {
		tag = 1;
		swap(&x1, &y1);
		swap(&x2, &y2);
		swap(&dx, &dy);
	}

	tx   = (x2 - x1) > 0 ? 1 : -1;
	ty   = (y2 - y1) > 0 ? 1 : -1;
	curx = x1;
	cury = y1;
	inc1 = 2 * dy;
	inc2 = 2 * (dy - dx);
	d    = inc1 - dx;

	while (curx != x2) {
		if (d < 0) {
			d += inc1;
		} else {
			cury += ty;
			d += inc2;
		}

		//if (tag) {
			fb_set_pixel(sfc, curx, cury, c);
		//} else {
		//	fb_set_pixel(sfc, curx, cury, fg);
		//}

		curx += tx;
	}
}
