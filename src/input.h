#ifndef INPUT_H
#define INPUT_H

#include <eris/pad.h>

#include "types.h"

enum { JOY_A, JOY_B, JOY_C, JOY_D, JOY_E, JOY_F, JOY_SELECT, JOY_START, JOY_UP, JOY_RIGHT, JOY_DOWN, JOY_LEFT, JOY_BUTTONS_NUM };	// in order to HW defines (1 << n)

void initInput(void);
void updateInput(void);

bool isAnyJoyButtonPressed(void);
bool isJoyButtonPressed(int joyButtonId);
bool isJoyButtonPressedOnce(int joyButtonId);

#endif
