#include "input.h"

static bool joyButtonPressed[JOY_BUTTONS_NUM];
static bool joyButtonPressedOnce[JOY_BUTTONS_NUM];

static bool anyJoyButtonPressed;

void initInput()
{
	int i;
	for (i=0; i<JOY_BUTTONS_NUM; ++i) {
		joyButtonPressed[i] = false;
		joyButtonPressedOnce[i] = false;
	}

	eris_pad_init(0);
}

void updateInput()
{
	int i;
	uint32 paddata = eris_pad_read(0);

	anyJoyButtonPressed = false;
	for (i=0; i<JOY_BUTTONS_NUM; ++i) {
		if (paddata & (1 << i)) {
			joyButtonPressedOnce[i] = !joyButtonPressed[i];
			joyButtonPressed[i] = true;
			anyJoyButtonPressed = true;
		} else {
			joyButtonPressed[i] = false;
			joyButtonPressedOnce[i] = false;
		}
	}
}

bool isAnyJoyButtonPressed()
{
	return anyJoyButtonPressed;
}

bool isJoyButtonPressed(int joyButtonId)
{
	if (joyButtonId < 0 || joyButtonId >= JOY_BUTTONS_NUM) return false;
	return joyButtonPressed[joyButtonId];
}

bool isJoyButtonPressedOnce(int joyButtonId)
{
	if (joyButtonId < 0 || joyButtonId >= JOY_BUTTONS_NUM) return false;
	return joyButtonPressedOnce[joyButtonId];
}
