#ifndef _GFX_GLYPH_H_
#define _GFX_GLYPH_H_

#include <stdarg.h>

#include <kernel/hal/fb/fb.h>

#include <lib/gfx/bitmap.h>

#ifdef __cplusplus
extern "C" {
#endif

FB_Surface *gfx_glyph_load(GFX_Bitmap *bitmap, FB_Color *c);

void gfx_glyph_draw(FB_Surface *sfc, FB_Surface *font, int x, int y, unsigned char c);

void gfx_glyph_string(FB_Surface *sfc, FB_Surface *font, int x, int y, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif // _GFX_GLYPH_H_
