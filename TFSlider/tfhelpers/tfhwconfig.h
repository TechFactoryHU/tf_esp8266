#ifndef TFHwConfig_H
#define TFHwConfig_H
#include "../tfheader.h"
#include "Arduino.h"
#include "FS.h"

struct TFConfigHardware {
	char devname[30];
	byte type;
	uint8_t pin1;
	uint8_t pin2;
	uint8_t pin3;
    int param1;
};

struct TFConfigPixelSegment {
    char pxname[30];
    uint8_t hwid;
    int from;
    int to;
};

class TFHwConfig {
    private:
		String _filename;
		File openFile(String filename, char* type);
    
	public:
		TFHwConfig(String filename);
		void begin(void);
		
		bool load(void);
		bool write(void);
		
		void initHW(uint8_t count);
		void setRGB(uint8_t id, uint8_t redPin, uint8_t greenPin, uint8_t bluePin, String name);
		void setLed(uint8_t id, uint8_t pin, String name);
		void setPixel(uint8_t id, uint8_t pin, int pixels, String name, bool scpixel = false);
		void initPixelSegments(uint8_t count);
		void setPixelSegment(uint8_t id, uint8_t hwid, int from, int to, String name);
		
		TFConfigHardware *_hw;
		TFConfigPixelSegment *_pxseg;
		
		uint8_t _hws;
		uint8_t _rgbs;
		uint8_t _leds;
		uint8_t _pixels;
		uint8_t _pxsegs;
    
};

#endif // TFHwConfig_H

