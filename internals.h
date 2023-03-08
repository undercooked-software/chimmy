
#ifndef INTERNALS_H
#define INTERNALS_H

#include "base_types.h"

#ifndef TARGET
# error "You must define a TARGET for building."
#endif

#if TARGET==wiz
# define SCREEN_DEPTH 16
typedef u16 PIXEL;
#elif TARGET==x86_64
# define SCREEN_DEPTH 32
typedef u32 PIXEL;
#else
# error "You must define TARGET to \"wiz\" or \"x86_64\""
#endif

#define BACKBUFFER_WIDTH  160
#define BACKBUFFER_HEIGHT 120

#define GAME_NAME "chimmy"

#endif
