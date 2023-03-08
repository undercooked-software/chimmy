#include <SDL/SDL.h>
#include <SDL/SDL_video.h>
#include <string.h>

#include "base_types.h"
#include "internals.h"
#include "display.h"
#include "surface.h"
#include "buttons.h"

#include "colors.c"
#include "surface.c"

enum { FALSE, TRUE };
enum { FAILURE = -1, SUCCESS };

enum {
  BITMAP_SMALLFONT,
  BITMAP_CHIMMY,
  BITMAP_INTERACTABLES,
  BITMAP_TREES,
  BITMAP_GUERNICA,
  BITMAP_KNIGHT,
  BITMAP_HORSEMAN,
  BITMAP_WIP_COWBOY,
  BITMAP_DRAGON,

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
  INTERACTABLE_GRAVESTONE, /* not actually interactable. just fitting in the sheet */
  INTERACTABLE_OCTAGON
};

enum {
  DRAGON_ANIM_NORMAL,
  DRAGON_ANIM_DEAD1,
  DRAGON_ANIM_DEAD2,
  DRAGON_ANIM_DESPAWN
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

enum {
  WORLD_START,
  WORLD_GUERNICA,
  WORLD_BOSS,
  WORLD_END,

  WORLD_COUNT
};

#define WORLD_START_ENTITIES (7)
#define WORLD_GUERNICA_ENTITIES (3)
global struct renderable world_start_entities[WORLD_START_ENTITIES];
global struct renderable world_guernica_entities[WORLD_GUERNICA_ENTITIES];

#define ENTITY_ARRAY(s) Join(Join(WORLD_,s),_ENTITIES)

struct world {
  u32 bg_col;                   /* 4byte */
  struct renderable* entities;  /* 4byte */
  u8 entity_count;              /* 1byte */
};

global struct world worlds[WORLD_COUNT];

int
main(int argc, char** argv) {
  struct display display;
  u32 flags = 0;
  u32 scaling_method = SURFACE_SCALE_PROGRESSIVE;

  display.scale = 4;
  display.w = BACKBUFFER_WIDTH * display.scale;
  display.h = BACKBUFFER_HEIGHT * display.scale;

  /* setenv("SDL_VIDEO_WAYLAND_WMCLASS", "com.example.chimmy", 0); */
  /* setenv("SDL_VIDEO_X11_WMCLASS", "com.example.chimmy", 0); */

  flags = SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK;
  {
    u32 init_success = SDL_InitSubSystem(flags);
    if (init_success != 0) return FAILURE;
  }
  /* disable cursor before screen comes into play so that wiz doesn't show it at all */
  SDL_ShowCursor(SDL_DISABLE);

  /* SDL_WM_SetIcon(SDL_LoadBMP("data/icon.bmp"), NULL); /1* must be called before SetVideoMode *1/ */
  SDL_WM_SetCaption(GAME_NAME, NULL); /* this may need to be called after SDL_SetVideoMode */

  /* initialize the screen / window */
  display.screen = SDL_SetVideoMode(display.w, display.h, SCREEN_DEPTH, SDL_SWSURFACE);
  if (!display.screen) return FAILURE;
  {
    SDL_PixelFormat* fmt = display.screen->format;
    display.backbuffer =
      SDL_CreateRGBSurface(SDL_SWSURFACE, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT, SCREEN_DEPTH,
                           fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
    if (!display.backbuffer) return FAILURE;
  }
  { /* initialize joystick - wiz only? */
    SDL_Joystick* gamepad = NULL;
    gamepad = SDL_JoystickOpen(0);
    /* NOTE: Don't fail here. Just produce a flag and continue */
  }

  /* loading game resources */
  texture[BITMAP_SMALLFONT]     = bmp_trans_load("data/image/small-font.bmp", MAGENTA, LOAD_BITMAP_PREOPTIMIZED);
  texture[BITMAP_CHIMMY]        = bmp_trans_load("data/image/chimmy.bmp", MAGENTA, LOAD_BITMAP_PREOPTIMIZED);
  texture[BITMAP_INTERACTABLES] = bmp_trans_load("data/image/interactables.bmp", MAGENTA, LOAD_BITMAP_PREOPTIMIZED);
  texture[BITMAP_GUERNICA]      = bmp_load("data/image/guernica.bmp", LOAD_BITMAP_PREOPTIMIZED);
  texture[BITMAP_KNIGHT]        = bmp_trans_load("data/image/knight.bmp", MAGENTA, LOAD_BITMAP_PREOPTIMIZED);
  texture[BITMAP_HORSEMAN]      = bmp_trans_load("data/image/horseman.bmp", MAGENTA, LOAD_BITMAP_PREOPTIMIZED);
  texture[BITMAP_WIP_COWBOY]    = bmp_trans_load("data/image/cowboy.bmp", MAGENTA, LOAD_BITMAP_PREOPTIMIZED);
  texture[BITMAP_DRAGON]        = bmp_trans_load("data/image/dragon.bmp", MAGENTA, LOAD_BITMAP_PREOPTIMIZED);

  {
    i32 i;
    for (i = 0; i < WORLD_START_ENTITIES; ++i) {
      world_start_entities[i].is_textured = TRUE;
      world_start_entities[i].data.texture.id = BITMAP_TREES;
    }
  }

  {
    i32 i;
    for (i = 0; i < WORLD_GUERNICA_ENTITIES; ++i) {
      world_guernica_entities[i].is_textured = TRUE;
      world_guernica_entities[i].data.texture.id = BITMAP_GUERNICA;
    }

    world_guernica_entities[0].src.x = 0;
    world_guernica_entities[0].src.y = 0;
    world_guernica_entities[0].src.w = 128;
    world_guernica_entities[0].src.h = 128;
    world_guernica_entities[0].clip.x = 0;
    world_guernica_entities[0].clip.y = 0;
    world_guernica_entities[0].clip.w = 128;
    world_guernica_entities[0].clip.h = 88;

    world_guernica_entities[1].src.x = 128;
    world_guernica_entities[1].src.y = 8;
    world_guernica_entities[1].src.w = 128;
    world_guernica_entities[1].src.h = 128;
    world_guernica_entities[1].clip.x = 0;
    world_guernica_entities[1].clip.y = 88;
    world_guernica_entities[1].clip.w = 32;
    world_guernica_entities[1].clip.h = 40;

    world_guernica_entities[2].src.x = 128;
    world_guernica_entities[2].src.y = 48;
    world_guernica_entities[2].src.w = 128;
    world_guernica_entities[2].src.h = 128;
    world_guernica_entities[2].clip.x = 32;
    world_guernica_entities[2].clip.y = 88;
    world_guernica_entities[2].clip.w = 32;
    world_guernica_entities[2].clip.h = 40;
  }


  /* construct the maps */
  worlds[WORLD_START].bg_col = CLOUD;
  worlds[WORLD_START].entities = world_start_entities;
  worlds[WORLD_START].entity_count = 0;
  worlds[WORLD_GUERNICA].bg_col = BLACK;
  worlds[WORLD_GUERNICA].entities = world_guernica_entities;
  worlds[WORLD_GUERNICA].entity_count = WORLD_GUERNICA_ENTITIES;
  worlds[WORLD_BOSS].bg_col = CRIMSON_RED;
  worlds[WORLD_BOSS].entities = NULL;
  worlds[WORLD_BOSS].entity_count = 0;
  worlds[WORLD_END].bg_col = BRITISH_RACING_GREEN;
  worlds[WORLD_END].entities = NULL;
  worlds[WORLD_END].entity_count = 0;

  {
    SDL_Rect chimmy, clip;
    SDL_Event event;
    u8 world_index = WORLD_GUERNICA;
    r32 x = 0, y = 0;
    r32 movespeed = 0.2;
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
      }
      chimmy.x = x;
      chimmy.y = y;
      /*
       * NOTE: if you clear the backbuffer instead of the screen, while in interlaced scale mode
       * you can create some interesting motion blur style effects.
       */
      memset(display.screen->pixels, 0, display.screen->h * display.screen->pitch);
      memset(display.backbuffer->pixels, worlds[world_index].bg_col,
             display.backbuffer->h * display.backbuffer->pitch);

      {
        i32 index;
        struct renderable* iter = worlds[world_index].entities;
        if (iter) {
          for (index = 0; index < worlds[world_index].entity_count; ++index, ++iter) {
            if (iter->is_textured)
              SDL_BlitSurface(texture[iter->data.texture.id], &iter->clip, display.backbuffer, &iter->src);
          }
        }
      }

      SDL_BlitSurface(texture[BITMAP_CHIMMY], &clip, display.backbuffer, &chimmy);

      switch (scaling_method) {
        case SURFACE_SCALE_PROGRESSIVE:{
          surface_progressive_scale(display.backbuffer, display.screen, display.scale);
        }break;
        case SURFACE_SCALE_PROGRESSIVE2:{
          surface_progressive_scale2(display.backbuffer, display.screen, display.scale);
        }break;
        case SURFACE_SCALE_INTERLACED:{
          surface_interlaced_scale(display.backbuffer, display.screen, display.scale);
        }break;
        case SURFACE_SCALE_INTERLACED2:{
          surface_interlaced_scale2(display.backbuffer, display.screen, display.scale);
        }break;
      }
      SDL_Flip(display.screen);

      frame_count++;

      /* update every second? */
      if (SDL_GetTicks() - update_timer > 1000) {
        fps = frame_count / ((SDL_GetTicks() - fps_timer) / 1000);
        printf("FPS: %i\n", fps);
        update_timer = SDL_GetTicks();
      }
    }
  }

defer:
  return SUCCESS;
}
