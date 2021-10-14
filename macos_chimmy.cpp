
#include "chimmy_internals.h"
#include "SDL_platform.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

static b32 bRunning = { 1 };

#include "SDL_platform.cpp"

int
main(int /* argc */, char ** /* argv */) {

  //SDL_Init(SDL_INIT_VIDEO);
  SDL_Window * pWindow = 
    SDL_CreateWindow(PROJECT_FULL_NAME, 
                     SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                     SCREEN_WIDTH, SCREEN_HEIGHT,
                     SDL_WINDOW_HIDDEN);

  SDL_Renderer * pRenderer =
    SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED);

  // Show after initialization of everything.
  SDL_ShowWindow(pWindow);

  SDL_Event event;
  for (;;) {
    if (!bRunning) break;

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT)
        bRunning = false;
    }

    SDL_Rect sRectDestination = { 5, 5, 32, 32 };

    // Start drawing below here.
    SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(pRenderer);

    SDL_RenderDrawRect(pRenderer, &sRectDestination);
    SDL_SetRenderDrawColor(pRenderer, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(pRenderer, &sRectDestination);
    //SDL_RenderCopy(pRenderer, NULL, NULL, &sRectDestination);
    SDL_RenderPresent(pRenderer);
  }

  SDL_DestroyRenderer(pRenderer);
  SDL_Quit();
  return 0;
}