#include <SDL/SDL.h>
#include <SDL/SDL_video.h>
#include <string.h>

#include "base_types.h"
#include "colors.h"
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

struct tex_data {
  u8 id;
  u8 anim;
  u8 unused[2];
}; /* 4byte */

struct renderable {
  b32 is_textured;        /* 4byte */
  union {
    struct tex_data texture;
    u32 color;
  } data;                 /* 4byte */
  SDL_Rect src;           /* 8byte */
  SDL_Rect clip;          /* 8byte */
};

struct entity {
  b32 is_textured;        /* 4byte */
  union {
    struct tex_data texture;
    u32 color;
  } data;                 /* 4byte */
  r32 x, y;               /* 8byte */
  SDL_Rect src;           /* 8byte */
  SDL_Rect clip;          /* 8byte */
  SDL_Rect crop_rect;     /* 8byte */
};

enum {
  COLLISION_SIDE_NONE,
  COLLISION_SIDE_LEFT,
  COLLISION_SIDE_TOP,
  COLLISION_SIDE_RIGHT,
  COLLISION_SIDE_BOTTOM
};

i32
entity_is_in_bounds(struct entity* e, struct display* display) {
  i32 collision_side = COLLISION_SIDE_NONE;

  if (e->y <= 0) {
    collision_side = COLLISION_SIDE_TOP;
  } else if (e->y + e->clip.h >= display->h) {
    collision_side = COLLISION_SIDE_BOTTOM;
  }

  if (e->x + e->clip.w <= 0) {
    collision_side = COLLISION_SIDE_LEFT;
  } else if (e->x - e->clip.w >= display->w) {
    collision_side = COLLISION_SIDE_RIGHT;
  }

  return collision_side;
}

void
entity_move(struct entity* e, struct display* display, r32 vx, r32 vy, r64 dt) {
  i32 collision_side = COLLISION_SIDE_NONE;

  if (vx == 0 && vy == 0) return;
  e->x += vx * dt;
  e->y += vy * dt;

  collision_side = entity_is_in_bounds(e, display);
  switch (collision_side) {
    case COLLISION_SIDE_TOP: { e->y = 0; }break;
    case COLLISION_SIDE_LEFT: { e->x = 0; }break;
    case COLLISION_SIDE_BOTTOM: {
      e->y = (display->h - e->clip.h);
    }break;
    case COLLISION_SIDE_RIGHT: {
      e->x = (display->w - e->clip.w);
    }break;
    InvalidDefaultCase;
  }

  e->src.x = e->x + 0.5;
  e->src.y = e->y + 0.5;
}

SDL_Rect*
entity_calculate_crop_bounds(struct entity* entity) {
  /* NOTE: I hate the way the texture width is stored */
  i32 texture_width = texture[entity->data.texture.id]->w;
  i32 texture_frame_width = texture_width / entity->clip.w;

  entity->crop_rect.x = (entity->data.texture.anim % texture_frame_width) * entity->clip.w;
  entity->crop_rect.y = (entity->data.texture.anim / texture_frame_width) * entity->clip.h;
  entity->crop_rect.w = entity->clip.w;
  entity->crop_rect.h = entity->clip.h;

  return &entity->crop_rect;
}

enum {
  WORLD_START,
  WORLD_HORSEMAN,
  WORLD_GUERNICA,
  WORLD_DOCK,
  WORLD_BOSS,
  WORLD_END,
  WORLD_COUNT
};

