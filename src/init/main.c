#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <kernel/os/task.h>

#include <kernel/hal/mouse/mouse.h>
#include <kernel/hal/kbd/kbd.h>
#include <kernel/hal/fb/fb.h>

#include <lib/gfx/glyph.h>
#include <lib/sys/log.h>
#include <srv/dpy/dpy.h>

#include "term/terminal.h"
#include "cli/cli.h"
#include "fs/fs.h"

int system_log_level = SYS_LOG_ALWAYS;

static const char *str = "pir{A}tos version " VERSION " (" PLATFORM ")";

extern GFX_Bitmap piratos_logo;
extern GFX_Bitmap piratos_font;

static void
AnimationTask(void *arg) {
	FB_Color bg = { 0x20, 0x10, 0x60, 0xff };
	FB_Surface *logo = NULL;
	FB_Rectangle dst;
	int dir, pos;

	logo = gfx_bitmap_load(&piratos_logo);

	pos = (1280 - logo->width) / 2;
	dst.y = 768 - logo->height;
	dst.w = logo->width;
	dst.h = logo->height;
	dir = 3;

	while (1) {
		dst.x = pos;
		pos += dir;

		dpy_fill(&dst, &bg);
		dpy_blit(logo, NULL, &dst);
		dpy_flip(&dst);

		if ((pos >= 1280 - dst.w) || (pos <= 0)) dir *= -1;

		vTaskDelay(10);
	}
}

static void
TerminalTask(void *arg) {
	MOUSE_Event mouse_ev;
	KBD_Event kbd_ev;

	Terminal *terminal = terminal_new(&piratos_font);

	while (1) {
		if (dpy_mouse_poll(&mouse_ev)) {
			syslog(SYS_LOG_DEBUG, "mouse poll: %i, %i, %i, %i\n",
				mouse_ev.x, mouse_ev.y, mouse_ev.state, mouse_ev.button);
		}

		if (dpy_kbd_poll(&kbd_ev)) {
			if (kbd_ev.state == KBD_EVENT_STATE_PRESSED)
				if (kbd_ev.symbol == KBD_KEY_ESCAPE)
					if (kbd_ev.modifier == KBD_MOD_LALT) exit(0);

			terminal_key_event(terminal, &kbd_ev);
		}

		terminal_update(terminal);

		vTaskDelay(10);
	}
}

void
piratos(void) {
	//FB_Color bg = { 0x20, 0x10, 0x60, 0xff };
	//FB_Color fg = { 0xc0, 0x60, 0x10, 0xff };
	//FB_Surface *font = NULL;

	syslog(SYS_LOG_INFO, "%s\n", str);

	dpy_init();
	fs_init();

	//font = gfx_glyph_load(&piratos_font, &fg);

	//gfx_glyph_string(NULL, font, 10, 540, str);

	cli_register_commands();

	//xTaskCreate(AnimationTask, "Animation", configMINIMAL_STACK_SIZE, NULL, 16, NULL);
	xTaskCreate(TerminalTask,  "Terminal",  configMINIMAL_STACK_SIZE, NULL, 12, NULL);

	vTaskStartScheduler();
}
