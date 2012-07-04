#include <stdio.h>

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"

#include "fat_filelib.h"

#include "fs.h"

#include "shell.h"

#define NUM_TASKS 4

int init_kbd(void);
int init_fb(void);

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
	print_uart0("pir{A}tos Version " VERSION " (" PLATFORM ")");

	CreateFlashTasks();

	fs_init();
	shell_init();
	fs_fini();

	vTaskStartScheduler();
}
