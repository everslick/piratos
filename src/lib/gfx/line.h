#ifndef _GFX_LINE_H_
#define _GFX_LINE_H_

#include <kernel/hal/fb/fb.h>

#ifdef __cplusplus
extern "C" {
#endif

void gfx_line_draw(FB_Surface *sfc, int x1, int y1, int x2, int y2, FB_Color *c);

#ifdef __cplusplus
}
#endif

#endif // _GFX_LINE_H_
