#include <SDL_video.h>
#include "ascii_table.h"
#include "font.h"

internal void
SDL_DrawMonospaceText(struct FontDefinition* fd, SDL_Surface* surface, char* str, i32 start_x, i32 start_y) {
  SDL_Surface* font;
  SDL_Rect* glyph;
  SDL_Rect src;
  u32 jump;

  font = fd->font.data;
  glyph = (SDL_Rect*)fd->glyphs;
  jump = glyph[fd->ascii.range.head].w + fd->font.kerning;
  src.x = start_x;
  src.y = start_y;
  src.w = font->w;
  src.h = font->h;

  {
    i32 index;
    char* c = str;
    while (*c) {
      u8 dec = (i32)(*c);
      if (dec < fd->ascii.range.head || dec > fd->ascii.range.tail)
        index = fd->ascii.range.head;
      else
        index = dec;

      SDL_BlitSurface(font, &glyph[index], surface, &src);
      src.x += jump;
      c++;
    }
  }
}
