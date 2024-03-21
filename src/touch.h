#pragma once

#define TOUCH_IICSCL 10
#define TOUCH_IICSDA 15
#define TOUCH_INT 11
#define TOUCH_RES 16

extern unsigned long last_touch_millis;

void setupTouch();
void loopTouch(void (*callback)(int x, int y));
