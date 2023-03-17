#ifndef SURFACE_H
#define SURFACE_H

#include <SDL.h>
#include "base_types.h"
#include "colors.h"
#include "internals.h"

enum bmp_load_type {
  LOAD_BITMAP_UNOPTIMIZED,
  LOAD_BITMAP_PREOPTIMIZED
};

enum {
  SURFACE_SCALE_PROGRESSIVE
  /* SURFACE_SCALE_PROGRESSIVE2 */
  /* SURFACE_SCALE_INTERLACED, */
  /* SURFACE_SCALE_INTERLACED2 */
};

internal SDL_Surface* bmp_postopt_load(const char*);
internal SDL_Surface* bmp_preopt_load(const char*);
internal SDL_Surface* bmp_trans_load(const char*, u32, enum bmp_load_type);
internal SDL_Surface* bmp_scale(SDL_Surface*, u32, u32);

/* internal void surface_interlaced_scale(SDL_Surface*, SDL_Surface*, u32); */
/* internal void surface_interlaced_scale2(SDL_Surface*, SDL_Surface*, u32); */
internal void surface_progressive_scale(SDL_Surface*, SDL_Surface*, u32);
/* internal void surface_progressive_scale2(SDL_Surface*, SDL_Surface*, u32); */

#endif /* SURFACE_H */
