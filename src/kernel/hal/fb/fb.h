#ifndef _FB_H_
#define _FB_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	FB_MODE_BEST,
	FB_MODE_640x350,
	FB_MODE_640x480,
	FB_MODE_800x600,
	FB_MODE_1024x768,
	FB_MODE_1280x720,
	FB_MODE_1280x768,
	FB_MODE_1280x800,
	FB_MODE_1280x1024,
	FB_MODE_1366x768,
	FB_MODE_1400x1050,
	FB_MODE_1600x1200,
	FB_MODE_1680x1050,
	FB_MODE_1920x1080,
	FB_MODE_1920x1200
} FB_Mode;

typedef enum {
	FB_FORMAT_BEST,
	FB_FORMAT_RGBA8888,
	FB_FORMAT_RGB888,
	FB_FORMAT_RGB565,
	FB_FORMAT_RGBA4444,
	FB_FORMAT_RGBA5551,
	FB_FORMAT_RGB332
} FB_Format;

typedef struct FB_Surface {
	int width;
	int height;
	int pitch;
	int flags;
	FB_Format format;
	void *pixels;
	void *user;
} FB_Surface;

typedef struct FB_Rectangle {
	int x, y, w, h;
} FB_Rectangle;

typedef struct FB_Color {
	unsigned char r, g, b, a;
} FB_Color;

int fb_init(void);
int fb_fini(void);

int fb_mode(FB_Mode mode, FB_Format format);

void fb_blit(FB_Surface *ss, FB_Rectangle *sr, FB_Surface *ds, FB_Rectangle *dr, FB_Rectangle *ud);

void fb_flip(FB_Rectangle *rect);

FB_Surface *fb_create_surface(int w, int h, FB_Format format);
void        fb_release_surface(FB_Surface *surface);

void fb_fill(FB_Surface *surface, FB_Rectangle *rect, FB_Color *color);

void fb_set_pixel(FB_Surface *surface, int x, int y, FB_Color *color);
void fb_get_pixel(FB_Surface *surface, int x, int y, FB_Color *color);

#ifdef __cplusplus
}
#endif

#endif // _fb_H_
