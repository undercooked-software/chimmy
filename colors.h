#ifndef COLORS_H
#define COLORS_H

#include "base_types.h"

/* URL: https://colors.artyclick.com/color-name-finder/ */
enum {
  BLACK                 = 0x000000, /* good */
  ASH_GREY              = 0xBBBBBB, /* good */
  CLOUD                 = 0xC4C4C4, /* good */
  SMOKEY_GREY           = 0x747474, /* good */
  STARDUST              = 0x9C9C9C, /* approximate */
  WHITE                 = 0xFFFFFF, /* good */
  REDWOOD               = 0x5E1901, /* fixed */
  CRIMSON_RED           = 0xA40001, /* approximate */
  TOMATO_RED            = 0xD92900, /* approximate */
  DEEP_SAFRON           = 0xFD9839, /* fixed */
  DARK_EBONY            = 0x412C01, /* fixed */
  MUDDY_BROWN           = 0x896F00, /* approximate */
  MACARONI_AND_CHEESE   = 0xF0BC3C, /* fixed */
  SUNSET                = 0xFCD8A9, /* approximate */
  SANDWISP              = 0xFBE4A0, /* fixed */
  BRITISH_RACING_GREEN  = 0x004F00, /* fixed */
  AO                    = 0x009400, /* approximate */
  APPLE_GREEN           = 0x81D010, /* approximate */
  FRESH_GREEN           = 0x5BD541, /* fixed */
  BLUE_GEM              = 0x25188D, /* approximate */
  OCEAN                 = 0x008187, /* fixed */
  CRYSTAL_BLUE          = 0x3CBCFC, /* approximate */
  BARNEY                = 0xBA01BB, /* approximate */
  PERSIAN_PINK          = 0xFB74B3, /* approximate */
  MAGENTA               = 0xFF00FF  /* good */
};

struct rgb {
  u8 r,g,b;
  u8 unused;
}; /* 4byte */

internal struct rgb rgb_unpack(u32);

#endif /* COLORS_H */
