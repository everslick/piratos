#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <kernel/os/queue.h>
#include <kernel/os/task.h>

#include <kernel/hal/mouse/mouse.h>
#include <kernel/hal/kbd/kbd.h>
#include <kernel/hal/fb/fb.h>

#include <lib/sys/log.h>

#include "dpy.h"

#define zero(_A_) memset((&_A_), 0, (sizeof (_A_)))

static xQueueHandle dpy_op_queue;
static xQueueHandle dpy_kbd_event_queue;
static xQueueHandle dpy_mouse_event_queue;

enum {
	DPY_OP_NONE,
	DPY_OP_FILL,
	DPY_OP_BLIT,
	DPY_OP_FLIP
};

typedef struct {
	int type;

	FB_Surface   sfc;
	FB_Rectangle src;
	FB_Rectangle dst;
	FB_Color     col;
} DPY_Op;

void
dpy_fill(FB_Rectangle *dst, FB_Color *col) {
	DPY_Op op;

	zero(op);
	op.type = DPY_OP_FILL;
	if (dst) op.dst = *dst;
	if (col) op.col = *col;

	xQueueSend(dpy_op_queue, &op, 0);
}

void
dpy_blit(FB_Surface *sfc, FB_Rectangle *src, FB_Rectangle *dst) {
	DPY_Op op;

	zero(op);
	op.type = DPY_OP_BLIT;
	if (sfc) op.sfc = *sfc;
	if (src) op.src = *src;
	if (dst) op.dst = *dst;

	xQueueSend(dpy_op_queue, &op, 0);
}

void
dpy_flip(FB_Rectangle *rect) {
	DPY_Op op;

	zero(op);
	op.type = DPY_OP_FLIP;
	if (rect) op.dst = *rect;

	xQueueSend(dpy_op_queue, &op, 0);
}

int
dpy_kbd_poll(KBD_Event *ev) {
	return (xQueueReceive(dpy_kbd_event_queue, ev, 0));
}

int
dpy_mouse_poll(MOUSE_Event *ev) {
	return (xQueueReceive(dpy_mouse_event_queue, ev, 0));
}

void
//static dpy_task(void *arg) {
dpy_update(void) {
		MOUSE_Event mouse_ev;
		KBD_Event kbd_ev;
		FB_Rectangle upd;
		DPY_Op op;

//	while (1) {
		if (mouse_poll(&mouse_ev)) {
			xQueueSend(dpy_mouse_event_queue, &mouse_ev, 0);
		}

		if (kbd_poll(&kbd_ev)) {
			xQueueSend(dpy_kbd_event_queue, &kbd_ev, 0);
		}

		while (xQueueReceive(dpy_op_queue, &op, 0)) {
			FB_Surface   *sfc = NULL;
			FB_Rectangle *src = NULL;
			FB_Rectangle *dst = NULL;
			FB_Color     *col = NULL;

			switch (op.type) {
				case DPY_OP_FLIP:
					if (op.dst.w && op.dst.h) {
						dst = &op.dst;
					}

					// update screen
					fb_flip(dst);
				break;

				case DPY_OP_BLIT:
					if (op.sfc.width && op.sfc.height) {
						sfc = &op.sfc;
					}
					if (op.src.w && op.src.h) {
						src = &op.src;
					}
					if (op.dst.w && op.dst.h) {
						dst = &op.dst;
					}

					// blit screen
					fb_blit(sfc, src, NULL, dst, &upd);

					// TODO mark dirty with upd rectangle
				break;

				case DPY_OP_FILL:
					if (op.dst.w && op.dst.h) {
						dst = &op.dst;
					}

					// fill rectangle
					fb_fill(NULL, dst, &op.col);
				break;
			}
		}
 
		//vTaskDelay(10);
//	}
}

void
dpy_init(void) {
	FB_Color bg = { 0x20, 0x10, 0x60, 0xff };

	syslog(SYS_LOG_INFO, "setting up display server\n");

	dpy_op_queue          = xQueueCreate(32, sizeof (DPY_Op));
	dpy_kbd_event_queue   = xQueueCreate( 8, sizeof (KBD_Event));
	dpy_mouse_event_queue = xQueueCreate(16, sizeof (MOUSE_Event));

	fb_init();

	fb_mode(FB_MODE_1280x768, FB_FORMAT_BEST);
	fb_fill(NULL, NULL, &bg);
	fb_flip(NULL);

	kbd_init();
	mouse_init();

	syslog(SYS_LOG_INFO, "display server started\n");

	//xTaskCreate(dpy_task, "Display",  configMINIMAL_STACK_SIZE, NULL, 16, NULL);
}
