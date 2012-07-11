#ifndef _TERMINAL_DRAW_H_
#define _TERMINAL_DRAW_H_

#include <kernel/hal/fb/fb.h>

#ifdef __cplusplus
extern "C" {
#endif

void DrawLine(FB_Surface *sfc, int x1, int y1, int x2, int y2, int c);

void DrawGlyph(FB_Surface *sfc,
					int x, int y,
					unsigned char c,
					FB_Surface *font[],
					int fg, int bg);

#ifdef __cplusplus
}
#endif

#endif // _TERMINAL_DRAW_H_
