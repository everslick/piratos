#ifndef _GFX_GLYPH_H_
#define _GFX_GLYPH_H_

#include <kernel/hal/fb/fb.h>

#ifdef __cplusplus
extern "C" {
#endif

void gfx_glyph(FB_Surface *sfc, int x, int y, unsigned char c, FB_Surface *font, FB_Color *bg);

#ifdef __cplusplus
}
#endif

#endif // _GFX_GLYPH_H_
