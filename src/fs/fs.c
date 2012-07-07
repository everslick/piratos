#include "fat_filelib.h"

#include "sd.h"

static int initialized = 0;

int
fs_init(void) {
	if (initialized) return (-1);

	// Initialise media
	sd_init();

	// Initialise File IO Library
	fl_init();

	// Attach media access functions to library
	if (fl_attach_media(sd_read, sd_write) != FAT_INIT_OK) {
		return (-1); 
	}

	initialized = 1;

	return (0);
}

int
fs_fini(void) {
	if (!initialized) return (-1);

	fl_shutdown();

	sd_fini();

	initialized = 0;

	return (0);
}
