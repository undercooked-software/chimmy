
#ifndef INTERNALS_H
#define INTERNALS_H

#include "base_types.h"

#define  SCREEN_DEPTH 32  /* set this as the common default */
#if TARGET_WIZ
# undef  SCREEN_DEPTH
# define SCREEN_DEPTH 16
#elif TARGET_X86_64
# undef  SCREEN_DEPTH
# define SCREEN_DEPTH 32
#endif

#if SCREEN_DEPTH==16
typedef u16 PIXEL;
#elif SCREEN_DEPTH==32
typedef u32 PIXEL;
#endif

#define BACKBUFFER_WIDTH  160
#define BACKBUFFER_HEIGHT 120

#define GAME_NAME "chimmy"
#define WM_CLASS  "com.example.chimmy"

#endif /* INTERNALS_H */
