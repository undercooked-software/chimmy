
internal SDL_Texture * 
BMPToTexture(SDL_Renderer *pRenderer, const char *filename, b32 useColorKey)
{

  SDL_Texture *pTexture = 0;
  
  SDL_Surface *pBMP = SDL_LoadBMP(filename);
  if (!pBMP)
    return pTexture;
    
  if (useColorKey) {
    u32 format =
      SDL_MapRGB(pBMP->format, color_key.r, color_key.g, color_key.b);
    SDL_SetColorKey(pBMP, SDL_TRUE, format);
  }

  pTexture = SDL_CreateTextureFromSurface(pRenderer, pBMP);
  SDL_FreeSurface(pBMP);
  
  // NOTE: SDL_CreateTextureFromSurface() fills pTexture with the correct return value
  return pTexture;
}