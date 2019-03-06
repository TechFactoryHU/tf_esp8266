#ifndef TFLIGHTANIM_H
#define TFLIGHTANIM_H
#include "FS.h"
#include "helpers.h"
#include "tfheader.h"
#include "tflights.h"

class TFLightAnim {
	public:
		TFLightAnim();
		 ~TFLightAnim();
		 
		void setTFLights(TFLights *l);
		bool load(int prog_id, uint8_t type);
		bool load(String filename, uint8_t type);
		
		bool save();
		inline int getPid(void) { return program_id; }
		inline void setPid(int id) { program_id = id; }
		inline String getName(void) { return String(prog_name); }
		inline int getType(void) { return type; }
		
		
		inline uint8_t get_hws(void) { return _hws; }
		inline uint8_t get_rgbs(void) { return _rgbs; }
		inline uint8_t get_pixels(void) { return _pixels; }
		inline uint8_t get_colors(void) { return _colors; }
		inline uint8_t get_anims(void) { return _anims; }
		inline RGB getColor(int index) { return (index<_colors)?color[index]:RGB{0,0,0}; }
		inline uint8_t getColor(int index, int c_id) { 
			if (index<_colors) {
				switch(c_id) {
					case 0: return color[index].r;
					case 1: return color[index].g;
					case 2: return color[index].b;
				}
			}
			return 0;
		}
		
		//bool write(String filename);
		bool remove();
		
		//int getNextProgId(uint8_t type);
		//int saveTo(uint8_t type, int prog_id, String data);
		
		//TFLightAnim_ItemList getProgList(uint8_t type, uint8_t start = 0);
		
		
		void setTFLights(TFLights &light);
		
		bool setType(uint8_t t, uint8_t c, uint8_t a = 0);
		bool setName(String name);
		bool setColors(uint8_t c);
		
		bool addHw(uint8_t hwid);
		int getHw(uint8_t index);
		bool remHw(uint8_t hwid);
		bool findHw(uint8_t hwid);
		void clearHws(void);
		bool resetHws(void);
		int hwCount(void);
		
		void reset();
		
		bool addColor(int index, RGB c);
		
		bool addPixelAnimation(int index, TFLightAnimPixelA prog);
		bool addRGBProgram(int index, TFLightAnimRGB prog);
		bool addPixelProgram(int index, TFLightAnimPixel prog);
		
		inline void setAnimLength(unsigned int l) { anim_length = l; }
		inline void resetOnEnd(bool r) { reset_on_end = r; }
		void update(unsigned long millis);
		
	private:
		uint8_t aindex;
		unsigned long frame;
		unsigned long _lastframe;
		unsigned long startTime;
		unsigned int  anim_length;
		
		TFLights *lights;
		
		byte type; //0-unknown,1-rgb,2-pixel
		bool reset_on_end;
		
		int program_id;
		char prog_name[30];
		
		unsigned int TMPPixelCount;
		unsigned int TMPPixelRangesCount;
		int TMPPixelLastId;
		int TMPPixels[TFLIGHTANIM_MAX_PIXEL_COUNT];
		TFLightPixelRange TMPPixelRanges[TFLIGHTANIM_MAX_PIXEL_RANGES];
		
		void _pix_fadeColor(RGB *c, RGB a ,RGB b, float p);
		int _pixelHelperString(String str);
		int _pixelHelper(char* pixels = NULL);
		void _setTMPPixel(int id);
		
		unsigned char _setParams(bool b[8]);
		unsigned char _setParam(int index, bool val, unsigned char c);
		bool _getParam(unsigned char c, int id);
		void _getParams(unsigned char c, bool *r);
		
		uint8_t _hws;
		uint8_t _rgbs;
		uint8_t _pixels;
		uint8_t _colors;
		uint8_t _anims;
		unsigned int _maxpxcount;
		
		byte hw[5];
		TFLightAnimRGB *rgb;
		TFLightAnimPixel *pixel;
		TFLightAnimPixelA *anim;
		RGB *color;
		TFLightAnimPixelS *pxstatus;
		
		bool _allocAnims(uint8_t num);
		bool _allocPixel(uint8_t num);
		bool _allocRGB(uint8_t num);
		bool _allocColor(uint8_t num);
		bool _allocPixelStatus(uint8_t num);
		
		void updatePixels(void);
		void updateRGBs(void);
		void nextRGBProgram(void);
		void setDevColor(RGB c);
		RGB getColor(uint8_t id);
		void fadeColor(RGB a, RGB b, float p);
		int _fadeColorHelper(int a, int b, float p);
		RGB colorWheel(uint8_t pos);
		TFLightAnimPixelA getAnimation(uint8_t index);
		void setAnimation(uint8_t index, TFLightAnimPixelA  a);
		void _resetPixelsStatus();
		void _setPixelsStatus(RGB color, unsigned char c);
		
		
};
#endif // TFLIGHTANIM_H

