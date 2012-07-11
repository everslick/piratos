#include <lib/gfx/line.h>

#include "draw.h"

#define BOX_HIGH 60
#define BOX_WIDE 60

#define MID_HIGH (BOX_HIGH/2)
#define MID_WIDE (BOX_WIDE/2)

#define SCALE_X(_N_,_W_) _N_ = (_N_ * (_W_ - 1)) / (BOX_WIDE - 1)
#define SCALE_Y(_N_,_H_) _N_ = (_N_ * (_H_ - 1)) / (BOX_HIGH - 1)

#define SEG(_X0_,_Y0_,_X1_,_Y1_) _X0_, _Y0_, _X1_, _Y1_

#define ABS(a) (((a) < 0) ? -(a) : (a))

#define SetColor(_C_,_R_,_G_,_B_) _C_.r=_R_;_C_.g=_G_;_C_.b=_B_;

static const short diamond[] = {
	SEG(  MID_WIDE,     BOX_HIGH/4, 3*BOX_WIDE/4,   MID_HIGH),
	SEG(3*BOX_WIDE/4,   MID_WIDE,     MID_WIDE,   3*BOX_HIGH/4),
	SEG(  MID_WIDE,   3*BOX_HIGH/4,   BOX_WIDE/4,   MID_HIGH),
	SEG(  BOX_WIDE/4,   MID_HIGH,     MID_WIDE,     BOX_HIGH/4),
	SEG(  MID_WIDE,     BOX_HIGH/3, 2*BOX_WIDE/3,   MID_HIGH),
	SEG(2*BOX_WIDE/3,   MID_WIDE,     MID_WIDE,   2*BOX_HIGH/3),
	SEG(  MID_WIDE,   2*BOX_HIGH/3,   BOX_WIDE/3,   MID_HIGH),
	SEG(  BOX_WIDE/3,   MID_HIGH,     MID_WIDE,     BOX_HIGH/3),
	SEG(  BOX_WIDE/4,   MID_HIGH,   3*BOX_WIDE/4,   MID_HIGH),
	SEG(  MID_WIDE,     BOX_HIGH/4,   MID_WIDE,   3*BOX_HIGH/4),
	-1
};

static const short degrees[] = {
	SEG(  MID_WIDE,     BOX_HIGH/4, 2*BOX_WIDE/3, 3*BOX_HIGH/8),
	SEG(2*BOX_WIDE/3, 3*BOX_HIGH/8,   MID_WIDE,     MID_HIGH),
	SEG(  MID_WIDE,     MID_HIGH,     BOX_WIDE/3, 3*BOX_HIGH/8),
	SEG(  BOX_WIDE/3, 3*BOX_HIGH/8,   MID_WIDE,     BOX_HIGH/4),
	-1
};

static const short lower_right_corner[] = {
	SEG(0,        MID_HIGH, MID_WIDE, MID_HIGH),
	SEG(MID_WIDE, MID_HIGH, MID_WIDE, 0),
	-1
};

static const short upper_right_corner[] = {
	SEG(0,        MID_HIGH, MID_WIDE, MID_HIGH),
	SEG(MID_WIDE, MID_HIGH, MID_WIDE, BOX_HIGH),
	-1
};

static const short upper_left_corner[] = {
	SEG(MID_WIDE, MID_HIGH, BOX_WIDE, MID_HIGH),
	SEG(MID_WIDE, MID_HIGH, MID_WIDE, BOX_HIGH),
	-1
};

static const short lower_left_corner[] = {
	SEG(MID_WIDE, 0,        MID_WIDE, MID_HIGH),
	SEG(MID_WIDE, MID_WIDE, BOX_WIDE, MID_HIGH),
	-1
};

static const short cross[] = {
	SEG(0,        MID_HIGH, BOX_WIDE, MID_HIGH),
	SEG(MID_WIDE, 0,        MID_WIDE, BOX_HIGH),
	-1
};

static const short scan_line_1[] = {
	SEG(0, 0, BOX_WIDE, 0),
	-1
};

static const short scan_line_3[] = {
	SEG(0, BOX_HIGH/4, BOX_WIDE, BOX_HIGH/4),
	-1
};

static const short scan_line_7[] = {
	SEG(0, MID_HIGH, BOX_WIDE, MID_HIGH),
	-1
};

static const short scan_line_9[] = {
	SEG(0, 3*BOX_HIGH/4, BOX_WIDE, 3*BOX_HIGH/4),
	-1
};

static const short horizontal_line[] = {
	SEG(0, BOX_HIGH, BOX_WIDE, BOX_HIGH),
	-1
};

static const short left_tee[] = {
	SEG(MID_WIDE, 0,        MID_WIDE, BOX_HIGH),
	SEG(MID_WIDE, MID_HIGH, BOX_WIDE, MID_HIGH),
	-1
};

static const short right_tee[] = {
	SEG(MID_WIDE, 0,        MID_WIDE, BOX_HIGH),
	SEG(MID_WIDE, MID_HIGH, 0,        MID_HIGH),
	-1
};

