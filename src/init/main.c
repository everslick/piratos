#include <stdio.h>

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"

#include "fat_filelib.h"

#include "sd.h"

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

static void
fat_test() {
	FL_FILE *file;

	// Initialise media
	sd_init();

	// Initialise File IO Library
	fl_init();

	// Attach media access functions to library
	if (fl_attach_media(sd_read, sd_write) != FAT_INIT_OK) {
		printf("ERROR: Media attach failed\n");
		return; 
	}

	// List root directory
	fl_listdirectory("/");

	// Create File
	file = fl_fopen("/file.bin", "w");
	if (file) {
		// Write some data
		unsigned char data[] = { 1, 2, 3, 4 };
		if (fl_fwrite(data, 1, sizeof(data), file) != sizeof(data))
			printf("ERROR: Write file failed\n");
	}
	else
		printf("ERROR: Create file failed\n");

	// Close file
	fl_fclose(file);

	// List root directory
	fl_listdirectory("/");

	// Delete File
	if (fl_remove("/file.bin") < 0)
		printf("ERROR: Delete file failed\n");

	// List root directory
	fl_listdirectory("/");

	fl_shutdown();

	sd_fini();
}

void piratos() {
	print_uart0("pir{a}tos version " VERSION " (" PLATFORM ")");

	CreateFlashTasks();

	fat_test();

	vTaskStartScheduler();
}
