#ifndef __TASK_H__
#define __TASK_H__

#include <library.h>

#define LED_TASK_TIMEOUT			500
#define LOW_BATTERY_THRESHOLD		576

void blink(void *pdata);
void serial(void *pdata);
void adcDemo(void *pdata);
void ledDemo(void *pdata);
void mpu6050Task(void *pdata);
void nrfRxTask(void *pdata);
void ledTask(void *pdata);

#endif
