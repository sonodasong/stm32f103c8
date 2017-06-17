#include "led.h"
#include "pwm.h"

void ledSet(uint8 brightness)
{
	pwmSet(brightness);
}
