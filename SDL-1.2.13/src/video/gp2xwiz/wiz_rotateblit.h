#ifndef _WIZ_ROTATEBLIT_H_
#define _WIZ_ROTATEBLIT_H_

#include "SDL_video.h"

extern int wiz_rotateblitsurface8(unsigned char* srcpixel,unsigned char* dstpixel,SDL_Rect* rect);
extern int wiz_rotateblitsurface16(unsigned short* srcpixel,unsigned short* dstpixel,SDL_Rect* rect);
extern int wiz_rotateblitsurface32(unsigned int* srcpixel,unsigned int* dstpixel,SDL_Rect* rect);

#endif


