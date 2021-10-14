
internal
SDL_PLATFORM_RENDERER_GET_FROM_WINDOWID(GetRenderer) {
  SDL_Window *pWindow = SDL_GetWindowFromID(windowID);
  if (pWindow) {
    SDL_Renderer *pRenderer = SDL_GetRenderer(pWindow);
    if (pRenderer)
      return pRenderer;
  }
  return 0;
}

internal
SDL_PLATFORM_DISPLAY_GET_DIMENSIONS(GetDisplayDimensions) {
  sdl_display_dimensions dimensions;
  SDL_GetWindowSize(pWindow, &dimensions.width, &dimensions.height);
  return dimensions;
}

internal
SDL_PLATFORM_DISPLAY_GET_REFRESH_RATE(GetRefreshRate) {
  // NOTE: Since we're in windowed mode, we can just use the desktop refresh rate.
  // If we were in fullscreen we would loop over all supported modes and pick the best one.
  SDL_DisplayMode mode;
  i32 displayIndex = SDL_GetWindowDisplayIndex(pWindow);
  // NOTE: Prep a psuedo refresh rate in case we can't find it.
  i32 defaultRefreshRate = 60;
  if (SDL_GetDesktopDisplayMode(displayIndex, &mode) != 0)
    return defaultRefreshRate;
  if (mode.refresh_rate == 0)
    return defaultRefreshRate;

  return mode.refresh_rate;
}

internal
SDL_PLATFORM_DISPLAY_RESIZE(ResizeDisplay) {
  Assert(width > 0 && height > 0);

  if (!pRenderer)
    return;

  // NOTE: If the size didn't change, so a resize isn't necessary.
  if (width == pBuffer->width && height == pBuffer->height)
    return;

  pBuffer->width = width;
  pBuffer->height = height;
  if (pBuffer->pTexture)
    SDL_DestroyTexture(pBuffer->pTexture);

  pBuffer->pTexture = 
    SDL_CreateTexture(pRenderer,
                      SDL_PIXELFORMAT_ARGB8888,
                      SDL_TEXTUREACCESS_STREAMING,
                      pBuffer->width, pBuffer->height);

}

internal
SDL_PLATFORM_DISPLAY_CAPTURE(SaveScreenshot) {
  // NOTE: SDL_RenderReadPixels tends to be a fairly slow operation
  // Although, because of this being a one-off operation it should be fine.
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
SDL_PLATFORM_DISPLAY_UPDATE(SwapBuffers) {
  // REVIEW: Originally we would process the raw pixel data and feed it to the texture.
  // This is no longer and option, but may also not be necessary.
  SDL_RenderCopy(pRenderer, pBuffer->pTexture, 0, 0);
  SDL_RenderPresent(pRenderer);
}

internal
SDL_PLATFORM_TIME_GET_TIME(GetSecondsElapsed) {
  Assert(!pSDL);
  Assert(pSDL->performanceCounterFrequency != 0);

  u64 counterElapsed = currentCounter - oldCounter;
  r32 secondsElapsed =
    ((r32)counterElapsed) / ((r32)pSDL->performanceCounterFrequency);

  return secondsElapsed;
}

internal
SDL_PLATFORM_EVENT_HANDLER(ProcessSystemEvents) {
  switch (pEvent->type) {
    case SDL_WINDOWEVENT:
    {
      switch (pEvent->window.event) {
        case SDL_WINDOWEVENT_CLOSE: {
          pSDL->isRunning = false;
        }break;
        case SDL_WINDOWEVENT_SIZE_CHANGED: {
          SDL_Renderer *pRenderer =
            SDL_PLATFORM_FUNC(GetRenderer)(pEvent->window.windowID);
          
          SDL_RenderSetLogicalSize(pRenderer,
                                   pEvent->window.data1, pEvent->window.data2);

          // REVIEW: Should we be using ResizeDisplay here or should that be 
          // a one-off optional change for larger viewport adjustments?
        }
        InvalidDefaultCase;
      }
    }break;
    InvalidDefaultCase;
  }
}