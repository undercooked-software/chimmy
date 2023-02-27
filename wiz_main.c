#include <SDL.h>
#include <SDL_video.h>
#include <string.h>

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

global i32 display_scale = 2;   /* this should get updated if the screen size gets altered */

#define GAME_NAME "chimmy"

enum {
  BITMAP_SMALLFONT,
  BITMAP_CHIMMY,
  BITMAP_INTERACTABLES,
  BITMAP_DRAGON,
  BITMAP_GUERNICA,
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

enum {
  INTERACTABLE_BANANA,
  INTERACTABLE_APPLE,
  INTERACTABLE_OCTAGON
};

enum {
  DRAGON_ANIM_NORMAL,
  DRAGON_ANIM_DEAD1,
  DRAGON_ANIM_DEAD2,
  DRAGON_ANIM_DESPAWN
};

enum {
  RECT_X,
  RECT_Y
};
enum {
  RECT_W,
  RECT_H
};

struct padded_u8 {
  u8 id;
  u8 unused[3];
}; /* 4byte */

struct renderable {
  b32 is_textured;        /* 4byte */
  union {
    struct padded_u8 texture;
    u32 color;
  } data;                 /* 4byte */
  SDL_Rect src;           /* ?? */
  SDL_Rect clip;          /* ?? */
};

#define WORLD_GUERNICA_ENTITIES (3)
global struct renderable guernica_entities[WORLD_GUERNICA_ENTITIES];

#define ENTITY_ARRAY(s) Join(Join(WORLD_,s),_ENTITIES)

struct world {
  u32 bg_col;                   /* 4byte */
  struct renderable* entities;  /* 4byte */
  struct world* prev;           /* 4byte */
  struct world* next;           /* 4byte */
};

/*
 * NOTE: perhaps we should consider having an array of worlds, or we should
 * be implementing some kind of macro to autogenerate these structs/names for us.
 */
global struct world*  world_list;
global struct world   world_start;
global struct world   world_guernica;
global struct world   world_boss;
global struct world   world_end;

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
  texture[BITMAP_SMALLFONT]     = bmp_trans_load("data/image/small-font.bmp", MAGENTA);
  texture[BITMAP_CHIMMY]        = bmp_trans_load("data/image/chimmy.bmp", MAGENTA);
  texture[BITMAP_INTERACTABLES] = bmp_trans_load("data/image/interactables.bmp", MAGENTA);
  texture[BITMAP_DRAGON]        = bmp_trans_load("data/image/dragon.bmp", MAGENTA);
  texture[BITMAP_GUERNICA]      = bmp_trans_load("data/image/guernica.bmp", MAGENTA);

  {
    i32 i;
    for (i = 0; i < WORLD_GUERNICA_ENTITIES; ++i) {
      guernica_entities[i].is_textured = TRUE;
      guernica_entities[i].data.texture.id = BITMAP_GUERNICA;
    }

    guernica_entities[0].src.x = 0;
    guernica_entities[0].src.y = 0;
    guernica_entities[0].src.w = 128;
    guernica_entities[0].src.h = 128;
    guernica_entities[0].clip.x = 0;
    guernica_entities[0].clip.y = 0;
    guernica_entities[0].clip.w = 128;
    guernica_entities[0].clip.h = 88;

    guernica_entities[1].src.x = 128;
    guernica_entities[1].src.y = 8;
    guernica_entities[1].src.w = 128;
    guernica_entities[1].src.h = 128;
    guernica_entities[1].clip.x = 0;
    guernica_entities[1].clip.y = 88;
    guernica_entities[1].clip.w = 32;
    guernica_entities[1].clip.h = 40;

    guernica_entities[2].src.x = 128;
    guernica_entities[2].src.y = 48;
    guernica_entities[2].src.w = 128;
    guernica_entities[2].src.h = 128;
    guernica_entities[2].clip.x = 32;
    guernica_entities[2].clip.y = 88;
    guernica_entities[2].clip.w = 32;
    guernica_entities[2].clip.h = 40;
  }


  /* construct the maps */
  world_start.bg_col = CLOUD;
  world_start.entities = NULL;
  world_start.prev = &world_end;
  world_start.next = &world_guernica;

  world_guernica.bg_col = BLACK;
  world_guernica.entities = guernica_entities;
  world_guernica.prev = NULL;
  world_guernica.next = NULL;
  /* world_guernica.next = &world_boss; */

  world_boss.bg_col = CRIMSON_RED;
  world_boss.entities = NULL;
  world_boss.prev = NULL;
  world_boss.next = &world_end;

  world_end.bg_col = BRITISH_RACING_GREEN;
  world_end.entities = NULL;
  world_end.prev = NULL;
  world_end.next = NULL;

  world_list = &world_start;

  {
    SDL_Rect chimmy, clip;
    SDL_Event event;
    r32 x = 0, y = 0;
    r32 movespeed = 0.2;
    struct rgb bg_col = rgb_unpack(world_list->bg_col);
    u32 frame_count = 0;
    u32 fps = 0;
    u32 fps_timer = SDL_GetTicks();
    u32 update_timer = SDL_GetTicks();

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
          /*
           * NOTE: this input method has some issues. particularly the fact that it will input
           * multiple instances of the same button press for a slight tap.
           * in some cases this is a good thing, in other areas it can be a source of issues.
           */
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
            case GP2X_BUTTON_L: {
              if (world_list->prev) {
                world_list = world_list->prev;
                bg_col = rgb_unpack(world_list->bg_col);
              }
            }break;
            case GP2X_BUTTON_R: {
              if (world_list->next) {
                world_list = world_list->next;
                bg_col = rgb_unpack(world_list->bg_col);
              }
            }break;
            InvalidDefaultCase;
          }
        }break;
        case SDL_JOYBUTTONUP: {
          InvalidDefaultCase;
        }break;
      }
      chimmy.x = x;
      chimmy.y = y;
      /*
       * NOTE: if you clear the backbuffer instead of the screen, while in interlaced scale mode
       * you can create some interesting motion blur style effects.
       */
      memset(screen->pixels, 0, screen->h * screen->pitch);
      /*
       * NOTE: it may be faster to memset to clear the backbuffer with the color of the map.
       * however, i'm not sure how to do that due to that since the depth is 16bit.
       */
      SDL_FillRect(backbuffer, NULL,
                   SDL_MapRGB(screen->format, bg_col.r, bg_col.g, bg_col.b));
      {
        i32 index;
        struct renderable* iter = world_list->entities;
        if (iter) {
          for (index = 0; index < ENTITY_ARRAY(GUERNICA); ++index, ++iter) {
            if (iter->is_textured)
              SDL_BlitSurface(texture[iter->data.texture.id], &iter->clip, backbuffer, &iter->src);
          }
        }
      }

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

      frame_count++;

      /* update every second? */
      if (SDL_GetTicks() - update_timer > 1000) {
        fps = frame_count / ((SDL_GetTicks() - fps_timer) / 1000);
        update_timer = SDL_GetTicks();
      }
    }
  }

/*
 * Typically programs on desktop operating systems, memory is freed on application exit.
 * I'm currently assuming applications closing on the Wiz would be handled the same way.
 * My thought process for this is that the handheld is in essence running on GNU/Linux.
 */
defer:
  return SUCCESS;
}
