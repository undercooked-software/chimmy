
typedef struct _Entity
{
  SDL_Texture *pTexture;
  u8 anim;
  u8 layer;

  SDL_Rect pCropRect; // { texture-x, texture-y, crop-width, crop-height }
  SDL_Rect pPosition; // { pos-x, pos-y, width, height}
} Entity;

#define ENTITY_NO_ANIM 255

void EntityMove(Entity *pEntity, i32 x, i32 y);
SDL_Rect * EntityCalculateCropRect(Entity *pEntity);
void EntityRemove(Entity *pEntity);