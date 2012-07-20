#ifndef _TERMINAL_DRAW_H_
#define _TERMINAL_DRAW_H_

#include <kernel/hal/fb/fb.h>

#include <lib/gfx/bitmap.h>

#ifdef __cplusplus
//extern "C" {
#endif

void DrawFont(GFX_Bitmap *bitmap, FB_Surface *font[8]);

void DrawLine(FB_Surface *sfc, int x1, int y1, int x2, int y2, int c);

void DrawGlyph(FB_Surface *sfc,
					FB_Surface *font[],
					int x, int y,
					unsigned char c,
					int fg, int bg);

#ifdef __cplusplus
//}
#endif

#endif // _TERMINAL_DRAW_H_
