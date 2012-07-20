#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <kernel/os/FreeRTOS.h>
#include <kernel/os/task.h>

#include <kernel/hal/mouse/mouse.h> // mouse
#include <kernel/hal/kbd/kbd.h>	  // keyboard
#include <kernel/hal/fb/fb.h>		 // framebuffer

//#include <lib/gfx/bitmap.h>
#include <lib/gfx/glyph.h>

#include "term/terminal.h"
#include "cli/cli.h"
#include "fs/fs.h"

static const char *str = "pir{A}tos version " VERSION " (" PLATFORM ")";

static int quit = 0;

extern GFX_Bitmap piratos_logo;
extern GFX_Bitmap piratos_font;

//extern "C"{
void piratos(void);
//}

static void
AnimationTask(void *parameters) {
	FB_Color bg = { 0x20, 0x10, 0x60, 0xff };
	FB_Surface *logo = NULL;
	FB_Rectangle dst, upd;
	int dir, pos;

	logo = gfx_bitmap_load(&piratos_logo);

	pos = (1280 - logo->width) / 2;
	dst.y = 768 - logo->height;
	dst.w = logo->width;
	dst.h = logo->height;
	dir = 3;

	while (!quit) {
		dst.x = pos;
		pos += dir;

		fb_fill(NULL, &dst, &bg);
		fb_blit(logo, NULL, NULL, &dst, &upd);
		fb_flip(&dst);

		if ((pos >= 1280 - dst.w) || (pos <= 0)) dir *= -1;

		vTaskDelay(10);
	}
}

static void
ShellTask(void *arg) {
	Terminal *terminal = (Terminal *)arg;
	MOUSE_Event mouse_ev;
	KBD_Event kbd_ev;

	while (!quit) {
		if (mouse_poll(&mouse_ev)) {
			//printf("mouse poll: %i, %i, %i, %i\n",
			//	mouse_ev.x, mouse_ev.y, mouse_ev.state, mouse_ev.button);
		}

		if (kbd_poll(&kbd_ev)) {
			//printf("kbd poll: %i, %i, %i\n", kbd_ev.state, kbd_ev.symbol, kbd_ev.unicode);

			if (kbd_ev.state == KBD_EVENT_STATE_PRESSED)
				if (kbd_ev.symbol == KBD_KEY_ESCAPE)
					if (kbd_ev.modifier == KBD_MOD_LALT)
						{ quit = 1; exit(0); }

			terminal->KeyEvent(&kbd_ev);
		}

		terminal->Update();

		//usleep(10*1000);
		vTaskDelay(10);
	}
}

void
piratos(void) {
	FB_Color bg = { 0x20, 0x10, 0x60, 0xff };
	FB_Color fg = { 0xc0, 0x60, 0x10, 0xff };
	FB_Surface *font = NULL;

	printf("%s\n", str);

	fb_init();
	fb_mode(FB_MODE_1280x768, FB_FORMAT_BEST);
	fb_fill(NULL, NULL, &bg);
	fb_flip(NULL);

	font = gfx_glyph_load(&piratos_font, &fg);

	gfx_glyph_string(NULL, font, 10, 540, str);

	cli_register_commands();

	fs_init();
	kbd_init();
	mouse_init();

	Terminal *terminal = new Terminal(&piratos_font);

	xTaskCreate(AnimationTask, "LOGOx",  configMINIMAL_STACK_SIZE, NULL, 8, NULL);
	xTaskCreate(ShellTask,     "SHELLx", configMINIMAL_STACK_SIZE, terminal, 12, NULL);

	vTaskStartScheduler();
}
