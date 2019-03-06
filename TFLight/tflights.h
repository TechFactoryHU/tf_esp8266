#ifndef TFLIGHTS_H
#define TFLIGHTS_H
#include <Adafruit_NeoPixel.h>
#include "helpers.h"
#include "tfeq.h"
#include "tfheader.h"

#ifndef TFLIGHTANIM_PARAM_COUNT
#define TFLIGHTANIM_PARAM_COUNT 5
#endif

#ifndef TFLIGHT_FRAME_TIME
#define TFLIGHT_FRAME_TIME 30
#endif

#ifndef TFLIGHTANIM_SAVED_PROG_COUNT
#define TFLIGHTANIM_SAVED_PROG_COUNT 30
#endif

#ifndef TFLIGHTANIM_MAX_PIXEL_COUNT
#define TFLIGHTANIM_MAX_PIXEL_COUNT 50
#endif

#ifndef TFLIGHTANIM_MAX_PIXEL_RANGES
#define TFLIGHTANIM_MAX_PIXEL_RANGES 10
#endif

#ifndef TFLIGHTANIM_PROGLIST_LIMIT
#define TFLIGHTANIM_PROGLIST_LIMIT 5
#endif


struct TFLightConfig {
	char devname[30];
	byte type;		//1=led, 2=rgb led, 3=neopixel, 4=switch
	uint8_t pin1;
	uint8_t pin2;
	uint8_t pin3;
    int param1;
};

struct TFLightPixelGroup {
    char pxname[30];
    uint8_t gid;
    uint8_t hwid;
    int start;
    int end;
};

struct TFLightState {
	uint8_t mode;
	//0 = off
	//1 = manual color
	//2 = animation prog
	//3 = eq (manual mode)
	//4 = eq (auto mode)
	//5 = group mode (pixel)
	
	//type depends on the mode
	uint8_t type;
	
	//mode 1 =  0=simple color, 1=simple color anim
	//mode 2 =  program_id
	//mode 3 =  eq mode
		//0=auto,	
		//1=blink_beat,	
		//2=blink_beat_rev,	
		//3=by volume,
		//4=pixeleq (fromleft) 	
		//5=pixeleq(fromleft+max)  
		//6=pixeleq(fromright)	
		//7=pixeleq(fromright + max)
		//8=pixeleq (half)	
		//9=pixeleq(half+max)		
		//10=pixeleq(half-middle)		
		//11=pixeleq(half-middle + max)

	uint8_t param1;
	//mode 3 = eq source (bar)
	uint8_t param2;
	//mode 3 = eq color (color palette id)
	uint8_t param3;
	//mode 3 = eq color (color id)
	uint8_t param4;
	RGB color;
};

struct TFLightPixelRange {
	int from;			//start pixel id or percentage
	int to;				//end pixel id or percentage
	bool is_percentage; //from and to values are percentages (0-100%) of a pixel range
};

struct TFLightAnimRGB {
	byte start_color;
	byte end_color;
	unsigned long fade; 
    unsigned long hold;
    byte hold_p;		//hold position (1-before,2-both,3-after)
    byte random;	
    byte repeat;
    byte repeat_v;
};

struct TFLightAnimPixel {
	unsigned long fstart;  //start frame
	unsigned long fend; 	//end frame
	unsigned int frames;	//frames count 										
	int position;			//position, default=0
    String pixels;			//affected pixels format:  	1,2,3,10,13,16-30  (ranges also supported)
							//or by percentage:			1x25%  (first 25%, (0-25%))
							//							2x25%  (second 25%, (25%-50%))
    byte anim[5];			//animations (effects) - TFLightAnimPixelA  
    byte params; 			
    //0=reset_pixels_on_start
    //1=pixel_pattern_repeat 
    //2=pixel_pattern_align_to_last 
    //3=reset_position_on_start
};

struct TFLightAnimPixelA {
	unsigned long fstart;
	unsigned long fend;
	byte action;		
	uint8_t paramb1;
	uint8_t paramb2;
	uint8_t paramb3;
	int parami1;
	int parami2;
	byte params; 	
    //0=reset_pixels_on_start, 
    //1=pixel_pattern_repeat, 
    //2=pixel_pattern_align_to_last, 
    //3=loop
	
	/*
	*	actions	
		0 - turn off selected pixels
		1 - switch colors
			paramb1 - color1;
			paramb2	- color2;
		2 - fade colors
			paramb1 - color1;
			paramb2	- color2;
			
		3 - rainbow between colors
		4 - rainbow from color1
		
		.... placeholders
		
		20 - move 	(move [paramb1|paramb2] pixel on every [frame] frame (~30ms=1frame))
			params (modifier)
			4=path anim
			
			
			paramb1 - move(+)
			paramb2 - move(-)
			paramb3 - path_anim: path lifetime (0-255; 0:default(10) 1:slowest 255:fastest) 
			parami1 - frame
						
		21 - limit 
			paramb1 (type) 1=%, 2=position
			paramb2	(from%)
			paramb3	(to%)
			parami1 - min
			parami2 - max
	*/
};