static const short bottom_tee[] = {
	SEG(0,        MID_HIGH, BOX_WIDE, MID_HIGH),
	SEG(MID_WIDE, 0,        MID_WIDE, MID_HIGH),
	-1
};

static const short top_tee[] = {
	SEG(0,        MID_HIGH, BOX_WIDE, MID_HIGH),
	SEG(MID_WIDE, MID_HIGH, MID_WIDE, BOX_HIGH),
	-1
};

static const short vertical_line[] = {
	SEG(MID_WIDE, 0, MID_WIDE, BOX_HIGH),
	-1
};

static const short less_than_or_equal[] = {
	SEG(5*BOX_WIDE/6,   BOX_HIGH/6,   BOX_WIDE/5,   MID_HIGH),
	SEG(5*BOX_WIDE/6, 5*BOX_HIGH/6,   BOX_WIDE/5,   MID_HIGH),
	SEG(  BOX_WIDE/6, 5*BOX_HIGH/6, 5*BOX_WIDE/6, 5*BOX_HIGH/6),
	-1
};

static const short greater_than_or_equal[] = {
	SEG(  BOX_WIDE/6,   BOX_HIGH/6, 5*BOX_WIDE/6,   MID_HIGH),
	SEG(  BOX_WIDE/6, 5*BOX_HIGH/6, 5*BOX_WIDE/6,   MID_HIGH),
	SEG(  BOX_WIDE/6, 5*BOX_HIGH/6, 5*BOX_WIDE/6, 5*BOX_HIGH/6),
	-1
};

static const short *lines[] = {
	0,                      /* 00 */
	diamond,                /* 01 */
	0,                      /* 02 */
	0,                      /* 03 */
	0,                      /* 04 */
	0,                      /* 05 */
	0,                      /* 06 */
	degrees,                /* 07 */
	0,                      /* 08 */
	0,                      /* 09 */
	0,                      /* 0A */
	lower_right_corner,     /* 0B */
	upper_right_corner,     /* 0C */
	upper_left_corner,      /* 0D */
	lower_left_corner,      /* 0E */
	cross,                  /* 0F */
	scan_line_1,            /* 10 */
	scan_line_3,            /* 11 */
	scan_line_7,            /* 12 */
	scan_line_9,            /* 13 */
	horizontal_line,        /* 14 */
	left_tee,               /* 15 */
	right_tee,              /* 16 */
	bottom_tee,             /* 17 */
	top_tee,                /* 18 */
	vertical_line,          /* 19 */
	less_than_or_equal,     /* 1A */
	greater_than_or_equal,  /* 1B */
	0,                      /* 1C */
	0,                      /* 1D */
	0,                      /* 1E */
	0,                      /* 1F */
};

static FB_Color *
AnsiColor(int ansi) {
	static FB_Color color = { 0xFF, 0xFF, 0xFF, 0xFF }; // white

	switch (ansi) {
		case 0: SetColor(color, 0x00, 0x00, 0x00); break; // black
		case 1: SetColor(color, 0xFF, 0x10, 0x10); break; // red
		case 2: SetColor(color, 0x00, 0xFF, 0x00); break; // green
		case 3: SetColor(color, 0xF0, 0xFF, 0x00); break; // yellow
		case 4: SetColor(color, 0x60, 0x60, 0xFF); break; // blue
		case 5: SetColor(color, 0xB0, 0x00, 0xFC); break; // magenta
		case 6: SetColor(color, 0x00, 0xF0, 0xF0); break; // cyan
	}

	return (&color);
}

void
DrawLine(FB_Surface *sfc, int x1, int y1, int x2, int y2, int c) {
	gfx_line_draw(sfc, x1, y1, x2, y2, AnsiColor(c));
}

void
DrawGlyph(FB_Surface *sfc,
			 int x, int y,
			 unsigned char c,
			 FB_Surface *font[],
			 int fg, int bg) {

	int w = font[0]->width / 95, h = font[0]->height;
	FB_Rectangle d = { x, y, w, h };
	FB_Rectangle s = { x, 0, w, h };
	const short *p;

	if (bg != -1) fb_fill(sfc, &d, AnsiColor(bg));

	if (c >= 32) {
		s.x = (c - 32) * w;
		fb_blit(font[fg], &s, sfc, &d, 0);
	} else {
		if (c < (int)(sizeof (lines) / sizeof (lines[0])) && (p=lines[c])!=0) {
			int coord[4], n = 0;

			while (*p >= 0) {
				coord[n++] = *p++;

				if (n == 4) {
					SCALE_X(coord[0], w); SCALE_Y(coord[1], h);
					SCALE_X(coord[2], w); SCALE_Y(coord[3], h);

					DrawLine(sfc, x + coord[0], y + coord[1],
					              x + coord[2], y + coord[3], fg);
					n = 0;
				}
			}
		}
	}
}
