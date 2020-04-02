#include "MQTTInterface.h"

#include "../GSM/gsm.h"
#include "../Systick/systick.h"



char TimerIsExpired(Timer* timer) {
	long left = timer->end_time - mqttTick;
	return (left < 0);
}


void TimerCountdownMS(Timer* timer, unsigned int timeout) {
	timer->end_time = mqttTick + timeout;
}


void TimerCountdown(Timer* timer, unsigned int timeout) {
	timer->end_time = mqttTick + (timeout * 1000);
}


int TimerLeftMS(Timer* timer) {
	long left = timer->end_time - mqttTick;
	return (left < 0) ? 0 : left;
}


void TimerInit(Timer* timer) {
	timer->end_time = 0;
}



