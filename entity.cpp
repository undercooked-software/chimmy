
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
  
  i32 textureWidth;
  SDL_QueryTexture(pEntity->pTexture, NULL, NULL, &textureWidth, NULL);

  // integer division to floor the value
  i32 textureModulus = textureWidth / frameWidth;

  // REVIEW: Maybe add data santizing checks if anim is OoB of texture?
  pEntity->pCropRect.w = frameWidth;
  pEntity->pCropRect.h = frameHeight;
  pEntity->pCropRect.x = (pEntity->anim % textureModulus) * frameWidth;
  pEntity->pCropRect.y = (pEntity->anim / textureModulus) * frameHeight;

  return &pEntity->pCropRect;
}

void
EntityRemove(entity *pEntity) {
  // FIXME For now this will just clear out the texture
  SDL_DestroyTexture(pEntity->pTexture);
}