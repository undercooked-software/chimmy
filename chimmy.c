#include "raylib.h"

#define global static
#define internal static
#define local_persist static

typedef int i32;
typedef i32 b32;
typedef float r32;
typedef double r64;

#define BASE_SCREEN_WIDTH 200
#define BASE_SCREEN_HEIGHT 150

global r32 screen_scale_modifier = 2;
global r32 screen_scale_min = 1;
global r32 screen_scale_max = 4;

typedef enum GameScene { LOGO = 0, OPTIONS, GAMEPLAY, ENDING } gameScene;

#define InvalidDefaultCase default:break

/* https://colors.artyclick.com/color-name-finder/ */
/* Prefixed all the colors with PAL_ to prevent name collisions with raylib */
enum PALETTE {
  PAL_BLACK = 0,
  PAL_CLOUD,
  PAL_WHITE,
  PAL_DARK_RED = 8,
  PAL_CRIMSON_RED,
  PAL_TOMATO,
  PAL_DEEP_SAFFRON,
  PAL_DARK_EBONY = 16,
  PAL_CORN_HARVEST,
  PAL_OLD_GOLD,
  PAL_BRANDY,
  PAL_WINTER_HAZEL,
  PAL_DEEP_GREEN = 24,
  PAL_DARK_LIME_GREEN,
  PAL_LEAFY_GREEN,
  PAL_BLUE_GEM = 32,
  PAL_OCEAN,
  PAL_BUTTERFLY_BLUE,
  PAL_BARNEY_PURPLE = 40
};

void WindowScaledResize(void) {
  SetWindowSize(BASE_SCREEN_WIDTH * (i32)screen_scale_modifier,
                BASE_SCREEN_HEIGHT * (i32)screen_scale_modifier);
}

int
main(void)
{
  /* SetConfigFlags(FLAG_MSAA_4X_HINT); */
  InitWindow(BASE_SCREEN_WIDTH, BASE_SCREEN_HEIGHT, "chimmy");
  WindowScaledResize();

  SetTargetFPS(60);
  SetExitKey(KEY_NULL);

  gameScene scene = LOGO;

  Texture2D test = LoadTexture("data/image/palette.bmp");

  while (!WindowShouldClose())
  {
    switch (scene) {
      case LOGO: {
        if (IsKeyPressed(KEY_ENTER)) { scene = GAMEPLAY; }
        if (IsKeyPressed(KEY_ESCAPE)) { scene = OPTIONS; }
      }break;
      case OPTIONS: {
        if (IsKeyPressed(KEY_LEFT)) {
          if (screen_scale_modifier - 1 > screen_scale_min)
            screen_scale_modifier -= 1;
        }
        if (IsKeyPressed(KEY_RIGHT)) {
          if (screen_scale_modifier + 1 > screen_scale_max)
            screen_scale_modifier += 1;
        }
        if (IsKeyPressed(KEY_ENTER)) {
          WindowScaledResize();
          scene = LOGO;
        }
      }break;
      InvalidDefaultCase;
    }

    BeginDrawing();
      ClearBackground(RAYWHITE);
      switch (scene) {
        case OPTIONS: {
          DrawText("Options:", 5, 5, 10, BLACK);
          DrawText("TEST", 550, 550, 10, BLACK);
        }break;
        case GAMEPLAY: {
          DrawText("PRESS ENTER or TAP to JUMP to GAMEPLAY SCREEN", 120, 220, 10, BLACK);
        }break;
        InvalidDefaultCase;
      }
    EndDrawing();
  }

  UnloadTexture(test);

  CloseWindow();
}
