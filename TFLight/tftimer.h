#ifndef TFTIMER_H
#define TFTIMER_H
#include "tfheader.h"
#include "helpers.h"

struct TFTIMER_ITEM {
    byte d; 			//0: off, 1-7 days from monday to sunday, 8 - every day, 9 - every weekday, 10 - every weekend, 10 > = every (x-10) day (11 = every day,12 = every second day,13 = every third day, ect...)
    byte h; 			//0-23 hour, 
    byte m; 			//0: every month, or 1-12 months
   
	byte action;		//0: turn off, 1: turn on, 2: start_program
	int action_value;	
	RGB color;	
	char hwids[5];
};

struct TFTIME {
	byte d;
	byte h;
	byte m;
	byte s;
	unsigned long ts;
};

class TFTimer {
	public:
		TFTimer(TFCustomCfg *config);
		void start();
		void stop();
		bool load();
		bool save();
		TFTIME getTime();
		void sleep(int intv);
		bool updateTime(bool forced);
		void update();
		bool set(int index, TFTIMER_ITEM item);
		void jobHandler(void (*jobHandlerCallback)(TFTIMER_ITEM));
		
	private:
		bool enabled;
		TFCustomCfg *_cfg;
		unsigned long timestamp;
		unsigned long lastupdate;
		unsigned long lastrun;
		unsigned long lasterror;	
		TFTIMER_ITEM timers[TFTIMER_TIMERS];
		TFTIME ltime;
		void (*jobHandlerCallback)(TFTIMER_ITEM);
};

#endif // TFTIMER_H