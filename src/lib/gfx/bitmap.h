#ifndef _GFX_BITMAP_H_
#define _GFX_BITMAP_H_

#include <kernel/hal/fb/fb.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct GFX_Bitmap {
	unsigned int  width;
	unsigned int  height;
	unsigned int  bytes_per_pixel; /* 3:RGB, 4:RGBA */ 
	unsigned char pixel_data[];
} GFX_Bitmap;

FB_Surface *gfx_bitmap_load(GFX_Bitmap *bitmap);

#ifdef __cplusplus
}
#endif

#endif // _GFX_BITMAP_H_
