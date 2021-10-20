
#include "chimmy_internals.h"
#include "SDL_platform.h"

#define SCREEN_WIDTH 200
#define SCREEN_HEIGHT 150

#include "SDL_platform.cpp"

// TODO: Move this to the build script preprocessors.
#ifndef USE_GAMEPAD
  #define USE_GAMEPAD
#endif

global b32 isGamepadSupported;
global b32 isRumbleSupported;

// TODO: Move this elsewhere
#define Bitwise_Bool(b) ((b) == 0 ? (0) : (~0))

// SECTION Map functionality.
// TODO: Move this to a proper location
typedef struct MAP_DATA {
  COLOR background;
  void (*pHandler)(struct MAP_DATA *pMap);
  
  struct MAP_DATA *pPrev, *pNext;
} map_data;
// !SECTION

int
main(int /* argc */, char ** /* argv */) {

  // SECTION: SDL Platform API Initialization
  sdl_platform platform = {};
  for (u32 systemIndex = 0; 
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
    (Bitwise_Bool(isGamepadSupported) & SDL_INIT_GAMECONTROLLER) |
    (Bitwise_Bool(isRumbleSupported) & SDL_INIT_HAPTIC);

  if (SDL_InitSubSystem(inputFlags) != 0) {
    // TODO: Perform diagnostic.
    SDL_GetError();
  }

  platform.subsystems.input.isInitialized = true;
  platform.subsystems.haptic.isInitialized = isRumbleSupported;

  // REVIEW: There seems to be no real reason to call SDL_Init() on SDL_INIT_VIDEO.
  // Video initialization is happening by default?

  // TODO: Perhaps we should move these settings to an external file somewhere.
  u32 windowFlags = SDL_WINDOW_HIDDEN;
  SDL_Window *pWindow = 
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
  SDL_Renderer *pRenderer = SDL_CreateRenderer(pWindow, -1, renderFlags);
  
  if (!pRenderer) {
    // TODO: Perform diagnostic.
    SDL_GetError();
  }

  SDL_ShowWindow(pWindow);

  platform.performanceCounterFrequency = SDL_GetPerformanceFrequency();
  platform.isRunning = true;
  // !SECTION

  // SECTION: Temp Data
  // NOTE makeshift linked-list 
  map_data * head;
  map_data start, test1, test2, finish;
  start.background = DEEP_GREEN;
  start.pNext = &test1;
  start.pPrev = &finish;
  test1.background = DARK_RED;
  test1.pPrev = &start;
  test1.pNext = &test2;
  test2.background = BLUE_GEM;
  test2.pPrev = &test1;
  test2.pNext = &finish;
  finish.background = BLACK;
  finish.pPrev = 0;
  finish.pNext = 0;

  head = &start;

  // NOTE: Texture loading, color keying
  // TODO: Move these textures somewhere else?
  u32 format;
  SDL_Surface *pPaletteBitmap = SDL_LoadBMP("data/image/palette.bmp");
  format = SDL_MapRGB(pPaletteBitmap->format, 
                      color_key.r, color_key.g, color_key.b);
  SDL_SetColorKey(pPaletteBitmap, SDL_TRUE, format);
  SDL_Texture *pPaletteTexture = SDL_CreateTextureFromSurface(pRenderer, pPaletteBitmap);
  SDL_FreeSurface(pPaletteBitmap);

  SDL_Surface *pChimmyBitmap = SDL_LoadBMP("data/image/chimmy.bmp");
  format = SDL_MapRGB(pChimmyBitmap->format, 
                      color_key.r, color_key.g, color_key.b);
  SDL_SetColorKey(pChimmyBitmap, SDL_TRUE, format);
  SDL_Texture *pChimmyTexture = SDL_CreateTextureFromSurface(pRenderer, pChimmyBitmap);
  SDL_FreeSurface(pChimmyBitmap);

  SDL_Surface *pTreeBitmap = SDL_LoadBMP("data/image/tree.bmp");
  format = SDL_MapRGB(pTreeBitmap->format, 
                      color_key.r, color_key.g, color_key.b);
  SDL_SetColorKey(pTreeBitmap, SDL_TRUE, format);
  SDL_Texture *pTreeTexture = SDL_CreateTextureFromSurface(pRenderer, pTreeBitmap);
  SDL_FreeSurface(pTreeBitmap);

  // NOTE: Test Data
  SDL_Rect chimmyPos = { 34, 65, 20, 20 };
  SDL_Rect paletteColor = { head->background % 8, head->background / 8, 
                            PIXEL_SIZE, PIXEL_SIZE };
  // !SECTION

  SDL_Event event;
  for (;;) {
    if (!platform.isRunning) break;

    while (SDL_PollEvent(&event)) {
      SDL_PLATFORM_FUNC(ProcessSystemEvents)(&platform, &event);
      switch (event.type) {
        case SDL_KEYDOWN: {
          switch (event.key.keysym.sym) {
            case SDLK_LEFT: {
              if (head->pPrev != 0)
                head = head->pPrev;
                paletteColor.x = head->background % 8; 
                paletteColor.y = head->background / 8;
            }break;
            case SDLK_RIGHT: {
              if (head->pNext != 0)
                head = head->pNext;
                paletteColor.x = head->background % 8; 
                paletteColor.y = head->background / 8;
            }break;
          }
        }break;
      }
    }

    SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(pRenderer);

    if (head->background)
      SDL_RenderCopy(pRenderer, pPaletteTexture, &paletteColor, 0);
      
    SDL_RenderCopy(pRenderer, pChimmyTexture, 0, &chimmyPos);

    SDL_PLATFORM_FUNC(SwapBuffers)(pRenderer, &platform.backbuffer);

    // FIXME: Add an actual way to limit framerate outside of vsync
    SDL_Delay(16);
  }
  
  SDL_DestroyTexture(pPaletteTexture);
  SDL_DestroyTexture(pChimmyTexture);
  SDL_DestroyTexture(pTreeTexture);

  // REVIEW: Windows tends to free the majority of things at the end of runtime.
  // This may not be the same on MacOS and should be reviewed.
  return 0;
}