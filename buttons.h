#ifndef BUTTONS_H
#define BUTTONS_H

enum {
/* d-pad */
  GP2X_BUTTON_UP,         /* 0 */
  GP2X_BUTTON_UPLEFT,
  GP2X_BUTTON_LEFT,
  GP2X_BUTTON_DOWNLEFT,
  GP2X_BUTTON_DOWN,
  GP2X_BUTTON_DOWNRIGHT,
  GP2X_BUTTON_RIGHT,
  GP2X_BUTTON_UPRIGHT,
  GP2X_BUTTON_CLICK = 18,
/* start+select */
  GP2X_BUTTON_START = 8,
  GP2X_BUTTON_SELECT,
/* shoulders */
  GP2X_BUTTON_L,          /* 10 */
  GP2X_BUTTON_R,
/* face buttons */
  GP2X_BUTTON_A,          /* 12 */
  GP2X_BUTTON_B,
  GP2X_BUTTON_X,
  GP2X_BUTTON_Y,
/* volume controls */
  GP2X_BUTTON_VOLUP,      /* 16 */
  GP2X_BUTTON_VOLDOWN,
/* -------------- */
  GP2X_BUTTON_COUNT = 19
};

#endif /* BUTTONS_H */
