#ifndef _SRV_DPY_H_
#define _SRV_DPY_H_

#include <kernel/hal/mouse/mouse.h>
#include <kernel/hal/kbd/kbd.h>
#include <kernel/hal/fb/fb.h>

void dpy_fill(FB_Rectangle *dst, FB_Color *col);
void dpy_blit(FB_Surface *sfc, FB_Rectangle *src, FB_Rectangle *dst);
void dpy_flip(FB_Rectangle *rect);

int dpy_kbd_poll(KBD_Event *ev);
int dpy_mouse_poll(MOUSE_Event *ev);

void dpy_init(void);

#endif // _SRV_DPY_H_
