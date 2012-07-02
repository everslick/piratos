#include <stdio.h>

static FILE *fat_image = NULL;

int
sd_init(void) {
	fat_image = fopen("/home/clemens/piratos.fat", "r+");

	if (!fat_image) return (0);

	return (1);
}

int
sd_fini(void) {
	if (!fat_image) return (0);

	fclose(fat_image);
	fat_image = NULL;

	return (1);
}

int
sd_read(unsigned long sector, unsigned char *buffer, unsigned long sector_count) {
	unsigned long i;

	if (!fat_image) return (0);

	// platform specific sector (512 bytes) read code
	fseek(fat_image, sector * 512, SEEK_SET);
	fread(buffer, 512, 1, fat_image);

	return (1);
}

int
sd_write(unsigned long sector, unsigned char *buffer, unsigned long sector_count) {
	unsigned long i;

	if (!fat_image) return (0);

	// platform specific sector (512 bytes) write code
	fseek(fat_image, sector * 512, SEEK_SET);
	fwrite(buffer, 512, 1, fat_image);

	return (1);
}
