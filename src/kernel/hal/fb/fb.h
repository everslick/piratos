#ifndef _FB_H_
#define _FB_H_

typedef enum {
	FB_FORMAT_ARGB8888,
	FB_FORMAT_RGB888,
	FB_FORMAT_RGB565,
	FB_FORMAT_ARGB4444,
	FB_FORMAT_ARGB1555,
	FB_FORMAT_RGB332
} FbFormat;

int fb_init(void);
int fb_fini(void);
int fb_mode(int mode, FbFormat format);

#endif // _fb_H_
