#include <SDL_video.h>
#include "base_types.h"
#include "internals.h"
#include "colors.h"

internal void
SDL_RGBColorKeySurface(SDL_Surface* src, u32 color) {
  struct rgb unpacked = rgb_unpack(color);
  u32 key = SDL_MapRGB(src->format, unpacked.r, unpacked.g, unpacked.b);
  SDL_SetColorKey(src, SDL_SRCCOLORKEY, key);
}

internal void
SDL_ChunkScaleCopySurface(SDL_Surface* src, SDL_Surface* dst, u32 scale) {
  SDL_Surface* line;
  SDL_Rect position;
  {
    SDL_PixelFormat* fmt = dst->format;
    line = SDL_CreateRGBSurface(SDL_SWSURFACE, dst->w, 1, fmt->BitsPerPixel,
                                fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
  }
  position.w = line->w;
  position.h = line->h;
  position.x = 0;

  {
    i32 x, y, i;
    PIXEL* src_pixels  = (PIXEL*)src->pixels;
    PIXEL* line_pixels        = (PIXEL*)line->pixels;
    for (y = 0; y < src->h; ++y) {
      for (x = 0; x < src->w; ++x) {
        for (i = 0; i < scale; ++i) {
          /* record the current row of data */
          line_pixels[(x * scale) + i] = src_pixels[(y * src->w + x)];
        }
      }
      for (i = 0; i < scale; ++i) {
        position.y = (y*scale)+i;
        SDL_BlitSurface(line, NULL, dst, &position);
      }
    }
  }
  SDL_FreeSurface(line);
}

internal void
SDL_DirectScaleCopySurface(SDL_Surface* src, SDL_Surface* dst, u32 scale) {
  /*
   * Counterpillow's clean implementation
   * I like the way it's done, but it seems to be less performant for some reason?
   */
  i32 x, y;
  PIXEL* src_pixels = (PIXEL*)src->pixels;
  PIXEL* dst_pixels = (PIXEL*)dst->pixels;
  for (y = 0; y < dst->h; y++) {
    for (x = 0; x < dst->w; x++) {
      dst_pixels[y * dst->w + x] =
        src_pixels[y / scale * dst->w / scale + x / scale];
    }
  }
}

internal SDL_Surface*
SDL_ScaleSurface(SDL_Surface* src, u32 scale) {
  SDL_Surface* dst;
  {
    u32 w, h;
    SDL_PixelFormat* fmt = src->format;
    w = src->w * scale;
    h = src->h * scale;

    dst = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, fmt->BitsPerPixel,
                               fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
    SDL_ChunkScaleCopySurface(src, dst, scale);
  }

  SDL_FreeSurface(src);
  return dst;
}