struct TFLightAnimPixelS {
	RGB color;
	uint8_t lifetime;
	byte params;
};

struct TFLightsProgItem {
	uint8_t program_id;
	uint8_t program_type;
	char name[30];
};

struct TFLightsProgList {
	TFLightsProgItem items[TFLIGHTANIM_PROGLIST_LIMIT] = {};
	uint8_t count; 
};

class TFLights {
	public:
		TFLights(String filename);
		 ~TFLights();
		TFLightState *status;
		
		void load(void (*ifconfigempty_callback)(TFLights*));
		void begin(uint8_t hwcount);
		void beginPixelGroup(uint8_t segs);
		
		void setRGB(uint8_t id, uint8_t redPin, uint8_t greenPin, uint8_t bluePin, String name);
		void setLed(uint8_t id, uint8_t pin, String name);
		void setSwitch(uint8_t id, uint8_t pin, String name);
		void setPixel(uint8_t id, uint8_t pin, int pixels, String name, bool scpixel = false);
		void setPixelGroup(uint8_t id, uint8_t hwid, int from, int to, String name);
		
		void setup();
		
		inline int getHwCount(bool abs) { return abs ? _hws+_pxsegs : _hws; }
		
		inline TFLightConfig getHw(uint8_t id) { 
			if (id >= 0 && id < _hws) {
				return hw[id];
			}
			else { return TFLightConfig{"", 0, 0, 0, 0, 0}; }
		}
		
		inline bool isPixelGroup(uint8_t hwid) {
			if (hwid >= _hws) { return true; }
			return false;
		}
		
		inline bool hasPixelGroup(void) { return _pxsegs>0?true:false; }
		bool hasPixelGroup(uint8_t hwid);
		int getPixelGroupCount(void);
		int getPixelGroupCount(uint8_t hwid);
		
		int getType(uint8_t hwid);
		inline int getHwIdByGroup(uint8_t pg) {
			if (pg >= _hws && pg < _hws+_pxsegs) {
				return pxsegs[pg].hwid;
			}
		}
		
		void setStatus(uint8_t hwid, uint8_t mode, uint8_t type);
		void setStatus(uint8_t hwid, uint8_t mode, uint8_t type, uint8_t p1, uint8_t p2, uint8_t p3, uint8_t p4, RGB color);
		void setStatus(uint8_t hwid, TFLightState s);
		void setColor(RGB color, uint8_t hwid);
		void setRawColor(int r, int g, int b, uint8_t hwid);
		void setBrightness(float p, uint8_t hwid);
		void setPixelsColor(RGB color, uint8_t hwid);
		void setPixelsColor(RGB color, uint8_t hwid, uint8_t groupid);
		void setPixelsColor(RGB color, uint8_t hwid, String pixels);
		void setPixelsColor(RGB color, uint8_t hwid, String pixels, char *delimiter);
		
		void setPixelsGroupColor(RGB color, uint8_t groupid);
		int getPixelCount(uint8_t hwid);
		
		void setPixelColor(RGB color, int pixel, uint8_t hwid);
		RGB getPixelColor(uint8_t hwid,int pixel);
		
		TFLightPixelGroup getPixelGroup(uint8_t groupid);
		TFLightPixelGroup getPixelGroup(uint8_t hwid, uint8_t index);
		
		void autoEQMode(TFEqData eq);
		void EQDisplay(TFEq *eq);
		void setEQColors(RGB *colors, uint8_t size);
		void setPixelEq(uint8_t hwid, TFEqData eq);
		void setRGBEq(uint8_t h, TFEqData eq);
		RGB getEQColor(uint8_t index, uint8_t sub);
		void setAutoEQMode(uint8_t autocolor, uint8_t autotype);
		int getAutoEQMode();
		
		void update(void);
		 
		uint8_t EQType; //0 = remote, 1 = internal (MSGEQ7)
		uint8_t _autoColorId;
		
		inline void setEqAutoTreshold(uint8_t t, uint8_t c) { 
			if (t > 0) { _autoEqTreshold = t; }
			if (c > 0) { _autoEqCTreshold = c; }
		}


	private:
		String _configfile;
		
		uint8_t _hws;
		uint8_t _pxsegs;
		uint8_t _rgbs;
		uint8_t _leds;
		uint8_t _pixels;
		uint8_t _sws;
		
		TFLightConfig *hw;
		TFLightPixelGroup *pxsegs;
		Adafruit_NeoPixel *pixels;
		
		bool dirty;
		void _pixelEqbar(TFLightPixelGroup seg, uint8_t stid, uint8_t value, uint8_t max, bool reverse = false);
		void _setPixelHelper(RGB color,  uint8_t hwid, char *pixchr, char *delimiter);
		
		uint8_t _eqcolors;
		RGB *EQColors;
		
		uint8_t _autoEqMode;
		uint8_t _autoEqHelper;
		uint8_t _autoEqCHelper;
		
		uint8_t _autoEqLedId;
		uint8_t _autoEqRGBId;
		uint8_t _autoEqPixelId;
		uint8_t	_autoEqTreshold;
		uint8_t	_autoEqCTreshold;
};


#endif // TFLIGHTS_H

