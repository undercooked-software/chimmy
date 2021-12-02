
#include "chimmy_internals.h"
#include "MSYS_platform.h"
#include "entity.h"

#define SCREEN_WIDTH 200
#define SCREEN_HEIGHT 150

#include "MSYS_platform.cpp"
#include "entity.cpp"
#include "texture.cpp"

global b32 isGamepadSupported;
global b32 isRumbleSupported;

typedef struct _Map_DATA
{
  COLOR background;
  //void (*pHandler)(struct MAP_DATA *pMap);
  
  struct _Map_DATA *pPrev, *pNext;
} Map_DATA;

int
main(int /* argc */, char ** /* argv */)
{
  // SECTION: MSYS Platform API Initialization
  MSYS_Platform platform = {};
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

  // NOTE: SDL_INIT_GAMECONTROLLER automatically initializes SDL_INIT_JOYSTICK.
  u32 inputFlags =
    (BitwiseBool(isGamepadSupported) & SDL_INIT_GAMECONTROLLER) |
    (BitwiseBool(isRumbleSupported) & SDL_INIT_HAPTIC);

  if (SDL_InitSubSystem(inputFlags) != 0) {
    // TODO: Perform diagnostic.
    // SDL_GetError();
  }

  platform.subsystems.input.isInitialized = true;
  platform.subsystems.haptic.isInitialized = isRumbleSupported;

  // LINK: https://wiki.libsdl.org/SDL_Init
  // REVIEW: There seems to be no real reason to call SDL_Init()?
  //         The following subsystems seem to initialize regardless:
  //           - SDL_INIT_TIMER, SDL_INIT_EVENTS, SDL_INIT_VIDEO

  // TODO: Perhaps we should move these settings to an external file somewhere.
  u32 windowFlags = SDL_WINDOW_HIDDEN;
  SDL_Window *pWindow = 
    SDL_CreateWindow(PROJECT_FULL_NAME, 
                     SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                     SCREEN_WIDTH, SCREEN_HEIGHT,
                     windowFlags);

  if (!pWindow) {
    // TODO: Perform diagnostic.
    // SDL_GetError();
  }

  i32 monitorRefreshHz = MSYS_PLATFORM_FUNC(GetRefreshRate)(pWindow);
  r32 gameUpdateHz = (monitorRefreshHz / 1.0f);
  r32 targetSecondsPerFrame = (1.0f / gameUpdateHz);

  // NOTE: Some systems may have VSYNC always enabled or disabled. 
  // Trying to enable VSYNC doesn't mean we're guaranteed to have it.
  // NOTE: Use SDL_RENDERER_SOFTWARE when building to run on virtual machines.
  u32 renderFlags = SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED;
  SDL_Renderer *pRenderer = SDL_CreateRenderer(pWindow, -1, renderFlags);
  
  if (!pRenderer) {
    // TODO: Perform diagnostic.
    // SDL_GetError();
  }

  SDL_ShowWindow(pWindow);

  platform.performanceCounterFrequency = SDL_GetPerformanceFrequency();
  platform.isRunning = true;
  // !SECTION

  // SECTION: Temp Data
  // NOTE makeshift linked-list for map traversal
  Map_DATA *head;
  Map_DATA start, test1, test2, finish;
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

  // NOTE: Test Entities (currently not attached to maps)
  Entity background = {};
  background.pTexture = BMPToTexture(pRenderer, "data/image/palette.bmp", false);
  background.pPosition.w = background.pPosition.h = PIXEL_SIZE;
  background.anim = start.background;

  const i32 treeCount = 6;
  Entity trees[treeCount];
  i32 points[treeCount*2] = {
    12, 7,    84, 7,    151, 7,
    12, 105,  84, 105,  151, 105
  };
  Entity *pHeadTree = trees;
  Entity *pTailTree = trees + treeCount;
  pHeadTree->pTexture = BMPToTexture(pRenderer, "data/image/tree.bmp", true);
  pHeadTree->pPosition.w = 32;
  pHeadTree->pPosition.h = 40;
  pHeadTree->anim = ENTITY_NO_ANIM;
  EntityMove(pHeadTree, points[0], points[1]);
  for (i32 i = 0; i < treeCount - 1; ++i) {
    trees[i+1].pTexture = trees[i].pTexture;
    trees[i+1].pPosition.w = trees[i].pPosition.w;
    trees[i+1].pPosition.h = trees[i].pPosition.h;
    trees[i+1].anim = ENTITY_NO_ANIM;
    EntityMove(&trees[i+1], points[(i+1)*2], points[((i+1)*2)+1]);
  }

  Entity chimmy = {};
  chimmy.pTexture = BMPToTexture(pRenderer, "data/image/chimmy.bmp", true);
  chimmy.pPosition.w = chimmy.pPosition.h = 20;
  EntityMove(&chimmy, 34, 65);
  // !SECTION

  SDL_Event event;
  for (;;) {
    if (!platform.isRunning) break;

    while (SDL_PollEvent(&event)) {
      MSYS_PLATFORM_FUNC(ProcessSystemEvents)(&platform, &event);
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
            case SDLK_1: {
              MSYS_PLATFORM_FUNC(SaveScreenshot)(pRenderer, SCREEN_WIDTH, SCREEN_HEIGHT);
            }break;
          }
        }break;
      }
    }

    SDL_RenderClear(pRenderer);
    {
      SDL_RenderCopy(pRenderer, background.pTexture,
                   EntityCalculateCropRect(&background), 0);

      for (pHeadTree = trees; pHeadTree < pTailTree; ++pHeadTree) {
        SDL_RenderCopy(pRenderer, pHeadTree->pTexture,
                       EntityCalculateCropRect(pHeadTree), &pHeadTree->pPosition);
      }

      SDL_RenderCopy(pRenderer, chimmy.pTexture,
                   EntityCalculateCropRect(&chimmy), &chimmy.pPosition);
    }
    MSYS_PLATFORM_FUNC(SwapBuffers)(pRenderer, &platform.backbuffer);

    // FIXME: Add an actual way to limit framerate outside of vsync
    SDL_Delay(16);
  }
  
  EntityRemove(&background);
  EntityRemove(&chimmy);

  // REVIEW: Windows tends to free the majority of things at the end of runtime.
  // This may not be the same on MacOS and should be reviewed.
}
