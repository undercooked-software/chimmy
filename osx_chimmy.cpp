#include "base_types.h"

#include <SDL2/SDL.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

static b32 bRunning = { 1 };

int
main(int argc, char **argv) {

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window = 
    SDL_CreateWindow("Chimmy", 
                      SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                      SCREEN_WIDTH, SCREEN_HEIGHT,
                      SDL_WINDOW_HIDDEN);

  SDL_Renderer *renderer =
    SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  // Show after initialization of everything.
  SDL_ShowWindow(window);

  SDL_Event event;
  for (;;) {
    if (!bRunning) break;

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT)
        bRunning = false;
    }

    SDL_Rect sRectDestination = { 5, 5, 32, 32 };

    // Start drawing below here.
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    SDL_RenderDrawRect(renderer, &sRectDestination);
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &sRectDestination);
    //SDL_RenderCopy(renderer, NULL, NULL, &sRectDestination);
    SDL_RenderPresent(renderer);
  }

  SDL_DestroyRenderer(renderer);
  SDL_Quit();
  return 0;
}