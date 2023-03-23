#ifndef SDL_DISPLAY_H
#define SDL_DISPLAY_H

#include <SDL_video.h>
#include "base_types.h"

struct SDL_Display {
  SDL_Surface* screen;
  SDL_Surface* backbuffer;
  u16 w, h;
  u8 scale;
};

#endif /* SDL_DISPLAY_H */
