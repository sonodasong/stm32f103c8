#include "task.h"

static int16 data[8];

static int16 magX;
static int16 magY;
static int16 magZ;

static boolean ledRdy = FALSE;

static uint16 hex16ToInt(char *hex);

void blink(void *pdata) {
	(void)pdata;
	while (1) {
		ledToggle();
		OSTimeDly(25);
	}
}

void serial(void *pdata) {
	char *str;

	(void)pdata;
	while (1) {
		usart1Read(&str);
		usart1Print(str);
	}
}

void adcDemo(void *pdata) {
	(void)pdata;
	while (1) {
		usart1PendingHex16(adc1Read());
		usart1PendingPrint("\r\n");
		OSTimeDly(10);
	}
}

void ledDemo(void *pdata) {
	uint8 i;

	i = 0;
	(void)pdata;
	while (1) {
		ledSet(i);
		i += 1;
		OSTimeDly(1);
	}
}

/*
void mpu6050Task(void *pdata) {
	uint8 mpuIntStatus;
	uint8 fifoBuffer[48];
	int16 quaternion[4];

	(void)pdata;
	while (mpu6050DMPInitFromTask());
	while (1) {
		mpu6050DMPSemPend();
		mpuIntStatus = MPU6050getIntStatus();
		if (mpuIntStatus & 0x02) {
			MPU6050getFIFOBytes(fifoBuffer, 48);
			MPU6050dmpGetQuaternion(quaternion, fifoBuffer);
			usart1PendingHex16(quaternion[0]);
			usart1PendingHex16(quaternion[1]);
			usart1PendingHex16(quaternion[2]);
			usart1PendingHex16(quaternion[3]);
			usart1PendingPrint("\r\n");
		} else if (mpuIntStatus & 0x10) {
			MPU6050resetFIFO();
		} else {
			MPU6050resetFIFO();			
		}
	}
}
*/

void mpu6050Task(void *pdata) {
	OS_CPU_SR cpu_sr;
	uint8 mpuIntStatus;
	uint8 fifoBuffer[48];
	int16 orientation[7];

	(void)pdata;
	while (mpu6050DMPInitFromTask());
	while (1) {
		mpu6050DMPSemPend();
		mpuIntStatus = MPU6050getIntStatus();
		if (mpuIntStatus & 0x02) {
			MPU6050getFIFOBytes(fifoBuffer, 48);
			MPU6050dmpGetQuaternion(orientation, fifoBuffer);
			MPU6050dmpGetMag(orientation + 4, fifoBuffer);
			OS_ENTER_CRITICAL();
			data[0] = orientation[0];
			data[1] = orientation[1];
			data[2] = orientation[2];
			data[3] = orientation[3];
			data[4] = orientation[5] - magX;
			data[5] = orientation[4] - magY;
			data[6] = -orientation[6] - magZ;
			OS_EXIT_CRITICAL();
		} else if (mpuIntStatus & 0x10) {
			MPU6050resetFIFO();
		} else {
			MPU6050resetFIFO();			
		}
	}
}

void nrfRxTask(void *pdata) {
	OS_CPU_SR cpu_sr;
	int16 orientation[8];
	uint8 buf[16];
	uint8 size;
	int16 mag[3];
	uint8 i;

	(void)pdata;
	nrf24l01pInitRxFromTask();
	for (i = 0; i < 6; i++) {
		*((uint8 *)mag + i) = eepromRead(i);
	}
	OS_ENTER_CRITICAL();
	magX = mag[0];
	magY = mag[1];
	magZ = mag[2];
	OS_EXIT_CRITICAL();
	while (1) {
		OS_ENTER_CRITICAL();
		orientation[0] = data[0];
		orientation[1] = data[1];
		orientation[2] = data[2];
		orientation[3] = data[3];
		orientation[4] = data[4];
		orientation[5] = data[5];
		orientation[6] = data[6];
		orientation[7] = data[7];
		OS_EXIT_CRITICAL();
		nrf24l01pTxAckPay((uint8 *)orientation, 16);
		nrf24l01pRxAckPay(buf, &size, 0xFFFFFFFF);
		if (size == 12) {
			for (i = 0;i < 3; i++) {
				mag[i] = hex16ToInt((char *)buf + (i << 2));
			}
			mag[0] += magX;
			mag[1] += magY;
			mag[2] += magZ;
			OS_ENTER_CRITICAL();
			magX = mag[0];
			magY = mag[1];
			magZ = mag[2];
			OS_EXIT_CRITICAL();
			for (i = 0; i < 6; i++) {
				eepromWrite(i, *((uint8 *)mag + i));
			}
			ledRdy = TRUE;
		}
	}
}

void ledTask(void *pdata) {
	uint8 time;
	uint8 mask;
	uint8 brightness;

	(void)pdata;
	time = 0;
	brightness = 0;
	while (1) {
		data[7] = adc1Read();
		if (ledRdy) {
			if (brightness == 255) {
				ledRdy = FALSE;
				ledSet(0);
				brightness = 0;
			} else {
				ledSet(brightness);
				brightness += 5;
			}
		} else {
			if (data[7] < LOW_BATTERY_THRESHOLD) {
				mask = 0x0F;
			} else {
				mask = 0x3F;
			}
			if (time & mask) {
				ledSet(0);
			} else {
				ledSet(255);
			}
			time++;
		}
		OSTimeDly(2);
	}
}

static uint16 hex16ToInt(char *hex) {
	return ((hex[3] - '0') << 12) | ((hex[2] - '0') << 8) | ((hex[1] - '0') << 4) | (hex[0] - '0');
}
