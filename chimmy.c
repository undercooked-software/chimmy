
#include "raylib.h"

int main(void)
{
  const int screenWidth = 200;
  const int screenHeight = 150;

  InitWindow(screenWidth, screenHeight, "chimmy");

  SetTargetFPS(60);

  while (!WindowShouldClose())
  {
    /* Update
     *----------------------------------------------------------------------------------
     * TODO: Update your variables here
     *----------------------------------------------------------------------------------
     */

    BeginDrawing();
      ClearBackground(RAYWHITE);
      DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);
    EndDrawing();
  }

  CloseWindow();
}