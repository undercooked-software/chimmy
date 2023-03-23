#include <stdio.h>  /* sprintf */
#include <string.h> /* memset */
#include <SDL.h>
#include <SDL_video.h>

#include "base_types.h"
#include "internals.h"

#include "colors.h"
#include "font.h"
#include "SDL_display.h"

#include "colors.c"
#include "surface.c"
#include "SDL_text.c"
#include "SDL_QOI.c"

enum {
  BITMAP_SMALLFONT,
  BITMAP_CHIMMY,
  BITMAP_INTERACTABLES,
  BITMAP_TREES,
  BITMAP_BOAT,
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
  INTERACTABLE_OCTAGON,
  INTERACTABLE_GRAVESTONE /* not actually interactable. just fitting in the sheet */
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
};

struct renderable {
  b32 is_textured;
  union {
    struct tex_data texture;
    u32 color;
  } data;
  SDL_Rect src;
  SDL_Rect clip;
};

struct entity {
  b32 is_textured;
  union {
    struct tex_data texture;
    u32 color;
  } data;
  r32 x, y;
  i32 move_speed;
  SDL_Rect src;
  SDL_Rect clip;

  void* action_data;
  void (*action)(struct entity*, r64);
};

struct line {
  i32 p1, p2;
};

enum {
  COLLISION_SIDE_NONE,
  COLLISION_SIDE_LEFT,
  COLLISION_SIDE_TOP,
  COLLISION_SIDE_RIGHT,
  COLLISION_SIDE_BOTTOM
};

