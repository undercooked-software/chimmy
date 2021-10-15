
#include "chimmy_internals.h"
#include "SDL_platform.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#include "SDL_platform.cpp"

// TODO: Move this to the build script preprocessors.
#ifndef USE_GAMEPAD
  #define USE_GAMEPAD
#endif

global b32 isGamepadSupported;

int
main(int /* argc */, char ** /* argv */) {

  // SECTION: SDL Platform API Initialization
  sdl_platform platform = {};
  for (i32 systemIndex = 0; 
       systemIndex < ArrayCount(platform.subsystems.systems);
       ++systemIndex)
  {
    // NOTE: Set all subsystems to be required by default.
    platform.subsystems.systems[systemIndex].isRequired = true;
    platform.subsystems.systems[systemIndex].isInitialized = false;
  }

#ifdef USE_GAMEPAD
  isGamepadSupported = platform.subsystems.haptic.isRequired = true;
#endif
  // NOTE: Define non-required subsystems below.
  // Not all users want haptic feedback, nor do all controllers support this.
#if 1
  // NOTE: Disable this toggle when we're done testing.
  platform.subsystems.haptic.isRequired = false;
#endif

#if 0
  u32 controllerInitFlag = (~(u32)isGamepadSupported & SDL_INIT_GAMECONTROLLER);
  u32 hapticInitFlag = 
  
  // TODO: Decide on how we want to handle variable amounts of controllers.
  // For now, initialize controller and haptic systems regardless of controller amount.
  u32 inputFlags = platform.subsystems.input.isRequired ? SDL_INIT_GAMECONTROLLER : 0;
#endif

  if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) != 0) {
    // TODO: Perform diagnostic.
    SDL_GetError();
  }

  // REVIEW: There seems to be no real reason to call SDL_Init() on SDL_INIT_VIDEO.
  // Video initialization is happening by default??

  u32 windowFlags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;
  SDL_Window * pWindow = 
    SDL_CreateWindow(PROJECT_FULL_NAME, 
                     SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                     SCREEN_WIDTH, SCREEN_HEIGHT,
                     windowFlags);

  if (!pWindow) {
    // TODO: Perform diagnostic.
    SDL_GetError();
  }

  i32 monitorRefreshHz = SDL_PLATFORM_FUNC(GetRefreshRate)(pWindow);
  r32 gameUpdateHz = (monitorRefreshHz / 1.0f);
  r32 targetSecondsPerFrame = (1.0f / gameUpdateHz);

  SDL_Renderer * pRenderer =
    SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED);

  // Show after initialization of everything.
  SDL_ShowWindow(pWindow);

  platform.performanceCounterFrequency = SDL_GetPerformanceFrequency();
  platform.isRunning = true;
  // !SECTION

  SDL_Event event;
  for (;;) {
    if (!platform.isRunning) break;

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT)
        platform.isRunning = false;
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