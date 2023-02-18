#include <SDL.h>
#include <SDL_video.h>
#include "base_types.h"

#include "colors.c"

enum { FALSE, TRUE };
enum { FAILURE = -1, SUCCESS };

#if 0
# define SCREEN_WIDTH  320 /* -- WIZ MAX WIDTH */
# define SCREEN_HEIGHT 240 /* -- WIZ MAX HEIGHT */
#else
# define SCREEN_WIDTH  200    /* approximately 10 times chimmy width */
# define SCREEN_HEIGHT 160    /* approximately 8 times chimmy height */
#endif

#define SCREEN_DEPTH  16     /* 16bpp is apparently the fastest on wiz? */

#define GAME_NAME "chimmy"

enum {
/* d-pad */
  WIZ_UP,               /* 0 */
  WIZ_UP_LEFT,
  WIZ_LEFT,
  WIZ_DOWN_LEFT,
  WIZ_DOWN,
  WIZ_DOWN_RIGHT,
  WIZ_RIGHT,
  WIZ_UP_RIGHT,
/* start+select */
  WIZ_MENU,             /* 8 */
  WIZ_SELECT,
/* shoulders */
  WIZ_L_BUTTON,         /* 10 */
  WIZ_R_BUTTON,
/* face buttons */
  WIZ_A,                /* 12 */
  WIZ_B,
  WIZ_X,
  WIZ_Y,
/* volume controls */
  WIZ_VOL_UP,           /* 16 */
  WIZ_VOL_DOWN,
/* -------------- */
  WIZ_BUTTON_COUNT,     /* 18 */
};

enum {
  BITMAP_CHIMMY,
/* ---------- */
  BITMAP_COUNT,
};

global SDL_Surface* texture[BITMAP_COUNT];

internal SDL_Surface*
bmp_load(const char* filename) {
  /* since the bmp is a 32bit image, it will be a different bpp than the screen.
   * this means that the texture will be unoptimized by default.
   * this isn't an issue on PC because we would always be using 32bpp
   */
  SDL_Surface* optimized_bmp;
  {
    SDL_Surface* raw_bmp = SDL_LoadBMP(filename);
    if (!raw_bmp) return NULL;
    optimized_bmp = SDL_DisplayFormat(raw_bmp);
    SDL_FreeSurface(raw_bmp);
  }

  return optimized_bmp;
}

internal SDL_Surface*
bmp_trans_load(const char* filename, u32 color) {
  SDL_Surface* optimized_bmp = bmp_load(filename);
  if (!optimized_bmp) return NULL;

  {
    struct rgb unpacked = rgb_unpack(MAGENTA);
    u32 key = SDL_MapRGB(optimized_bmp->format,
                         unpacked.r, unpacked.g, unpacked.b);
    SDL_SetColorKey(optimized_bmp, SDL_SRCCOLORKEY, key);
  }

  return optimized_bmp;
}

int
main(int argc, char ** argv) {
  SDL_Surface* screen = NULL;
  u32 flags = 0;

  flags = SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK;
  {
    u32 init_success = SDL_InitSubSystem(flags);
    if (init_success != 0) return FAILURE;
  }
  /* disable cursor before screen comes into play so that wiz doesn't show it at all */
  SDL_ShowCursor(SDL_DISABLE);

#if 0 /* only for desktop usage (win32/nix) */
  SDL_WM_SetIcon(SDL_LoadBMP("data/icon.bmp"), NULL); /* must be called before SetVideoMode */
  SDL_WM_SetCaption(GAME_NAME, NULL); /* this may need to be called after SDL_SetVideoMode */
#endif
  /* initialize the screen / window */
  screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT,
                            SCREEN_DEPTH, SDL_SWSURFACE);
  if (!screen) return FAILURE;
  { /* initialize joystick - wiz only? */
    SDL_Joystick* gamepad = NULL;
    gamepad = SDL_JoystickOpen(0);
    if (!gamepad) return FAILURE;
  }

  /* loading game resources */
  texture[BITMAP_CHIMMY] = bmp_trans_load("data/image/chimmy.bmp", MAGENTA);

  {
    SDL_Rect chimmy, clip;
    SDL_Event event;
    r32 x = 0, y = 0;
    r32 movespeed = 0.2;
    struct rgb bg_col = rgb_unpack(CLOUD);

    chimmy.x = (SCREEN_WIDTH / 4);
    chimmy.y = (SCREEN_HEIGHT / 2) - (chimmy.h / 2);
    chimmy.w = texture[BITMAP_CHIMMY]->w;
    chimmy.h = texture[BITMAP_CHIMMY]->h;
    clip.x = 20;
    clip.y = 0;
    clip.w = clip.h = 20;

    x = chimmy.x;
    y = chimmy.y;
    for (;;) {
      SDL_PollEvent(&event);
      switch (event.type) {
        case SDL_QUIT: { goto defer; }break;
        case SDL_JOYBUTTONDOWN: {
          switch (event.jbutton.button) {
            case WIZ_MENU: { goto defer; }break;
            case WIZ_UP: { y -= movespeed; }break;
            case WIZ_UP_LEFT: { y -= movespeed; x -= movespeed; }break;
            case WIZ_LEFT: { x -= movespeed; }break;
            case WIZ_DOWN_LEFT: { y += movespeed; x -= movespeed; }break;
            case WIZ_DOWN: { y += movespeed; }break;
            case WIZ_DOWN_RIGHT: { y += movespeed; x += movespeed; }break;
            case WIZ_RIGHT: { x += movespeed; }break;
            case WIZ_UP_RIGHT: { y -= movespeed; x += movespeed; }break;
            InvalidDefaultCase;
          }
        }break;
      }
      chimmy.x = x;
      chimmy.y = y;
      SDL_FillRect(screen, NULL,
                   SDL_MapRGB(screen->format, bg_col.r, bg_col.g, bg_col.b));
      SDL_BlitSurface(texture[BITMAP_CHIMMY], &clip, screen, &chimmy);
      SDL_Flip(screen);
    }
  }

/* Typically programs on desktop operating systems, memory is freed on application exit.
 * I'm currently assuming applications closing on the Wiz would be handled the same way.
 * My thought process for this is that the handheld is in essence running on GNU/Linux.
 */
defer:
  return SUCCESS;
}
