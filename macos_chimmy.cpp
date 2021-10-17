
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
global b32 isRumbleSupported;

#define Bitwise_Bool(b) ((b) == 0 ? (0) : (~0))

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

  // NOTE: Define non-required subsystems below.
  // Not all users want haptic feedback, nor do all controllers support this.
  platform.subsystems.haptic.isRequired = false;

#ifdef USE_GAMEPAD
  isGamepadSupported = true;
  isRumbleSupported = 
    isGamepadSupported && platform.subsystems.haptic.isRequired;
#endif

  u32 inputFlags = 
    Bitwise_Bool(isGamepadSupported) & SDL_INIT_GAMECONTROLLER |
    Bitwise_Bool(isRumbleSupported) & SDL_INIT_HAPTIC;

  if (SDL_InitSubSystem(inputFlags) != 0) {
    // TODO: Perform diagnostic.
    SDL_GetError();
  }

  platform.subsystems.input.isInitialized = true;
  platform.subsystems.haptic.isInitialized = isRumbleSupported;

  // REVIEW: There seems to be no real reason to call SDL_Init() on SDL_INIT_VIDEO.
  // Video initialization is happening by default?

  // TODO: Perhaps we should move these settings to an external file somewhere.
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

  // NOTE: Some systems may have VSYNC always enabled or disabled. 
  // Trying to enable VSYNC doesn't mean we're guaranteed to have it.
  // NOTE: Use SDL_RENDERER_SOFTWARE when building to run on virtual machines.
  u32 renderFlags = SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED;
  SDL_Renderer * pRenderer = SDL_CreateRenderer(pWindow, -1, renderFlags);
  
  if (!pRenderer) {
    // TODO: Perform diagnostic.
    SDL_GetError();
  }

  SDL_ShowWindow(pWindow);

  platform.performanceCounterFrequency = SDL_GetPerformanceFrequency();
  platform.isRunning = true;
  // !SECTION

  

  SDL_Event event;
  for (;;) {
    if (!platform.isRunning) break;

    while (SDL_PollEvent(&event)) {
      SDL_PLATFORM_FUNC(ProcessSystemEvents)(&platform, &event);
    }

    //SDL_Rect sRectDestination = { 5, 5, 32, 32 };

    // Start drawing below here.
    /*SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(pRenderer);

    SDL_RenderDrawRect(pRenderer, &sRectDestination);
    SDL_SetRenderDrawColor(pRenderer, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(pRenderer, &sRectDestination);*/
    //SDL_RenderCopy(pRenderer, NULL, NULL, &sRectDestination);
    //SDL_RenderPresent(pRenderer);
    
    SDL_PLATFORM_FUNC(SwapBuffers)(pRenderer, &platform.backbuffer);
  }

  // REVIEW: From previous experience it seems as if most things free themselves.
  // Particularly all SDL related systems. This should be double checked though.
  return 0;
}