#include "SDL.h"

#include "fb.h"

typedef struct ModeDescriptor {
	int width;
	int height;
	const char *description; 
} ModeDescriptor;

static ModeDescriptor fb_modes[] = {
	{ 1920, 1200, "WUXGA"        },
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

static int fb_pixel_format   = 0;
static int fb_bits_per_pixel = 0;

static void
FB2SDL_Rectangle(FB_Rectangle *in, SDL_Rect *out) {
	if (!in || !out) return;

	out->x = in->x;
	out->y = in->y;
	out->w = in->w;
	out->h = in->h;
}

static void
SDL2FB_Rectangle(SDL_Rect *in, FB_Rectangle *out) {
	if (!in || !out) return;

	out->x = in->x;
	out->y = in->y;
	out->w = in->w;
	out->h = in->h;
}

static void
FB2SDL_Color(FB_Color *in, SDL_Color *out) {
	if (!in || !out) return;

	out->r      = in->r;
	out->g      = in->g;
	out->b      = in->b;
	out->unused = in->a;
}

static void
SDL2FB_Color(SDL_Color *in, FB_Color *out) {
	if (!in || !out) return;

	out->r = in->r;
	out->g = in->g;
	out->b = in->b;
	out->a = in->unused;
}

static int
bits_per_pixel(FB_Format format) {
	switch (format) {
		case FB_FORMAT_BEST:
		case FB_FORMAT_RGBA8888: return (32);
		case FB_FORMAT_RGB888:   return (24);
		case FB_FORMAT_RGB565:
		case FB_FORMAT_RGBA4444:
		case FB_FORMAT_RGBA5551: return (16);
		case FB_FORMAT_RGB332:   return (8);
	}

	return (0);
}

int
fb_init(void) {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE);

	return (0);
}

int
fb_fini(void) {
	//SDL_QuitSubsystem(SDL_INIT_VIDEO);

	SDL_Quit();

	return (0);
}

int
fb_mode(FB_Mode mode, FB_Format format) {
	int w = fb_modes[mode].width;
	int h = fb_modes[mode].height;

	screen = SDL_SetVideoMode(w, h, 0, SDL_SWSURFACE | SDL_ANYFORMAT);

	fb_pixel_format   = format;
	fb_bits_per_pixel = bits_per_pixel(format);

	return (0);
}

void
fb_blit(FB_Surface *ss, FB_Rectangle *sr, FB_Surface *ds, FB_Rectangle *dr, FB_Rectangle *ud) {
	SDL_Surface *sdl_ss = screen;
	SDL_Surface *sdl_ds = screen;
	SDL_Rect sdl_sr, sdl_dr;

	if (ss) sdl_ss = (SDL_Surface *)ss->user;
	if (ds) sdl_ds = (SDL_Surface *)ds->user;

	if (sr) {
		FB2SDL_Rectangle(sr, &sdl_sr);
	} else {
		sdl_sr.x = 0; sdl_sr.w = sdl_ss->w;
		sdl_sr.y = 0; sdl_sr.h = sdl_ss->h;
	}

	if (dr) {
		FB2SDL_Rectangle(dr, &sdl_dr);
	} else {
		sdl_dr.x = 0; sdl_dr.w = sdl_ds->w;
		sdl_dr.y = 0; sdl_dr.h = sdl_ds->h;
	}

	//SDL_SetClipRect(sdl_ds, &sdl_dr);
	SDL_BlitSurface(sdl_ss, &sdl_sr, sdl_ds, &sdl_dr);

	SDL2FB_Rectangle(&sdl_dr, ud);
}

void
fb_flip(FB_Rectangle *rect) {
	if (rect) {
		SDL_UpdateRect(screen, rect->x, rect->y, rect->w, rect->h);
	} else {
		SDL_Flip(screen);
	}
}

FB_Surface *
fb_create_surface(int w, int h, FB_Format format) {
	SDL_Surface *sdl_surface, *tmp_surface;
	int bpp = bits_per_pixel(format);
	FB_Surface *fb_surface;

	tmp_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, bpp, 0, 0, 0, 0);
	sdl_surface = SDL_DisplayFormatAlpha(tmp_surface);
	SDL_FreeSurface(tmp_surface);

	fb_surface = (FB_Surface *)malloc(sizeof (FB_Surface));

	fb_surface->format = format;
	fb_surface->flags  = sdl_surface->flags;
	fb_surface->width  = sdl_surface->w;
	fb_surface->height = sdl_surface->h;
	fb_surface->pitch  = sdl_surface->pitch;
	fb_surface->pixels = sdl_surface->pixels;
	fb_surface->user   = sdl_surface;

	return (fb_surface);
}

void
fb_release_surface(FB_Surface *surface) {
	SDL_FreeSurface((SDL_Surface *)surface->user);

	free(surface);
}

void
fb_fill(FB_Surface *surface, FB_Rectangle *rect, FB_Color *color) {
	SDL_Rect *r = NULL, sdl_rect;
	SDL_Surface *dst = screen;
	unsigned int c;

	if (rect) {
		FB2SDL_Rectangle(rect, &sdl_rect);
		r = &sdl_rect;
	}

	if (surface) {
		dst = (SDL_Surface *)surface->user;
	}

	c = SDL_MapRGBA(dst->format, color->r, color->g, color->b, color->a);

	SDL_FillRect(dst, r, c);
}

void
fb_set_pixel(FB_Surface *surface, int x, int y, FB_Color *color) {
	SDL_Surface *dst = screen;
	unsigned int pixel;
	int bpp;

	if (surface) {
		dst = (SDL_Surface *)surface->user;
	}

	bpp = dst->format->BytesPerPixel; 
	pixel = SDL_MapRGBA(dst->format, color->r, color->g, color->b, color->a);

	// Here p is the address to the pixel we want to set 
	unsigned char *p = (unsigned char *)dst->pixels + y * dst->pitch + x * bpp; 

	switch (bpp) { 
		case 1: 
			*p = pixel; 
		break; 

		case 2: 
			*(unsigned short int *)p = pixel; 
		break; 

		case 3: 
			if(SDL_BYTEORDER == SDL_BIG_ENDIAN) { 
				p[0] = (pixel >> 16) & 0xff; 
				p[1] = (pixel >> 8) & 0xff; 
				p[2] = pixel & 0xff; 
			} else { 
				p[0] = pixel & 0xff; 
				p[1] = (pixel >> 8) & 0xff; 
				p[2] = (pixel >> 16) & 0xff; 
			} 
		break; 

		case 4: 
			*(unsigned int *)p = pixel; 
		break; 
	} 
}

void
fb_get_pixel(FB_Surface *surface, int x, int y, FB_Color *color) {
}

