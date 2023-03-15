#ifndef SURFACE_H
#define SURFACE_H

#include <SDL/SDL.h>
#include "base_types.h"
#include "internals.h"
#include "colors.h"

enum bmp_load_type {
  LOAD_BITMAP_UNOPTIMIZED,
  LOAD_BITMAP_PREOPTIMIZED
};

enum {
  SURFACE_SCALE_PROGRESSIVE,
  SURFACE_SCALE_PROGRESSIVE2,
  SURFACE_SCALE_INTERLACED,
  SURFACE_SCALE_INTERLACED2
};

internal void surface_progressive_scale(SDL_Surface*, SDL_Surface*, u32);

#endif /* SURFACE_H */
