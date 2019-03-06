#ifndef TFEQ_H
#define TFEQ_H
#include "Arduino.h"
#include "tfheader.h"


/*#define TFEQ_MSGEQ7_STROBE 4
#define TFEQ_MSGEQ7_RESET 5
*/

struct TFEqData {
    uint8_t value;
    uint8_t max;
    uint8_t beat;
    int beatTime;
    unsigned long lastbeat;
};

class TFEq {
    private:
		TFEqData *_bdata;
		uint8_t _bars;
		uint8_t *_frames;
		uint8_t _framesize;
				
		bool msgeq7;
		void shiftFrames(uint8_t bar);
		void addFrame(uint8_t bar, uint8_t value);
		
		unsigned long _ts;
		unsigned long _beatts;
		unsigned long _eqts;
		
		int _analogMapMin  = 150;
		int _analogMapMax  = 1024;
		
		int dropTime 		= 30; 	//in ms
		uint8_t dropVMax	= 2;	//drop value / dropTime (from TFEqData.max)
		uint8_t dropVLevel  = 10;	//drop value / dropTime (from TFEqData.value)
		
		int beatDropTime	= 200;
		uint8_t dropVBeat	= 50;
		uint8_t eqInterval  = 30;
		    
	public:
		TFEq(uint8_t bars);
		void setup();
		void setAnalogMap(int min, int max);
		void initBeatDetection(int framesize);
		void data(uint8_t bar, uint8_t value);
		void shiftBarsLeft(uint8_t value);
		void shiftBarsRight(uint8_t value);
		void paramsFromString(String p);
		String paramsToString(void);
		
		uint8_t beat();
		uint8_t beat(uint8_t bar);
		void setDrop(int time, uint8_t dropVMax, uint8_t dropVLevel);
		
		void update(unsigned long ts = millis());
		TFEqData get(uint8_t bar);
		uint8_t getBeat(uint8_t bar);
		uint8_t bars() { return _bars; };
		int average(uint8_t bar);
		
		void setMSGEQ(bool status);
		inline bool getMSGEQ() { return msgeq7; }
		void readMSGEQ();
		bool hasMSGEQ();
    
};

#endif // TFEQ_H

