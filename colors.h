#ifndef COLORS_H
#define COLORS_H

#include "base_types.h"

enum {
  BLACK           = 0x000000,
  CLOUD           = 0xC4C4C4,
  WHITE           = 0xFFFFFF,
  DARK_RED        = 0x7F0702,
  CRIMSON_RED     = 0xA50002,
  TOMATO          = 0xDB2800,
  DEEP_SAFRON     = 0xFE983A,
  DARK_EBONY      = 0x422C02,
  CORN_HARVEST    = 0x8B6F01,
  OLD_GOLD        = 0xF1BD3C,
  BRANDY          = 0xFFD8AB,
  WINTER_HAZEL    = 0xFEE5A2,
  DEEP_GREEN      = 0x065E03,
  DARK_LIME_GREEN = 0x81D011,
  LEAFY_GREEN     = 0x4CDC48,
  BLUE_GEM        = 0x26178F,
  OCEAN           = 0x008189,
  BUTTERFLY_BLUE  = 0x3CBDFE,
  BARNEY_PURPLE   = 0xBC02BC,
  MAGENTA         = 0xFF00FF,
};

/* 4byte struct */
struct rgb {
  u8 r,g,b;
  u8 unused;
};

#endif /* COLORS_H */
