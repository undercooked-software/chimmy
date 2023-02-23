#include "surface.h"

internal void
surface_interlaced_scale(SDL_Surface* backbuffer, SDL_Surface* screen, i32 scale) {
  i32 x, y, i;
  u16* screen_pixels      = (u16*)screen->pixels;
  u16* backbuffer_pixels  = (u16*)backbuffer->pixels;
  for (y = 0; y < backbuffer->h; ++y) {
    for (x = 0; x < backbuffer->w; ++x) {
      for (i = 0; i < scale; ++i) {
        screen_pixels[((y * scale * screen->w) + (x * scale)) + i] =
           backbuffer_pixels[(y * backbuffer->w) + x];
      }
    }
  }
}

internal void
surface_interlaced_scale2(SDL_Surface* backbuffer, SDL_Surface* screen, i32 scale) {
  /*
   * Counterpillow's clean implementation, modified for interlaced scale
   * Currently performs better than mine based on our current FPS margins!
   */
  i32 x, y;
  u16* screen_pixels      = (u16*)screen->pixels;
  u16* backbuffer_pixels  = (u16*)backbuffer->pixels;
  for (y = 0; y < screen->h; y+=2) {
    for (x = 0; x < screen->w; x++) {
      screen_pixels[y * screen->w + x] =
        backbuffer_pixels[y / scale * screen->w / scale + x / scale];
    }
  }
}

internal void
surface_progressive_scale(SDL_Surface* backbuffer, SDL_Surface* screen, i32 scale) {
  /* This is very consistent when measured via FPS? */
  i32 x, y, i;
  SDL_Surface* line;
  SDL_Rect position;
  line = SDL_CreateRGBSurface(SDL_SWSURFACE, screen->w, 1, 16,
                              screen->format->Rmask, screen->format->Gmask,
                              screen->format->Rmask, screen->format->Amask);
  position.w = line->w;
  position.h = line->h;
  position.x = 0;

  {
    u16* backbuffer_pixels  = (u16*)backbuffer->pixels;
    u16* line_pixels        = (u16*)line->pixels;
    for (y = 0; y < backbuffer->h; ++y) {
      for (x = 0; x < backbuffer->w; ++x) {
        for (i = 0; i < scale; ++i) {
          /* record the current row of data */
          line_pixels[(x * scale) + i] = backbuffer_pixels[(y * backbuffer->w + x)];
        }
      }
      for (i = 0; i < scale; ++i) {
        position.y = (y*scale)+i;
        SDL_BlitSurface(line, NULL, screen, &position);
      }
    }
  }
  SDL_FreeSurface(line);
}

internal void
surface_progressive_scale2(SDL_Surface* backbuffer, SDL_Surface* screen, i32 scale) {
  /*
   * Counterpillow's clean implementation
   * I like the way it's done, but it seems to be less performant for some reason?
   */
  i32 x, y;
  u16* screen_pixels      = (u16*)screen->pixels;
  u16* backbuffer_pixels  = (u16*)backbuffer->pixels;
  for (y = 0; y < screen->h; y++) {
    for (x = 0; x < screen->w; x++) {
      screen_pixels[y * screen->w + x] =
        backbuffer_pixels[y / scale * screen->w / scale + x / scale];
    }
  }
}
