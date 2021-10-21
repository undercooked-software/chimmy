
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

typedef struct TEXTURE_DATA {
  SDL_Texture *pHandle;
  u32 width, height;
} texture;

b32 BMPToTexture(texture *pTexture, SDL_Renderer *pRenderer, const char *filename,
                  u32 width, u32 height, b32 useColorKey = false);

b32
BMPToTexture(texture *pTexture, SDL_Renderer *pRenderer, const char *filename,
             u32 width, u32 height, b32 useColorKey) {

  SDL_Surface *pBMP = SDL_LoadBMP(filename);
  if (!pBMP)
    return false;

  pTexture->pHandle = 0;

  if (useColorKey) {
    u32 format =
      SDL_MapRGB(pBMP->format, color_key.r, color_key.g, color_key.b);
    SDL_SetColorKey(pBMP, SDL_TRUE, format);
  }

  pTexture->pHandle = SDL_CreateTextureFromSurface(pRenderer, pBMP);
  SDL_FreeSurface(pBMP);

  if (!pTexture->pHandle)
    return false;

  pTexture->width = width;
  pTexture->height = height;
  return true;
}

typedef struct ENTITY {
  texture pTexture;
  u32 anim;

  SDL_Rect pCropRect; // { texture-x, texture-y, crop-width, crop-height }
  SDL_Rect pPosition; // { pos-x, pos-y, width, height}
} entity;

#define ENTITY_NO_ANIM 255

void EntityMove(entity *pEntity, i32 x, i32 y);
SDL_Rect * EntityCalculateCropRect(entity *pEntity);
void EntityRemove(entity *pEntity);

void
EntityMove(entity *pEntity, i32 x, i32 y) {
  pEntity->pPosition.x = x;
  pEntity->pPosition.y = y;
}

SDL_Rect *
EntityCalculateCropRect(entity *pEntity) {
  // NOTE: 0 or NULL is passed to use the entire texture
  if (pEntity->anim == ENTITY_NO_ANIM)
    return 0;

  i32 frameWidth = pEntity->pPosition.w;
  i32 frameHeight = pEntity->pPosition.h;

  // integer division to floor the value
  i32 texture_modulus = pEntity->pTexture.width / frameWidth;

  // REVIEW: Maybe add data santizing checks if anim is OoB of texture?

  pEntity->pCropRect.w = frameWidth;
  pEntity->pCropRect.h = frameHeight;
  pEntity->pCropRect.x = (pEntity->anim % texture_modulus) * frameWidth;
  pEntity->pCropRect.y = (pEntity->anim / texture_modulus) * frameHeight;

  return &pEntity->pCropRect;
}

void
EntityRemove(entity *pEntity) {
  // FIXME For now this will just clear out the texture
  SDL_DestroyTexture(pEntity->pTexture.pHandle);
}

typedef struct MAP_DATA {
  COLOR background;
  //void (*pHandler)(struct MAP_DATA *pMap);
  
  struct MAP_DATA *pPrev, *pNext;
} map_data;

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
  map_data *head;
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

  // NOTE: Test Entities
  entity background = {};
  BMPToTexture(&background.pTexture, pRenderer, "data/image/palette.bmp", 8, 8);
  background.pPosition.w = background.pPosition.h = PIXEL_SIZE;
  background.anim = start.background;

  entity chimmy = {};
  BMPToTexture(&chimmy.pTexture, pRenderer, "data/image/chimmy.bmp", 64, 64, true);
  chimmy.pPosition.w = chimmy.pPosition.h = 20;
  EntityMove(&chimmy, 34, 65);
  // !SECTION

  SDL_Event event;
  for (;;) {
    if (!platform.isRunning) break;

    while (SDL_PollEvent(&event)) {
      SDL_PLATFORM_FUNC(ProcessSystemEvents)(&platform, &event);
      // TODO: Actually utilize the functions in SDL_platform.cpp
      switch (event.type) {
        case SDL_KEYDOWN: {
          switch (event.key.keysym.sym) {
            case SDLK_LEFT: {
              if (head->pPrev != 0)
                head = head->pPrev;
                background.anim = head->background;
            }break;
            case SDLK_RIGHT: {
              if (head->pNext != 0)
                head = head->pNext;
                background.anim = head->background;
            }break;
            case SDLK_UP: {
              chimmy.anim++;
            }break;
            case SDLK_DOWN: {
              chimmy.anim--;
            }break;
          }
        }break;
      }
    }

    SDL_RenderClear(pRenderer);

    SDL_RenderCopy(pRenderer, background.pTexture.pHandle,
                   EntityCalculateCropRect(&background), 0);
    SDL_RenderCopy(pRenderer, chimmy.pTexture.pHandle,
                   EntityCalculateCropRect(&chimmy), &chimmy.pPosition);

    SDL_PLATFORM_FUNC(SwapBuffers)(pRenderer, &platform.backbuffer);

    // FIXME: Add an actual way to limit framerate outside of vsync
    SDL_Delay(16);
  }
  
  EntityRemove(&background);
  EntityRemove(&chimmy);

  // REVIEW: Windows tends to free the majority of things at the end of runtime.
  // This may not be the same on MacOS and should be reviewed.
  return 0;
}