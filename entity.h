
typedef struct TYPE_ENTITY {
  SDL_Texture *pTexture;
  u32 anim;

  SDL_Rect pCropRect; // { texture-x, texture-y, crop-width, crop-height }
  SDL_Rect pPosition; // { pos-x, pos-y, width, height}
} entity;

#define ENTITY_NO_ANIM 255

void EntityMove(entity *pEntity, i32 x, i32 y);
SDL_Rect * EntityCalculateCropRect(entity *pEntity);
void EntityRemove(entity *pEntity);