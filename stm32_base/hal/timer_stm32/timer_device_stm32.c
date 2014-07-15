#include "timer_device.h"
#define TIMER_CHANNELS 3

unsigned int g_timer0_overflow;
unsigned int g_timer1_overflow;
unsigned int g_timer2_overflow;
unsigned int g_timer_counts[TIMER_CHANNELS];

static void init_timer_0(size_t divider, unsigned int slowTimerMode){
}

static void init_timer_1(size_t divider, unsigned int slowTimerMode){

}

static void init_timer_2(size_t divider, unsigned int slowTimerMode){

}

static unsigned int getTimer0Period(){
	return 0;
}

static unsigned int getTimer1Period(){
	return 0;
}

static unsigned int getTimer2Period(){
	return 0;
}

int timer_device_init(size_t channel, unsigned int divider, unsigned int slowTimerMode){
	switch(channel){
		case 0:
			init_timer_0(divider, slowTimerMode);
			return 1;
		case 1:
			init_timer_1(divider, slowTimerMode);
			return 1;
		case 2:
			init_timer_2(divider, slowTimerMode);
			return 1;
		default:
			return 0;
	}
}

void timer_device_get_all_periods(unsigned int *t0, unsigned int *t1, unsigned int *t2){
	*t0 = getTimer0Period();
	*t1 = getTimer1Period();
	*t2 = getTimer2Period();
}

void timer_device_reset_count(unsigned int channel){
	if (channel >= 0 && channel < TIMER_CHANNELS){
		g_timer_counts[channel] = 0;
	}
}

unsigned int timer_device_get_count(unsigned int channel){
	if (channel >= 0 && channel < TIMER_CHANNELS){
		return g_timer_counts[channel];
	}
	else{
		return 0;
	}
}

unsigned int timer_device_get_period(unsigned int channel){
	switch (channel){
		case 0:
			return getTimer0Period();
		case 1:
			return getTimer1Period();
		case 2:
			return getTimer2Period();
	}
	return 0;
}


