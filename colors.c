#include "colors.h"

internal struct rgb
rgb_unpack(u32 color) {
  struct rgb unpacked;
  unpacked.r = (color & 0xFF0000) >> 16;
  unpacked.g = (color & 0x00FF00) >> 8;
  unpacked.b = (color & 0x0000FF);
  unpacked.unused = 0;
  return unpacked;
}

internal struct rgba
rgba_unpack(u32 color) {
  struct rgba unpacked;
  unpacked.r = (color & 0xFF000000) >> 24;
  unpacked.g = (color & 0x00FF0000) >> 16;
  unpacked.b = (color & 0x0000FF00) >> 8;
  unpacked.a = (color & 0x000000FF);
  return unpacked;
}
