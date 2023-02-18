#include "colors.h"

internal struct rgb
rgb_unpack(u32 color) {
  struct rgb unpacked;
  unpacked.r  = (color & 0x00FF0000) >> 16;
  unpacked.g  = (color & 0x0000FF00) >> 8;
  unpacked.b  = (color & 0x000000FF);
  return unpacked;
}