#define WORLD_START_ENTITIES (1)
#define WORLD_HORSEMAN_ENTITIES (1)
#define WORLD_DOCK_ENTITIES (10)
#define WORLD_GUERNICA_ENTITIES (3)
global struct renderable world_start_entities[WORLD_START_ENTITIES];
global struct renderable world_horseman_entities[WORLD_HORSEMAN_ENTITIES];
global struct renderable world_guernica_entities[WORLD_GUERNICA_ENTITIES];
global struct renderable world_dock_entities[WORLD_DOCK_ENTITIES];

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

  setenv("SDL_VIDEO_WAYLAND_WMCLASS", WM_CLASS, 0);
  setenv("SDL_VIDEO_X11_WMCLASS",     WM_CLASS, 0);
  setenv("SDL_VIDEO_CENTERED",        "",       0);

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
    display.screen_backbuffer =
      SDL_CreateRGBSurface(SDL_SWSURFACE, display.w, display.h, SCREEN_DEPTH,
                           fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
    if (!display.screen_backbuffer) return FAILURE;
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

  if (display.scale > 1) {
    texture[BITMAP_CHIMMY]      = bmp_scale(texture[BITMAP_CHIMMY], MAGENTA, display.scale);
  }

  {
    world_start_entities[0].data.color = MUDDY_BROWN;
    world_start_entities[0].src.x = 0;
    world_start_entities[0].src.y = 44;
    world_start_entities[0].src.w = BACKBUFFER_WIDTH;
    world_start_entities[0].src.h = 32;
  }

  {
    world_horseman_entities[0].data.color = DARK_EBONY;
    world_horseman_entities[0].src.x = 0;
    world_horseman_entities[0].src.y = 44;
    world_horseman_entities[0].src.w = BACKBUFFER_WIDTH;
    world_horseman_entities[0].src.h = 32;
  }

  {
    local_persist i32 entity_count;
    world_dock_entities[0].data.color = SANDWISP;
    world_dock_entities[0].src.x = 0;
    world_dock_entities[0].src.y = 0;
    world_dock_entities[0].src.w = 32;
    world_dock_entities[0].src.h = BACKBUFFER_HEIGHT;
    entity_count++;
    {
      i32 i;
      for (i = entity_count; i < entity_count+5; ++i) {
        world_dock_entities[i].data.color = DARK_EBONY;
        world_dock_entities[i].src.x = 58 - ((i-1) * 16);
        world_dock_entities[i].src.y = 44;
        world_dock_entities[i].src.w = 13;
        world_dock_entities[i].src.h = 32;
      }
      entity_count += 5;
      for (i = entity_count; i < entity_count+4; ++i) {
        world_dock_entities[i].data.color = DARK_EBONY;
        world_dock_entities[i].src.x = 54 - ((i-entity_count) * 16);
        world_dock_entities[i].src.y = 44;
        world_dock_entities[i].src.w = 3;
        world_dock_entities[i].src.h = 52;
      }
      entity_count += 4;
    }
  }

  {
    i32 i;
    for (i = 0; i < WORLD_GUERNICA_ENTITIES; ++i) {
      world_guernica_entities[i].is_textured = TRUE;
      world_guernica_entities[i].data.texture.id = BITMAP_GUERNICA;
    }

    world_guernica_entities[0].src.x = 0;   /* entity start position x */
    world_guernica_entities[0].src.y = 0;   /* entity start position y */
    world_guernica_entities[0].src.w = 128; /* texture width */
    world_guernica_entities[0].src.h = 128; /* texture height */
    world_guernica_entities[0].clip.x = 0;  /* clip texture x position */
    world_guernica_entities[0].clip.y = 0;  /* clip texture y position */
    world_guernica_entities[0].clip.w = 128;/* clip width */
    world_guernica_entities[0].clip.h = 88; /* clip height */

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
  worlds[WORLD_START].entity_count = WORLD_START_ENTITIES;
  worlds[WORLD_HORSEMAN].bg_col = BRITISH_RACING_GREEN;
  worlds[WORLD_HORSEMAN].entities = world_horseman_entities;
  worlds[WORLD_HORSEMAN].entity_count = WORLD_HORSEMAN_ENTITIES;
  worlds[WORLD_GUERNICA].bg_col = BLACK;
  worlds[WORLD_GUERNICA].entities = world_guernica_entities;
  worlds[WORLD_GUERNICA].entity_count = WORLD_GUERNICA_ENTITIES;
  worlds[WORLD_DOCK].bg_col = OCEAN;
  worlds[WORLD_DOCK].entities = world_dock_entities;
  worlds[WORLD_DOCK].entity_count = WORLD_DOCK_ENTITIES;
  worlds[WORLD_BOSS].bg_col = CRIMSON_RED;
  worlds[WORLD_BOSS].entities = NULL;
  worlds[WORLD_BOSS].entity_count = 0;
  worlds[WORLD_END].bg_col = BRITISH_RACING_GREEN;
  worlds[WORLD_END].entities = NULL;
  worlds[WORLD_END].entity_count = 0;

  {
    SDL_Event event;
    struct entity chimmy;
    u8 world_index = WORLD_START;
    r32 vx = 0.f, vy = 0.f;
    u32 move_speed = 40.f * display.scale;
    struct rgb bg_col = rgb_unpack(worlds[world_index].bg_col);
    /* r32 target_seconds_per_frame = 1.f / 60; */
    u32 frame_count = 0;
    u32 fps = 0;
    u32 fps_timer = SDL_GetTicks();
    u32 update_timer = SDL_GetTicks();
    u32 NOW = 0, LAST = 0;
    r32 dt = 0.f;

    chimmy.is_textured = TRUE;
    chimmy.data.texture.id = BITMAP_CHIMMY;
    chimmy.data.texture.anim = 0;
    chimmy.clip.x = chimmy.clip.y = 0;
    chimmy.clip.w = chimmy.clip.h = 16 * display.scale;
    chimmy.src.w = texture[BITMAP_CHIMMY]->w;
    chimmy.src.h = texture[BITMAP_CHIMMY]->h;
    chimmy.src.x = (display.w / 4);
    chimmy.src.y = (display.h / 2) - (chimmy.clip.h / 2);

    chimmy.x = chimmy.src.x;
    chimmy.y = chimmy.src.y;

    for (;;) {
      LAST = NOW;
      NOW = SDL_GetTicks();
      dt = (NOW - LAST) / 1000.f;

      while (SDL_PollEvent(&event)) {
        switch (event.type) {
          case SDL_QUIT: { goto defer; }break;
          case SDL_KEYDOWN: {
            switch (event.key.keysym.sym) {
              case SDLK_ESCAPE: { goto defer; }break;
              case SDLK_w: { vy -= move_speed; }break;
              case SDLK_s: { vy += move_speed; }break;
              case SDLK_a: { vx -= move_speed; }break;
              case SDLK_d: { vx += move_speed; }break;
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
            }
          }break;
          case SDL_KEYUP: {
            switch (event.key.keysym.sym) {
              case SDLK_w: { vy += move_speed; }break;
              case SDLK_s: { vy -= move_speed; }break;
              case SDLK_a: { vx += move_speed; }break;
              case SDLK_d: { vx -= move_speed; }break;
              InvalidDefaultCase;
            }
          }break;
        }
      }

      entity_move(&chimmy, &display, vx, vy, dt);

      /*
       * NOTE: if you clear the backbuffer instead of the screen, while in interlaced scale mode
       * you can create some interesting motion blur style effects.
       */
      memset(display.screen->pixels, 0, display.screen->h * display.screen->pitch);
      memset(display.screen_backbuffer->pixels, 0, display.screen->h * display.screen->pitch);
      SDL_FillRect(display.backbuffer, NULL,
                   SDL_MapRGB(display.backbuffer->format, bg_col.r, bg_col.g, bg_col.b));

      {
        i32 index;
        struct renderable* iter = worlds[world_index].entities;
        if (iter) {
          for (index = 0; index < worlds[world_index].entity_count; ++index, ++iter) {
            if (iter->is_textured)
              SDL_BlitSurface(texture[iter->data.texture.id], &iter->clip, display.backbuffer, &iter->src);
            else
              SDL_FillRect(display.backbuffer, &iter->src, iter->data.color);
          }
        }
      }

      if (display.scale > 1) {
        switch (scaling_method) {
          case SURFACE_SCALE_PROGRESSIVE:{
            surface_progressive_scale(display.backbuffer, display.screen_backbuffer, display.scale);
          }break;
          case SURFACE_SCALE_PROGRESSIVE2:{
            surface_progressive_scale2(display.backbuffer, display.screen_backbuffer, display.scale);
          }break;
          case SURFACE_SCALE_INTERLACED:{
            surface_interlaced_scale(display.backbuffer, display.screen_backbuffer, display.scale);
          }break;
          case SURFACE_SCALE_INTERLACED2:{
            surface_interlaced_scale2(display.backbuffer, display.screen_backbuffer, display.scale);
          }break;
        }
      } else {
        SDL_BlitSurface(display.backbuffer, NULL, display.screen_backbuffer, NULL);
      }

      SDL_BlitSurface(texture[chimmy.data.texture.id],
                      entity_calculate_crop_bounds(&chimmy), display.screen_backbuffer, &chimmy.src);

      SDL_BlitSurface(display.screen_backbuffer, NULL, display.screen, NULL);
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
