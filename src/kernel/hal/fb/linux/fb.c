#include "SDL.h"

#include "fb.h"

#define FB_MODES 14

typedef struct FbMode {
	int width, height;
	const char *description; 
} FbMode;

static FbMode fb_modes[FB_MODES] = {
	{  640,  350, "EGA"          },
	{  640,  480, "VGA"          },
	{  800,  600, "SVGA"         },
	{ 1024,  768, "XGA"          },
	{ 1280,  720, "720p HDTV"    },
	{ 1280,  768, "WXGA Variant" },
	{ 1280,  800, "WXGA Variant" },
	{ 1280, 1024, "SXGA"         },
	{ 1366,  768, "WXGA Variant" },
	{ 1400, 1050, "SXGA+"        },
	{ 1600, 1200, "UXGA"         },
	{ 1680, 1050, "WXGA+"        },
	{ 1920, 1080, "1080p HDTV"   },
	{ 1920, 1200, "WUXGA"        }
};

static SDL_Surface *screen = NULL;

static int pixel_format   = 0;
static int bits_per_pixel = 0;

int
fb_init(void) {
	/* Initialize SDL */
	SDL_Init(SDL_INIT_VIDEO);
	
	return (1);
}

int
fb_fini(void) {
	return (1);
}

int
fb_mode(int mode, FbFormat format) {
	int bpp = 0;

	switch (format) {
		case FB_FORMAT_RGB888:
			bpp = 24;
		break;
		case FB_FORMAT_RGB565:
		case FB_FORMAT_ARGB4444:
		case FB_FORMAT_ARGB1555:
			bpp = 16;
		break;
		case FB_FORMAT_RGB332:
			bpp = 8;
		break;
		default:
			bpp = 32;
		break;
	}

	/* Initialize the screen / window */
	screen = SDL_SetVideoMode(fb_modes[mode].width, fb_modes[mode].height, bpp, SDL_SWSURFACE);

	return (1);
}
