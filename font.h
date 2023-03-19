#ifndef FONT_DEFINITION_H
#define FONT_DEFINITION_H

#include "base_types.h"

struct FontDefinition {
  struct {
    void* data;   /* texture data, implmentation defined */
    i32 kerning;
  } font;
  struct {
    struct {
      u8 head, tail;
    } range;
  } ascii;
  void** glyphs;  /* array of glyph information, implementation defined. */
};

#endif /* FONT_DEFINITION_H */
