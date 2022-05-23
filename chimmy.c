#include "base_types.h"

#include <raylib.h>

#define BASE_SCREEN_WIDTH 200
#define BASE_SCREEN_HEIGHT 150

global r32 screen_scale_modifier = 2;
global r32 screen_scale_min = 1;
global r32 screen_scale_max = 4;

typedef enum GameScene { LOGO = 0, OPTIONS, GAMEPLAY } gameScene;

/* https://colors.artyclick.com/color-name-finder/ */
/* Prefixed all the colors with PAL_ to prevent name collisions with raylib */
enum PALETTE {
  PAL_BLACK = 0, PAL_CLOUD, PAL_WHITE,
  PAL_DARK_RED = 8, PAL_CRIMSON_RED, PAL_TOMATO, PAL_DEEP_SAFFRON,
  PAL_DARK_EBONY = 16, PAL_CORN_HARVEST, PAL_OLD_GOLD, PAL_BRANDY, PAL_WINTER_HAZEL,
  PAL_DEEP_GREEN = 24, PAL_DARK_LIME_GREEN, PAL_LEAFY_GREEN,
  PAL_BLUE_GEM = 32, PAL_OCEAN, PAL_BUTTERFLY_BLUE,
  PAL_BARNEY_PURPLE = 40
};

#define PIXEL_SIZE  1
#define CHIMMY_SIZE 20

void
WindowScaledResize(void) {
  SetWindowSize(BASE_SCREEN_WIDTH * (i32)screen_scale_modifier,
                BASE_SCREEN_HEIGHT * (i32)screen_scale_modifier);
}

int
main(void)
{
  /* SetConfigFlags(FLAG_VSYNC_HINT); */
  InitWindow(BASE_SCREEN_WIDTH, BASE_SCREEN_HEIGHT, "chimmy");
  SetExitKey(KEY_NULL);
  WindowScaledResize();

  r64 previousTime = GetTime();
  r64 currentTime = 0.0;
  r64 updateDrawTime = 0.0;
  r64 waitTime = 0.0;
  r32 deltaTime = 0.0f;

  r32 timeCounter = 0.0f;

  int targetFPS = 60;

  gameScene scene = LOGO;
  i32 map_color = PAL_WHITE;

  Texture2D chimmy_sprite = LoadTexture("data/image/chimmy.png");
  Rectangle chimmy_anim = { 0, 0, CHIMMY_SIZE, CHIMMY_SIZE };
  Rectangle chimmy_rect = { 0, 0, CHIMMY_SIZE, CHIMMY_SIZE };

  Texture2D map_bg = LoadTexture("data/image/palette.png");
  Rectangle palette_point = { 0, 0, PIXEL_SIZE, PIXEL_SIZE };
  Rectangle screen_rect = { 0, 0, BASE_SCREEN_WIDTH, BASE_SCREEN_HEIGHT };

  while (!WindowShouldClose())
  {
    PollInputEvents();

    switch (scene) {
      case LOGO: {
        if (IsKeyPressed(KEY_ENTER)) {
          palette_point.x = (r32)((i32)map_color % 8);
          palette_point.y = (r32)(map_color / 8);
          scene = GAMEPLAY;
        }
        if (IsKeyPressed(KEY_ESCAPE)) { scene = OPTIONS; }
      }break;
      case OPTIONS: {
        if (IsKeyPressed(KEY_LEFT)) {
          if (screen_scale_modifier - 1 >= screen_scale_min)
            screen_scale_modifier -= 1;
        }
        if (IsKeyPressed(KEY_RIGHT)) {
          if (screen_scale_modifier + 1 <= screen_scale_max)
            screen_scale_modifier += 1;
        }
        if (IsKeyPressed(KEY_ENTER)) {
          WindowScaledResize();
          scene = LOGO;
        }
      }break;
      InvalidDefaultCase;
    }

    timeCounter += deltaTime;

    BeginDrawing();
      ClearBackground(RAYWHITE);
      switch (scene) {
        case OPTIONS: {
          DrawText("Options:", 50, 125, 10, BLACK);
        }break;
        case GAMEPLAY: {
          Vector2 origin = { 0, 0 };
          screen_rect.width = BASE_SCREEN_WIDTH * screen_scale_modifier;
          screen_rect.height = BASE_SCREEN_HEIGHT * screen_scale_modifier;
          chimmy_rect.x = chimmy_rect.y = 32;
          chimmy_rect.width = CHIMMY_SIZE * screen_scale_modifier;
          chimmy_rect.height = CHIMMY_SIZE * screen_scale_modifier;
          DrawTexturePro(map_bg, palette_point, screen_rect, origin, 0, WHITE);
          DrawTexturePro(chimmy_sprite, chimmy_anim, chimmy_rect, origin, 0, WHITE);
        }break;
        InvalidDefaultCase;
      }
      DrawFPS(10, 10);
    EndDrawing();

    currentTime = GetTime();
    updateDrawTime = currentTime - previousTime;
    waitTime = (1.0f/(r32)targetFPS) - updateDrawTime;
    if (waitTime > 0.0) {
      WaitTime((r32)waitTime * 1000.f);
      currentTime = GetTime();
      deltaTime = (r32)(currentTime - previousTime);
    }
    else {
      deltaTime = (r32)updateDrawTime;
    }
    previousTime = currentTime;
  }

  UnloadTexture(map_bg);

  CloseWindow();
}
