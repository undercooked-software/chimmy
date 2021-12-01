
// REVIEW: Alter this accordingly when we have more information.
#ifdef _WIN32
  #define SDL_MAIN_HANDLED
  #include <SDL.h>
  #if 0
    #include <SDL_opengl.h>
  #endif
#else
  #include <SDL2/SDL.h>
#endif

// NOTE: Building with c++, typedef struct foo {} should already be implied.
// REVIEW: typedef struct {} may cause issues with C++. Needs testing.
typedef struct _MSYS_DisplayDimensions
{
  i32 width, height;
} MSYS_DisplayDimensions;

typedef struct _MSYS_DisplayBuffer
{
  SDL_Texture *pHandle;
  i32 width, height;
  i32 pitch;
} MSYS_DisplayBuffer;

typedef struct _MSYS_Gamepad
{
  SDL_GameController *pHandle;
  SDL_Haptic *pRumbleHandle;
} MSYS_Gamepad;

typedef struct _MSYS_SubsystemState
{
  b32 isInitialized;
  b32 isRequired;
} MSYS_SubsystemState;

typedef struct _MSYS_Subsystems
{
  union {
    MSYS_SubsystemState systems[3];
    struct {
      MSYS_SubsystemState video;
      // NOTE: We may want to separate input types [controller/keyboard]
      MSYS_SubsystemState input;
      MSYS_SubsystemState haptic;
      // NOTE: systems array should match anonymous struct
      MSYS_SubsystemState terminator;
    };
  };
} MSYS_Subsystems;

typedef struct _MSYS_Platform
{
  MSYS_Subsystems subsystems;

  MSYS_DisplayBuffer backbuffer;

  u64 performanceCounterFrequency;
  b32 isRunning;
} MSYS_Platform;

// SECTION: Platform API definitions
#define MSYS_PLATFORM_FUNC(name) Join(MSYS_Platform_, name)
// NOTE: Renderer
#define MSYS_PLATFORM_RENDERER_GET_FROM_WINDOWID(name) \
  SDL_Renderer * MSYS_PLATFORM_FUNC(name)(u32 windowID)
// NOTE: Display
#define MSYS_PLATFORM_DISPLAY_GET_DIMENSIONS(name) \
  MSYS_DisplayDimensions MSYS_PLATFORM_FUNC(name)(SDL_Window *pWindow)
#define MSYS_PLATFORM_DISPLAY_GET_REFRESH_RATE(name) \
  i32 MSYS_PLATFORM_FUNC(name)(SDL_Window *pWindow)
#define MSYS_PLATFORM_DISPLAY_CAPTURE(name) \
  void MSYS_PLATFORM_FUNC(name)(SDL_Renderer *pRenderer, \
                                i32 width, i32 height)
#define MSYS_PLATFORM_DISPLAY_RESIZE(name) \
  void MSYS_PLATFORM_FUNC(name)(SDL_Renderer *pRenderer, \
                                MSYS_DisplayBuffer *pBuffer, \
                                i32 width, i32 height)
#define MSYS_PLATFORM_DISPLAY_UPDATE(name) \
  void MSYS_PLATFORM_FUNC(name)(SDL_Renderer *pRenderer, \
                                MSYS_DisplayBuffer *pBuffer)
// NOTE: Time
#define MSYS_PLATFORM_TIME_GET_TIME(name) \
  r32 MSYS_PLATFORM_FUNC(name)(MSYS_Platform *pMSYS, \
                               u64 oldCounter, u64 currentCounter)
// NOTE: Events
#define MSYS_PLATFORM_EVENT_HANDLER(name) \
  void MSYS_PLATFORM_FUNC(name)(MSYS_Platform *pMSYS, SDL_Event *pEvent)
// !SECTION
