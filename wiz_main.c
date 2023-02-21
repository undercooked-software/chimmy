#include <SDL.h>
#include <SDL_video.h>
#include "base_types.h"
#include "buttons.h"

#include "colors.c"
#include "surface.c"

enum { FALSE, TRUE };
enum { FAILURE = -1, SUCCESS };

#define SCREEN_WIDTH      320
#define SCREEN_HEIGHT     240
#define SCREEN_DEPTH      16    /* 16bpp is apparently the fastest on wiz? */
#define BACKBUFFER_WIDTH  160
#define BACKBUFFER_HEIGHT 120

global i32 display_scale = 2;

#define GAME_NAME "chimmy"

enum {
  BITMAP_CHIMMY,
/* ---------- */
  BITMAP_COUNT
};

global SDL_Surface* texture[BITMAP_COUNT];

enum {
  CHIMMY_ANIM_UP    = 0,
  CHIMMY_ANIM_RIGHT = 3,
  CHIMMY_ANIM_DOWN  = 8,
  CHIMMY_ANIM_LEFT  = 12,
  CHIMMY_ANIM_WIN   = 17,
  CHIMMY_ANIM_LOSE  = 18
};

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
    struct rgb unpacked = rgb_unpack(color);
    u32 key = SDL_MapRGB(optimized_bmp->format,
                         unpacked.r, unpacked.g, unpacked.b);
    SDL_SetColorKey(optimized_bmp, SDL_SRCCOLORKEY, key);
  }

  return optimized_bmp;
}

int
main(int argc, char ** argv) {
  SDL_Surface* screen = NULL;
  SDL_Surface* backbuffer = NULL;
  u32 scaling_method = SURFACE_SCALE_PROGRESSIVE;
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
  screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DEPTH, SDL_SWSURFACE);
  if (!screen) return FAILURE;
  backbuffer =
    SDL_CreateRGBSurface(SDL_SWSURFACE, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT, SCREEN_DEPTH,
                         screen->format->Rmask, screen->format->Gmask, screen->format->Rmask,
                         screen->format->Amask);
  if (!backbuffer) return FAILURE;
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

    clip.x = clip.y = 0;
    clip.w = clip.h = 16;
    chimmy.w = texture[BITMAP_CHIMMY]->w;
    chimmy.h = texture[BITMAP_CHIMMY]->h;
    chimmy.x = (BACKBUFFER_WIDTH / 4);
    chimmy.y = (BACKBUFFER_HEIGHT / 2) - (clip.h / 2);

    x = chimmy.x;
    y = chimmy.y;
    for (;;) {
      SDL_PollEvent(&event);
      switch (event.type) {
        case SDL_QUIT: { goto defer; }break;
        case SDL_JOYBUTTONDOWN: {
          switch (event.jbutton.button) {
            case GP2X_BUTTON_START: { goto defer; }break;
            case GP2X_BUTTON_UP: { y -= movespeed; }break;
            case GP2X_BUTTON_UPLEFT: { y -= movespeed; x -= movespeed; }break;
            case GP2X_BUTTON_LEFT: { x -= movespeed; }break;
            case GP2X_BUTTON_DOWNLEFT: { y += movespeed; x -= movespeed; }break;
            case GP2X_BUTTON_DOWN: { y += movespeed; }break;
            case GP2X_BUTTON_DOWNRIGHT: { y += movespeed; x += movespeed; }break;
            case GP2X_BUTTON_RIGHT: { x += movespeed; }break;
            case GP2X_BUTTON_UPRIGHT: { y -= movespeed; x += movespeed; }break;
            case GP2X_BUTTON_A: { scaling_method = SURFACE_SCALE_PROGRESSIVE; }break;
            case GP2X_BUTTON_Y: { scaling_method = SURFACE_SCALE_PROGRESSIVE2; }break;
            case GP2X_BUTTON_X: { scaling_method = SURFACE_SCALE_INTERLACED; }break;
            case GP2X_BUTTON_B: { scaling_method = SURFACE_SCALE_INTERLACED2; }break;
            InvalidDefaultCase;
          }
        }break;
        case SDL_JOYBUTTONUP: {
          InvalidDefaultCase;
        }break;
      }
      chimmy.x = x;
      chimmy.y = y;
      SDL_FillRect(screen, NULL, 0x000000);
      SDL_FillRect(backbuffer, NULL,
                   SDL_MapRGB(screen->format, bg_col.r, bg_col.g, bg_col.b));
      SDL_BlitSurface(texture[BITMAP_CHIMMY], &clip, backbuffer, &chimmy);
      switch (scaling_method) {
        case SURFACE_SCALE_PROGRESSIVE:{
          surface_progressive_scale(backbuffer, screen, display_scale);
        }break;
        case SURFACE_SCALE_PROGRESSIVE2:{
          surface_progressive_scale2(backbuffer, screen, display_scale);
        }break;
        case SURFACE_SCALE_INTERLACED:{
          surface_interlaced_scale(backbuffer, screen, display_scale);
        }break;
        case SURFACE_SCALE_INTERLACED2:{
          surface_interlaced_scale2(backbuffer, screen, display_scale);
        }break;
      }
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
