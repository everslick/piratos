#include <stdio.h>

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"

#include "mouse.h" // mouse
#include "kbd.h"   // keyboard
#include "fb.h"    // framebuffer
#include "fs.h"    // filesystem

#include "shell.h"
#include "logo.h"

#define NUM_TASKS 4

/* Structure used to pass parameters to the LED tasks. */
typedef struct TASK_PARAMETERS {
	char str[4];				/*< The output the task should use. */
	portTickType rate;		/*< The rate at which the LED should flash. */
} TaskParameters;

static TaskParameters param[NUM_TASKS];

static void FlashTask(void *);

static volatile unsigned int * const UART0DR = (unsigned int *)0x101f1000;
static char t = 0;

static void print_uart0(const char *s) {
	puts(s);

//	while(*s != '\0') { /* Loop until end of string */
//		*UART0DR = *s; /* Transmit char */
//		s++; /* Next char */
//	}
}

static void CreateFlashTasks(void) {
	const portTickType rate = 500;

	for (t = 0; t < NUM_TASKS; t++) {
		param[t].str[0] = 'A' + t;
		param[t].str[1] = '\0';
		param[t].rate = (rate + (rate * t));
		param[t].rate /= portTICK_RATE_MS;

		xTaskCreate(FlashTask, "LEDx", configMINIMAL_STACK_SIZE, &(param[t]), t, NULL);
	}
}

static void FlashTask(void *parameters) {
	TaskParameters *p = (TaskParameters *)parameters;

	for (;;) {
		vTaskDelay(p->rate);
		print_uart0(p->str);
	}
}

void piratos(void) {
	FB_Color c, bg = { 0x50, 0x20, 0xa0, 0xff };
	MOUSE_Event mouse_ev;
	KBD_Event kbd_ev;
	FB_Surface *logo;

	print_uart0("pir{A}tos Version " VERSION " (" PLATFORM ")");

	fb_init();
	fb_mode(FB_MODE_1280x768, FB_FORMAT_BEST);
	fb_fill(NULL, NULL, &bg);

	logo = fb_create_surface(piratos_logo.width, piratos_logo.height, FB_FORMAT_BEST);

	for (int y=0; y<logo->height; y++) {
		for (int x=0; x<logo->width; x++) {
			int idx = y * logo->width * 4 + x * 4;

			c.r = piratos_logo.pixel_data[idx + 0];
			c.g = piratos_logo.pixel_data[idx + 1];
			c.b = piratos_logo.pixel_data[idx + 2];
			c.a = piratos_logo.pixel_data[idx + 3];

			fb_set_pixel(logo, x, y, &c);
		}
	}

	fb_blit(logo, NULL, 280, 568, NULL);
	fb_flip(NULL);

	mouse_init();
	kbd_init();
	while (1) {
		if (mouse_poll(&mouse_ev)) {
			printf("mouse poll: %i, %i, %i, %i\n",
				mouse_ev.x, mouse_ev.y, mouse_ev.state, mouse_ev.button);
		}

		if (kbd_poll(&kbd_ev)) {
			printf("kbd poll: %i, %i\n", kbd_ev.state, kbd_ev.symbol);
		}
	}

	fs_init();
	shell_init();
	fs_fini();
	fb_fini();

	CreateFlashTasks();

	vTaskStartScheduler();
}
