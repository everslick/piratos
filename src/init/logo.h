#ifndef _PIRATOS_LOGO_
#define _PIRATOS_LOGO_

typedef struct PiratosLogo {
  unsigned int  width;
  unsigned int  height;
  unsigned int  bytes_per_pixel; /* 3:RGB, 4:RGBA */ 
  unsigned char pixel_data[800 * 160 * 4 + 1];
} PiratosLogo;

extern PiratosLogo piratos_logo;

#endif // _PIRATOS_LOGO_
