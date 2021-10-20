
// LINK: https://colors.artyclick.com/color-name-finder/
enum COLOR {
  BLACK = 0,
  CLOUD,
  WHITE,
  DARK_RED = 8,
  CRIMSON_RED,
  TOMATO,
  DEEP_SAFFRON,
  DARK_EBONY = 16,
  CORN_HARVEST,
  OLD_GOLD,
  BRANDY,
  WINTER_HAZEL,
  DEEP_GREEN = 24,
  DARK_LIME_GREEN,
  LEAFY_GREEN,
  BLUE_GEM = 32,
  OCEAN,
  BUTTERFLY_BLUE,
  BARNEY_PURPLE = 40
};

typedef struct RGB {
  u8 r, g, b;
} rgb;

global rgb color_key = { 0xFF, 0, 0xFF };