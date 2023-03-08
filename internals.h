
#ifndef INTERNALS_H
#define INTERNALS_H

#include "base_types.h"

#ifndef TARGET
# error "You must define a TARGET for building."
#endif

#if TARGET==wiz
typedef u16 PIXEL;
# define SCREEN_DEPTH 16
#elif TARGET==x86_64
typedef u32 PIXEL;
# define SCREEN_DEPTH 32
#else
# error "You must define TARGET to \"wiz\" or \"x86_64\""
#endif

global u32 display_scale = 2;   /* this should get updated if the screen size gets altered */
#define BACKBUFFER_WIDTH  160
#define BACKBUFFER_HEIGHT 120
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#define GAME_NAME "chimmy"

#endif
