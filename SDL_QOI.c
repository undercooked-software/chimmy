#include <SDL_video.h>
#define QOI_NO_STDIO
#define QOI_MALLOC(size)  SDL_malloc(size)
#define QOI_FREE(p)       SDL_free(p)
#define QOI_ZEROARR(arr)  SDL_memset((arr), 0, sizeof(arr))
#define QOI_IMPLEMENTATION
#include "qoi.h"

static void*
fload(const char *path, size_t *out_size) {
  FILE* fh = fopen(path, "rb");
  void* buffer;
  size_t size;
  if (!fh) { return NULL; }

  fseek(fh, 0, SEEK_END);
  size = ftell(fh);
  fseek(fh, 0, SEEK_SET);

  buffer = malloc(size);
  if (!buffer) { return NULL; }

  if (!fread(buffer, size, 1, fh)) {
    free(buffer);
    return NULL;
  }
  fclose(fh);

  *out_size = size;
  return buffer;
}

static void*
get_pixels(const char* path, qoi_desc* desc) {
  void* data;
  void* pixels;
  size_t size;

  data = fload(path, &size);
  if (!data) { return NULL; }

  pixels = qoi_decode(data, size, desc, 0);
  if (!pixels) {
    free(pixels);
    return NULL;
  }

  free(data);
  return pixels;
}

static SDL_Surface*
SDL_LoadQOI(const char* path) {
  qoi_desc desc;
  SDL_Surface* surface;
  SDL_Surface* optimized;
  void* pixels = get_pixels(path, &desc);
  if (!pixels) { return NULL; }

  /* add code here to adjust argb mask for endian size? */

  surface =
    SDL_CreateRGBSurfaceFrom(pixels, desc.width, desc.height,
                             desc.channels * 8, desc.width * desc.channels,
                             0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
  optimized = SDL_DisplayFormat(surface);

  SDL_FreeSurface(surface);
  return optimized;
}
