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
}

int
fs_fini(void) {
	if (!initialized) return (-1);

	fl_shutdown();

	sd_fini();

	initialized = 0;

	return (0);
}

int
fs_test(void) {
	FL_FILE *file;

	fs_init();

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

	fs_fini();
}
