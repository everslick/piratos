#ifndef _SD_H_
#define _SD_H_

int sd_init(void);
int sd_fini(void);
int sd_read(unsigned long sector, unsigned char *buffer, unsigned long sector_count);
int sd_write(unsigned long sector, unsigned char *buffer, unsigned long sector_count);

#endif // _SD_H_
