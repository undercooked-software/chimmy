
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
typedef struct SDL_DISPLAY_DIMENSIONS {
  i32 width, height;
} sdl_display_dimensions;

typedef struct SDL_OFFSCREEN_BUFFER {
  SDL_Texture * pTexture;
  //void * pvMemory;

  i32 width, height;
  // NOTE: pitch is only necessary if we are handling our own buffer memory.
  //i32 pitch;
} sdl_offscreen_buffer;

typedef struct SDL_GAMEPAD {
  SDL_GameController *pHandle;
  SDL_Haptic *pRumbleHandle;
} sdl_gamepad;

typedef struct SDL_SUBSYSTEM_STATE {
  b32 isInitialized;
  b32 isRequired;
} sdl_subsystem_state;

typedef struct SDL_SUBSYSTEMS {
  union {
    sdl_subsystem_state systems[3];
    struct {
      sdl_subsystem_state video;
      // NOTE: We may want to separate input types [controller/keyboard]
      sdl_subsystem_state input;
      sdl_subsystem_state haptic;
      // NOTE: systems array should match anonymous struct
      sdl_subsystem_state terminator;
    };
  };
} sdl_subsystems;

typedef struct SDL_PLATFORM {
  sdl_subsystems subsystems;

  sdl_offscreen_buffer backbuffer;

  u64 performanceCounterFrequency;
  b32 isRunning;
} sdl_platform;

// SECTION: Platform API definitions
#define SDL_PLATFORM_FUNC(name) glue(SDL_Platform_, name)
// NOTE: Renderer
#define SDL_PLATFORM_RENDERER_GET_FROM_WINDOWID(name) \
  SDL_Renderer *SDL_PLATFORM_FUNC(name)(u32 windowID)
// NOTE: Display
#define SDL_PLATFORM_DISPLAY_GET_DIMENSIONS(name) \
  sdl_display_dimensions SDL_PLATFORM_FUNC(name)(SDL_Window *pWindow)
#define SDL_PLATFORM_DISPLAY_GET_REFRESH_RATE(name) \
  i32 SDL_PLATFORM_FUNC(name)(SDL_Window *pWindow)
#define SDL_PLATFORM_DISPLAY_RESIZE(name) \
  void SDL_PLATFORM_FUNC(name)(SDL_Renderer *pRenderer, \
                               sdl_offscreen_buffer *pBuffer, \
                               i32 width, i32 height)
#define SDL_PLATFORM_DISPLAY_CAPTURE(name) \
  void SDL_PLATFORM_FUNC(name)(SDL_Renderer *pRenderer, \
                               i32 width, i32 height)
#define SDL_PLATFORM_DISPLAY_UPDATE(name) \
  void SDL_PLATFORM_FUNC(name)(SDL_Renderer *pRenderer, \
                               sdl_offscreen_buffer *pBuffer)
// NOTE: Time
#define SDL_PLATFORM_TIME_GET_TIME(name) \
  r32 SDL_PLATFORM_FUNC(name)(sdl_platform *pSDL, \
                              u64 oldCounter, u64 currentCounter)
// NOTE: Events
#define SDL_PLATFORM_EVENT_HANDLER(name) \
  void SDL_PLATFORM_FUNC(name)(sdl_platform *pSDL, SDL_Event *pEvent)
// !SECTION