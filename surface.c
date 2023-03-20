#include "surface.h"

#define bmp_load(filename, is_opt) \
  (is_opt == LOAD_BITMAP_UNOPTIMIZED) ? \
    bmp_postopt_load(filename) : \
    bmp_preopt_load(filename)

internal SDL_Surface*
bmp_postopt_load(const char* filename) {
  SDL_Surface* optimized_bmp = 0;
  SDL_Surface* raw_bmp = SDL_LoadBMP(filename);
  if (raw_bmp) {
    optimized_bmp = SDL_DisplayFormat(raw_bmp);
    SDL_FreeSurface(raw_bmp);
  }
  return optimized_bmp;
}

internal SDL_Surface*
bmp_preopt_load(const char* filename) {
  return SDL_LoadBMP(filename);
}

internal SDL_Surface*
bmp_trans_load(const char* filename, u32 color, enum bmp_load_type loader) {
  SDL_Surface* bmp = bmp_load(filename, loader);
  if (bmp) {
    struct rgb unpacked = rgb_unpack(color);
    u32 key = SDL_MapRGB(bmp->format, unpacked.r, unpacked.g, unpacked.b);
    SDL_SetColorKey(bmp, SDL_SRCCOLORKEY, key);
  }
  return bmp;
}

internal SDL_Surface*
bmp_scale(SDL_Surface* texture, u32 color, u32 scale) {
  SDL_Surface* scaled_texture;
  {
    u32 w, h;
    SDL_PixelFormat* fmt = texture->format;
    w = texture->w * scale;
    h = texture->h * scale;
    scaled_texture =
      SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, fmt->BitsPerPixel,
                           fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
    surface_progressive_scale(texture, scaled_texture, scale);
    {
      struct rgb unpacked = rgb_unpack(color);
      u32 key = SDL_MapRGB(scaled_texture->format, unpacked.r, unpacked.g, unpacked.b);
      SDL_SetColorKey(scaled_texture, SDL_SRCCOLORKEY, key);
    }
  }

  SDL_FreeSurface(texture);
  return scaled_texture;
}

/* internal void */
/* surface_interlaced_scale(SDL_Surface* backbuffer, SDL_Surface* screen, u32 scale) { */
/*   i32 x, y, i; */
/*   PIXEL* screen_pixels      = (PIXEL*)screen->pixels; */
/*   PIXEL* backbuffer_pixels  = (PIXEL*)backbuffer->pixels; */
/*   for (y = 0; y < backbuffer->h; ++y) { */
/*     for (x = 0; x < backbuffer->w; ++x) { */
/*       for (i = 0; i < scale; ++i) { */
/*         screen_pixels[((y * scale * screen->w) + (x * scale)) + i] = */
/*            backbuffer_pixels[(y * backbuffer->w) + x]; */
/*       } */
/*     } */
/*   } */
/* } */

/*internal void */
/*surface_interlaced_scale2(SDL_Surface* backbuffer, SDL_Surface* screen, u32 scale) { */
/*  /1* */
/*   * Counterpillow's clean implementation, modified for interlaced scale */
/*   * Currently performs better than mine based on our current FPS margins! */
/*   *1/ */
/*  i32 x, y; */
/*  PIXEL* screen_pixels      = (PIXEL*)screen->pixels; */
/*  PIXEL* backbuffer_pixels  = (PIXEL*)backbuffer->pixels; */
/*  for (y = 0; y < screen->h; y+=2) { */
/*    for (x = 0; x < screen->w; x++) { */
/*      screen_pixels[y * screen->w + x] = */
/*        backbuffer_pixels[y / scale * screen->w / scale + x / scale]; */
/*    } */
/*  } */
/*} */

internal void
surface_progressive_scale(SDL_Surface* backbuffer, SDL_Surface* screen, u32 scale) {
  SDL_Surface* line;
  SDL_Rect position;
  {
    SDL_PixelFormat* fmt = screen->format;
    line = SDL_CreateRGBSurface(SDL_SWSURFACE, screen->w, 1, fmt->BitsPerPixel,
                                fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
  }
  position.w = line->w;
  position.h = line->h;
  position.x = 0;

  {
    i32 x, y, i;
    PIXEL* backbuffer_pixels  = (PIXEL*)backbuffer->pixels;
    PIXEL* line_pixels        = (PIXEL*)line->pixels;
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

/*internal void */
/*surface_progressive_scale2(SDL_Surface* backbuffer, SDL_Surface* screen, u32 scale) { */
/*  /1* */
/*   * Counterpillow's clean implementation */
/*   * I like the way it's done, but it seems to be less performant for some reason? */
/*   *1/ */
/*  i32 x, y; */
/*  PIXEL* screen_pixels      = (PIXEL*)screen->pixels; */
/*  PIXEL* backbuffer_pixels  = (PIXEL*)backbuffer->pixels; */
/*  for (y = 0; y < screen->h; y++) { */
/*    for (x = 0; x < screen->w; x++) { */
/*      screen_pixels[y * screen->w + x] = */
/*        backbuffer_pixels[y / scale * screen->w / scale + x / scale]; */
/*    } */
/*  } */
/*} */
