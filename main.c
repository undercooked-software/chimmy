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
  CHIMMY_ANIM_DOWN  = 6,
  CHIMMY_ANIM_LEFT  = 9,
  CHIMMY_ANIM_WIN   = 12,
  CHIMMY_ANIM_LOSE  = 13
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

void
move_entity(struct renderable* entity, r32 vel_x, r32 vel_y, r64 dt) {
  entity->src.x = vel_x;
  entity->src.y = vel_y;
}

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

  /* setenv("SDL_VIDEO_WAYLAND_WMCLASS", WM_CLASS, 0); */
  /* setenv("SDL_VIDEO_X11_WMCLASS", WM_CLASS, 0); */

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
  worlds[WORLD_START].bg_col = AO;
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
    SDL_Event event;
    struct renderable chimmy;
    u8 world_index = WORLD_START;
    r32 x = 0.f, y = 0.f;
    r32 vel_x = 0.f, vel_y = 0.f;
    r32 move_speed = 0.05f;
    struct rgb bg_col = rgb_unpack(worlds[world_index].bg_col);
    u32 frame_count = 0;
    u32 fps = 0;
    u32 fps_timer = SDL_GetTicks();
    u32 update_timer = SDL_GetTicks();
    u32 NOW = 0, LAST = 0;
    r32 dt = 0.f;

    chimmy.is_textured = TRUE;
    chimmy.data.texture.id = BITMAP_CHIMMY;
    chimmy.clip.x = chimmy.clip.y = 0;
    chimmy.clip.w = chimmy.clip.h = 16;
    chimmy.src.w = texture[BITMAP_CHIMMY]->w;
    chimmy.src.h = texture[BITMAP_CHIMMY]->h;
    chimmy.src.x = (BACKBUFFER_WIDTH / 4);
    chimmy.src.y = (BACKBUFFER_HEIGHT / 2) - (chimmy.clip.h / 2);

    x = chimmy.src.x;
    y = chimmy.src.y;

    for (;;) {
      LAST = NOW;
      NOW = SDL_GetTicks();
      dt = (NOW - LAST);

      while (SDL_PollEvent(&event)) {
        switch (event.type) {
          case SDL_QUIT: { goto defer; }break;
          case SDL_KEYDOWN: {
            switch (event.key.keysym.sym) {
              case SDLK_ESCAPE: { goto defer; }break;
              case SDLK_w: { vel_y -= move_speed; }break;
              case SDLK_s: { vel_y += move_speed; }break;
              case SDLK_a: { vel_x -= move_speed; }break;
              case SDLK_d: { vel_x += move_speed; }break;
              case SDLK_q: {
                if (world_index == WORLD_START) { world_index = WORLD_END; }
                else { world_index--; }
                bg_col = rgb_unpack(worlds[world_index].bg_col);
              }break;
              case SDLK_e: {
                if (world_index != WORLD_END) { world_index++; }
                bg_col = rgb_unpack(worlds[world_index].bg_col);
              }break;
              InvalidDefaultCase;
            };
          }break;
          case SDL_KEYUP: {
            switch (event.key.keysym.sym) {
              case SDLK_w: { vel_y += move_speed; }break;
              case SDLK_s: { vel_y -= move_speed; }break;
              case SDLK_a: { vel_x += move_speed; }break;
              case SDLK_d: { vel_x -= move_speed; }break;
              InvalidDefaultCase;
            };
          }break;
        }
      }

      x += vel_x * dt;
      y += vel_y * dt;

      if (y <= 0) {
        y = 0;
      } else if (y + chimmy.clip.h >= BACKBUFFER_HEIGHT) {
        y = (BACKBUFFER_HEIGHT - chimmy.clip.h);
      }

      if (world_index == WORLD_START) {
        if (x + chimmy.clip.w <= 0) {
          world_index = WORLD_END;
          bg_col = rgb_unpack(worlds[world_index].bg_col);
          x = BACKBUFFER_WIDTH - chimmy.clip.w;
        }
      } else {
        if (x <= 0) { x = 0; }
      }

      if (x - chimmy.clip.x >= BACKBUFFER_WIDTH) {
        world_index++;
        bg_col = rgb_unpack(worlds[world_index].bg_col);
        x = 0;
      }

      move_entity(&chimmy, x, y, dt);

      /*
       * NOTE: if you clear the backbuffer instead of the screen, while in interlaced scale mode
       * you can create some interesting motion blur style effects.
       */
      memset(display.screen->pixels, 0, display.screen->h * display.screen->pitch);
      SDL_FillRect(display.backbuffer, NULL,
                   SDL_MapRGB(display.backbuffer->format, bg_col.r, bg_col.g, bg_col.b));

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

      SDL_BlitSurface(texture[chimmy.data.texture.id], &chimmy.clip, display.backbuffer, &chimmy.src);

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

      SDL_Delay(1); /* We should properly cap the FPS */
    }
  }

defer:
  return SUCCESS;
}