i32
entity_is_in_bounds(struct entity* e, struct SDL_Display* display) {
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
entity_move(struct entity* e, struct SDL_Display* display, r32 vx, r32 vy, r64 dt) {
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

global SDL_Rect crop_rect;

SDL_Rect*
entity_calculate_crop_bounds(struct entity* entity) {
  /* NOTE: I hate the way the texture width is stored */
  i32 texture_width = texture[entity->data.texture.id]->w;
  i32 texture_frame_width = texture_width / entity->clip.w;

  crop_rect.x = (entity->data.texture.anim % texture_frame_width) * entity->clip.w;
  crop_rect.y = (entity->data.texture.anim / texture_frame_width) * entity->clip.h;
  crop_rect.w = entity->clip.w;
  crop_rect.h = entity->clip.h;

  return &crop_rect;
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

#define WORLD_START_OBJECTS (1)
global struct renderable world_start_objects[WORLD_START_OBJECTS];
#define WORLD_HORSEMAN_OBJECTS (1)
global struct renderable world_horseman_objects[WORLD_HORSEMAN_OBJECTS];
#define WORLD_GUERNICA_OBJECTS (3)
global struct renderable world_guernica_objects[WORLD_GUERNICA_OBJECTS];
#define WORLD_DOCK_OBJECTS (10)
#define WORLD_DOCK_ENTITIES (1)
global struct renderable world_dock_objects[WORLD_DOCK_OBJECTS];
global struct entity world_dock_entities[WORLD_DOCK_ENTITIES];
#define WORLD_BOSS_ENTITIES (2)
global struct entity world_boss_entities[WORLD_BOSS_ENTITIES];

struct world {
  u32 bg_col;
  struct renderable* objects;
  u8 object_count;
  struct entity* entities;
  u8 entity_count;
};

global struct world worlds[WORLD_COUNT];

u64
average(r64* vals, u32 count) {
  u64 accumulator = 0;
  i32 i;
  for (i = 0; i < count; ++i) { accumulator += vals[i]; }
  return accumulator / count;
}

u32
fill_color(SDL_PixelFormat* fmt, u32 color) {
  struct rgb unpacked;
  unpacked = rgb_unpack(color);
  return SDL_MapRGB(fmt, unpacked.r, unpacked.g, unpacked.b);
}

void
horizontal_bounds_movement(struct entity* e, r64 dt) {
  struct line* line = (struct line*)e->action_data;

  e->x += e->move_speed * dt;
  if (e->x < line->p1) {
    e->move_speed *= -1;
    e->x = line->p1;
  }
  else if (e->x + e->clip.w > line->p2) {
    e->move_speed *= -1;
    e->x = line->p2 - e->clip.w;
  }
  e->src.x = e->x;
}

int
main(int argc, char** argv) {
  struct SDL_Display display;
  u32 flags = 0;

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
      SDL_CreateRGBSurface(SDL_SWSURFACE, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT, fmt->BitsPerPixel,
                           fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
    if (!display.backbuffer) return FAILURE;
  }
  { /* initialize joystick - wiz only? */
    SDL_Joystick* gamepad = NULL;
    gamepad = SDL_JoystickOpen(0);
    if (!gamepad) {
      /* NOTE: Don't fail here. Just produce a flag and continue */
    }
  }

  /* loading game resources */
  texture[BITMAP_SMALLFONT]     = SDL_LoadQOI("data/image/small-font.qoi");
  texture[BITMAP_CHIMMY]        = SDL_LoadQOI("data/image/chimmy.qoi");
  texture[BITMAP_INTERACTABLES] = SDL_LoadQOI("data/image/interactables.qoi");
  texture[BITMAP_BOAT]          = SDL_LoadQOI("data/image/boat.qoi");
  texture[BITMAP_GUERNICA]      = SDL_LoadQOI("data/image/guernica.qoi");
  texture[BITMAP_KNIGHT]        = SDL_LoadQOI("data/image/knight.qoi");
  texture[BITMAP_HORSEMAN]      = SDL_LoadQOI("data/image/horseman.qoi");
  texture[BITMAP_WIP_COWBOY]    = SDL_LoadQOI("data/image/cowboy.qoi");
  texture[BITMAP_DRAGON]        = SDL_LoadQOI("data/image/dragon.qoi");

  if (display.scale > 1) {
    texture[BITMAP_SMALLFONT]     = SDL_ScaleSurface(texture[BITMAP_SMALLFONT], MAGENTA, display.scale);
    texture[BITMAP_CHIMMY]        = SDL_ScaleSurface(texture[BITMAP_CHIMMY], MAGENTA, display.scale);
    texture[BITMAP_INTERACTABLES] = SDL_ScaleSurface(texture[BITMAP_INTERACTABLES], MAGENTA, display.scale);
    texture[BITMAP_BOAT]          = SDL_ScaleSurface(texture[BITMAP_BOAT], MAGENTA, display.scale);
    texture[BITMAP_KNIGHT]        = SDL_ScaleSurface(texture[BITMAP_KNIGHT], MAGENTA, display.scale);
    texture[BITMAP_HORSEMAN]      = SDL_ScaleSurface(texture[BITMAP_HORSEMAN], MAGENTA, display.scale);
    texture[BITMAP_DRAGON]        = SDL_ScaleSurface(texture[BITMAP_DRAGON], MAGENTA, display.scale);
  }

  {
    world_start_objects[0].data.color = MUDDY_BROWN;
    world_start_objects[0].src.x = 0;
    world_start_objects[0].src.y = 44;
    world_start_objects[0].src.w = BACKBUFFER_WIDTH;
    world_start_objects[0].src.h = 32;
  }

  {
    world_horseman_objects[0].data.color = DARK_EBONY;
    world_horseman_objects[0].src.x = 0;
    world_horseman_objects[0].src.y = 44;
    world_horseman_objects[0].src.w = BACKBUFFER_WIDTH;
    world_horseman_objects[0].src.h = 32;
  }

  {
    local_persist i32 object_count;
    world_dock_objects[0].data.color = SANDWISP;
    world_dock_objects[0].src.x = 0;
    world_dock_objects[0].src.y = 0;
    world_dock_objects[0].src.w = 32;
    world_dock_objects[0].src.h = BACKBUFFER_HEIGHT;
    object_count++;
    {
      i32 i;
      for (i = object_count; i < object_count+5; ++i) {
        world_dock_objects[i].data.color = DARK_EBONY;
        world_dock_objects[i].src.x = 58 - ((i-1) * 16);
        world_dock_objects[i].src.y = 44;
        world_dock_objects[i].src.w = 13;
        world_dock_objects[i].src.h = 32;
      }
      object_count += 5;
      for (i = object_count; i < object_count+4; ++i) {
        world_dock_objects[i].data.color = DARK_EBONY;
        world_dock_objects[i].src.x = 54 - ((i-object_count) * 16);
        world_dock_objects[i].src.y = 44;
        world_dock_objects[i].src.w = 3;
        world_dock_objects[i].src.h = 52;
      }
      object_count += 4;
    }

    world_dock_entities[0].is_textured = TRUE;
    world_dock_entities[0].data.texture.id = BITMAP_BOAT;
    world_dock_entities[0].src.x = 101 * display.scale;
    world_dock_entities[0].src.y = 73 * display.scale;
    /* world_dock_entities[0].src.w = 64; */
    /* world_dock_entities[0].src.h = 64; */
    world_dock_entities[0].clip.x = 0;
    world_dock_entities[0].clip.y = 0;
    world_dock_entities[0].clip.w = 47 * display.scale;
    world_dock_entities[0].clip.h = 46 * display.scale;
  }

  {
    i32 i;
    for (i = 0; i < WORLD_GUERNICA_OBJECTS; ++i) {
      world_guernica_objects[i].is_textured = TRUE;
      world_guernica_objects[i].data.texture.id = BITMAP_GUERNICA;
    }

    world_guernica_objects[0].src.x = 0;   /* object start position x */
    world_guernica_objects[0].src.y = 0;   /* object start position y */
    /* world_guernica_objects[0].src.w = 128; /1* texture width *1/ */
    /* world_guernica_objects[0].src.h = 128; /1* texture height *1/ */
    world_guernica_objects[0].clip.x = 0;  /* clip texture x position */
    world_guernica_objects[0].clip.y = 0;  /* clip texture y position */
    world_guernica_objects[0].clip.w = 128;/* clip width */
    world_guernica_objects[0].clip.h = 88; /* clip height */

    world_guernica_objects[1].src.x = 128;
    world_guernica_objects[1].src.y = 8;
    /* world_guernica_objects[1].src.w = 128; */
    /* world_guernica_objects[1].src.h = 128; */
    world_guernica_objects[1].clip.x = 0;
    world_guernica_objects[1].clip.y = 88;
    world_guernica_objects[1].clip.w = 32;
    world_guernica_objects[1].clip.h = 40;

    world_guernica_objects[2].src.x = 128;
    world_guernica_objects[2].src.y = 48;
    /* world_guernica_objects[2].src.w = 128; */
    /* world_guernica_objects[2].src.h = 128; */
    world_guernica_objects[2].clip.x = 32;
    world_guernica_objects[2].clip.y = 88;
    world_guernica_objects[2].clip.w = 32;
    world_guernica_objects[2].clip.h = 40;
  }

  {
    local_persist struct line dragon_path;
    world_boss_entities[0].is_textured = TRUE;
    world_boss_entities[0].data.texture.id = BITMAP_INTERACTABLES;
    world_boss_entities[0].src.x = 46 * display.scale;
    world_boss_entities[0].src.y = 56 * display.scale;
    /* world_boss_entities[0].src.w = 32; */
    /* world_boss_entities[0].src.h = 32; */
    world_boss_entities[0].clip.x = 0;
    world_boss_entities[0].clip.y = 16 * display.scale;
    world_boss_entities[0].clip.w = 16 * display.scale;
    world_boss_entities[0].clip.h = 16 * display.scale;

    world_boss_entities[1].is_textured = TRUE;
    world_boss_entities[1].data.texture.id = BITMAP_DRAGON;
    world_boss_entities[1].x = 110 * display.scale;
    world_boss_entities[1].y = 27 * display.scale;
    world_boss_entities[1].move_speed = -8.f * display.scale;
    world_boss_entities[1].src.x = (i32)world_boss_entities[1].x;
    world_boss_entities[1].src.y = (i32)world_boss_entities[1].y;
    /* world_boss_entities[1].src.w = 64; */
    /* world_boss_entities[1].src.h = 64; */
    world_boss_entities[1].clip.x = 0;
    world_boss_entities[1].clip.y = 0;
    world_boss_entities[1].clip.w = 48 * display.scale;
    world_boss_entities[1].clip.h = 48 * display.scale;

    dragon_path.p1 = (world_boss_entities[1].src.x - (2 * display.scale));
    dragon_path.p2 = dragon_path.p1;
    dragon_path.p2 += world_boss_entities[1].clip.w + (3 * display.scale);

    world_boss_entities[1].action_data = (void*)&dragon_path;
    world_boss_entities[1].action = &horizontal_bounds_movement;
  }

  /* construct the maps */
  worlds[WORLD_START].bg_col = AO;
  worlds[WORLD_START].objects = world_start_objects;
  worlds[WORLD_START].object_count = WORLD_START_OBJECTS;
  worlds[WORLD_START].entities = NULL;
  worlds[WORLD_START].entity_count = 0;
  worlds[WORLD_HORSEMAN].bg_col = BRITISH_RACING_GREEN;
  worlds[WORLD_HORSEMAN].objects = world_horseman_objects;
  worlds[WORLD_HORSEMAN].object_count = WORLD_HORSEMAN_OBJECTS;
  worlds[WORLD_HORSEMAN].entities = NULL;
  worlds[WORLD_HORSEMAN].entity_count = 0;
  worlds[WORLD_GUERNICA].bg_col = BLACK;
  worlds[WORLD_GUERNICA].objects = world_guernica_objects;
  worlds[WORLD_GUERNICA].object_count = WORLD_GUERNICA_OBJECTS;
  worlds[WORLD_GUERNICA].entities = NULL;
  worlds[WORLD_GUERNICA].entity_count = 0;
  worlds[WORLD_DOCK].bg_col = OCEAN;
  worlds[WORLD_DOCK].objects = world_dock_objects;
  worlds[WORLD_DOCK].object_count = WORLD_DOCK_OBJECTS;
  worlds[WORLD_DOCK].entities = world_dock_entities;
  worlds[WORLD_DOCK].entity_count = WORLD_DOCK_ENTITIES;
  worlds[WORLD_BOSS].bg_col = REDWOOD;
  worlds[WORLD_BOSS].objects = NULL;
  worlds[WORLD_BOSS].object_count = 0;
  worlds[WORLD_BOSS].entities = world_boss_entities;
  worlds[WORLD_BOSS].entity_count = WORLD_BOSS_ENTITIES;
  worlds[WORLD_END].bg_col = BRITISH_RACING_GREEN;
  worlds[WORLD_END].objects = NULL;
  worlds[WORLD_END].object_count = 0;
  worlds[WORLD_END].entities = NULL;
  worlds[WORLD_END].entity_count = 0;

  {
    SDL_Event event;
    struct entity chimmy;
    struct FontDefinition fd;
    char str[20];
    u8 world_index = WORLD_START;
    r32 vx = 0.f, vy = 0.f;
    u32 bg_col = worlds[world_index].bg_col;
    u32 frame_count = 0;
    local_persist r64 fps[10];
    local_persist r64 msperframe;
    u32 fps_timer = SDL_GetTicks();
    u32 update_timer = SDL_GetTicks();
    u32 NOW = 0, LAST = 0;
    r32 dt = 0.f;

    fd.font.data = (void*)texture[BITMAP_SMALLFONT];
    fd.font.kerning = 1 * display.scale;
    fd.ascii.range.head = ASCII_SPACE;
    fd.ascii.range.tail = ASCII_TILDE;
    { /* consider this as if it was a separate function */
      i32 i;
      local_persist SDL_Rect glyphs[ASCII_COUNT];
      SDL_Surface* texture = (SDL_Surface*)fd.font.data;
      i32 clip_w = 6 * display.scale;
      i32 clip_h = 8 * display.scale;
      i32 glyph_width = texture->w / clip_w;
      for (i = fd.ascii.range.head; i < fd.ascii.range.tail; ++i) {
        glyphs[i].x = ((i - fd.ascii.range.head) % glyph_width) * clip_w;
        glyphs[i].y = ((i - fd.ascii.range.head) / glyph_width) * clip_h;
        glyphs[i].w = clip_w;
        glyphs[i].h = clip_h;
      }
      fd.glyphs = (void*)&glyphs;
    }

    chimmy.is_textured = TRUE;
    chimmy.data.texture.id = BITMAP_CHIMMY;
    chimmy.data.texture.anim = 0;
    chimmy.clip.x = chimmy.clip.y = 0;
    chimmy.clip.w = chimmy.clip.h = 16 * display.scale;
    chimmy.src.w = texture[BITMAP_CHIMMY]->w;
    chimmy.src.h = texture[BITMAP_CHIMMY]->h;
    chimmy.src.x = (display.w / 4);
    chimmy.src.y = (display.h / 2) - (chimmy.clip.h / 2);
    chimmy.move_speed = 40.f * display.scale;

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
              case SDLK_w: { vy -= chimmy.move_speed; }break;
              case SDLK_s: { vy += chimmy.move_speed; }break;
              case SDLK_a: { vx -= chimmy.move_speed; }break;
              case SDLK_d: { vx += chimmy.move_speed; }break;
              case SDLK_q: {
                if (world_index == WORLD_START) { world_index = WORLD_END; }
                else { world_index--; }
                bg_col = worlds[world_index].bg_col;
              }break;
              case SDLK_e: {
                if (world_index != WORLD_END) { world_index++; }
                bg_col = worlds[world_index].bg_col;
              }break;
              InvalidDefaultCase;
            }
          }break;
          case SDL_KEYUP: {
            switch (event.key.keysym.sym) {
              case SDLK_w: { vy += chimmy.move_speed; }break;
              case SDLK_s: { vy -= chimmy.move_speed; }break;
              case SDLK_a: { vx += chimmy.move_speed; }break;
              case SDLK_d: { vx -= chimmy.move_speed; }break;
              InvalidDefaultCase;
            }
          }break;
        }
      }

      entity_move(&chimmy, &display, vx, vy, dt);
      {
        i32 index;
        struct entity* iter = worlds[world_index].entities;
        if (iter) {
          for (index = 0; index < worlds[world_index].entity_count; ++index, ++iter) {
            if (iter->action) { iter->action(iter, dt); }
          }
        }
      }

      /*
       * NOTE: if you clear the backbuffer instead of the screen, while in interlaced scale mode
       * you can create some interesting motion blur style effects.
       */
      memset(display.screen->pixels, 0, display.screen->h * display.screen->pitch);
      SDL_FillRect(display.backbuffer, NULL, fill_color(display.backbuffer->format, bg_col));

      {
        i32 index;
        struct renderable* iter = worlds[world_index].objects;
        if (iter) {
          for (index = 0; index < worlds[world_index].object_count; ++index, ++iter) {
            if (iter->is_textured)
              SDL_BlitSurface(texture[iter->data.texture.id], &iter->clip, display.backbuffer, &iter->src);
            else
              SDL_FillRect(display.backbuffer, &iter->src, fill_color(display.backbuffer->format, iter->data.color));
          }
        }
      }

      if (display.scale > 1) {
        surface_progressive_scale(display.backbuffer, display.screen, display.scale);
      } else {
        SDL_BlitSurface(display.backbuffer, NULL, display.screen, NULL);
      }

      {
        i32 index;
        struct entity* iter = worlds[world_index].entities;
        if (iter) {
          for (index = 0; index < worlds[world_index].entity_count; ++index, ++iter) {
            if (iter->is_textured)
              SDL_BlitSurface(texture[iter->data.texture.id], &iter->clip, display.screen, &iter->src);
            else
              SDL_FillRect(display.backbuffer, &iter->src, fill_color(display.screen->format, iter->data.color));
          }
        }
      }

      SDL_BlitSurface(texture[chimmy.data.texture.id],
                      entity_calculate_crop_bounds(&chimmy), display.screen, &chimmy.src);

      /* sprintf(str, "%.fF/S", fps[0]); */
      sprintf(str, "%.05fMS/F", msperframe);
      SDL_DrawMonospaceText(&fd, display.screen, str, 2, 2);
      SDL_Flip(display.screen);

      frame_count++;

      /* update every second? */
      if (SDL_GetTicks() - update_timer > 1000) {
        i32 i;
        u64 counter_elapsed;
        for (i = 0; i < 9; ++i) { fps[i+1] = fps[i]; }
        counter_elapsed = SDL_GetTicks() - fps_timer;
        fps[0] = (r64)frame_count / (r64)(counter_elapsed / 1000.f);
        msperframe = (r64)(counter_elapsed) / (r64)frame_count;
        update_timer = SDL_GetTicks();
      }

      SDL_Delay(1); /* We should properly cap the FPS */
    }
  }

/*
 * Typically programs on desktop operating systems, memory is freed on application exit.
 * I'm currently assuming applications closing on the Wiz would be handled the same way.
 * My thought process for this is that the handheld is in essence running on GNU/Linux.
 */
defer:
  {
    i32 i;
    for (i = 0; i < BITMAP_COUNT; ++i)
      SDL_FreeSurface(texture[i]);
  }
  SDL_FreeSurface(display.backbuffer);
  SDL_Quit();
  return SUCCESS;
}
