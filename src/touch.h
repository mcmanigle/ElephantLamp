#pragma once

#define TOUCH_IICSCL 10
#define TOUCH_IICSDA 15
#define TOUCH_INT 11
#define TOUCH_INT_GPIO GPIO_NUM_11
#define TOUCH_RES 16

extern unsigned long last_touch_millis;

void setupTouch(void (*callback)(int x, int y));
void loopTouch();
