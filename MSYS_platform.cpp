
internal
MSYS_PLATFORM_RENDERER_GET_FROM_WINDOWID(GetRenderer)
{
  SDL_Window *pWindow = SDL_GetWindowFromID(windowID);
  if (pWindow) {
    SDL_Renderer *pRenderer = SDL_GetRenderer(pWindow);
    if (pRenderer)
      return pRenderer;
  }
  return 0;
}

internal
MSYS_PLATFORM_DISPLAY_GET_DIMENSIONS(GetDisplayDimensions)
{
  MSYS_DisplayDimensions dimensions;
  SDL_GetWindowSize(pWindow, &dimensions.width, &dimensions.height);
  return dimensions;
}

internal
MSYS_PLATFORM_DISPLAY_GET_REFRESH_RATE(GetRefreshRate)
{
  // NOTE: While in windowed mode, we can just use the desktop refresh rate.
  //       Loop over all supported modes and pick the best one when fullscreen.
  SDL_DisplayMode mode;
  i32 displayIndex = SDL_GetWindowDisplayIndex(pWindow);
  // NOTE: Prepare a psuedo refresh rate in case we can't find it.
  i32 defaultRefreshRate = 60;
  if (SDL_GetDesktopDisplayMode(displayIndex, &mode) != 0)
    return defaultRefreshRate;
  if (mode.refresh_rate == 0)
    return defaultRefreshRate;

  return mode.refresh_rate;
}

internal
MSYS_PLATFORM_DISPLAY_CAPTURE(SaveScreenshot)
{
  // NOTE: SDL_RenderReadPixels tends to be a fairly slow operation
  //       Although, because of this being a one-off operation it should be fine.
  // FIXME: The path given to SDL_SaveBMP should be randomly generated.
  // REVIEW: We may be interested in changing the format instead of using BMP.
  u32 format = SDL_PIXELFORMAT_ARGB8888;

  SDL_Surface *pSurface =
    SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, format);
  SDL_RenderReadPixels(pRenderer, NULL, format, pSurface->pixels, pSurface->pitch);
  SDL_SaveBMP(pSurface, "screenshot.bmp");
  SDL_FreeSurface(pSurface);
}

internal
MSYS_PLATFORM_DISPLAY_RESIZE(ResizeDisplay)
{
  Assert(width > 0 && height > 0);
  if (!pRenderer)
    return;

  // NOTE: If the size doesn't change then a resize isn't necessary.
  if (width == pBuffer->width && height == pBuffer->height)
    return;

  pBuffer->width = width;
  pBuffer->height = height;
  u8 bytes_per_pixel = 4;
  pBuffer->pitch = pBuffer->width * bytes_per_pixel;

  if (pBuffer->pHandle)
    SDL_DestroyTexture(pBuffer->pHandle);

  pBuffer->pHandle = SDL_CreateTexture(pRenderer,
                                       SDL_PIXELFORMAT_RGBA8888,
                                       SDL_TEXTUREACCESS_TARGET,
                                       pBuffer->width, pBuffer->height);
}

// FIXME: Don't bother using this until it ends up doing more...
internal
MSYS_PLATFORM_DISPLAY_UPDATE(SwapBuffers)
{
  //SDL_RenderCopy(pRenderer, pBuffer->pHandle, 0, 0);
  SDL_RenderPresent(pRenderer);
}

internal
MSYS_PLATFORM_TIME_GET_TIME(GetSecondsElapsed) 
{
  Assert(!pMSYS);
  Assert(pMSYS->performanceCounterFrequency != 0);

  u64 counterElapsed = currentCounter - oldCounter;
  r32 secondsElapsed =
    ((r32)counterElapsed) / ((r32)pMSYS->performanceCounterFrequency);

  return secondsElapsed;
}

internal
MSYS_PLATFORM_EVENT_HANDLER(ProcessSystemEvents)
{
  switch (pEvent->type) {
    case SDL_WINDOWEVENT:
    {
      switch (pEvent->window.event) {
        case SDL_WINDOWEVENT_SHOWN: {
          SDL_Window *pWindow = SDL_GetWindowFromID(pEvent->window.windowID);
          SDL_Renderer *pRenderer = SDL_GetRenderer(pWindow);
                    
          MSYS_DisplayDimensions size =
            MSYS_PLATFORM_FUNC(GetDisplayDimensions)(pWindow);
                    
          MSYS_PLATFORM_FUNC(ResizeDisplay)(pRenderer, &pMSYS->backbuffer,
                                           size.width, size.height);

        }break;
        case SDL_WINDOWEVENT_CLOSE: {
          pMSYS->isRunning = false;
        }break;
        case SDL_WINDOWEVENT_SIZE_CHANGED: {
          SDL_Renderer *pRenderer =
            MSYS_PLATFORM_FUNC(GetRenderer)(pEvent->window.windowID);

          //MSYS_PLATFORM_FUNC(ResizeDisplay)(pRenderer, &pMSYS->backbuffer,
          //  pEvent->window.data1, pEvent->window.data2);
        }break;
        InvalidDefaultCase; // cppcheck-suppress[unreachableCode]
      }
    }break;
    InvalidDefaultCase; // cppcheck-suppress[unreachableCode]
  }
}