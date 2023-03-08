#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL/SDL_video.h>
#include "base_types.h"

struct display {
  SDL_Surface* screen;
  SDL_Surface* backbuffer;
  u16 w, h;
  u8 scale;
};

#endif /* DISPLAY_H */
