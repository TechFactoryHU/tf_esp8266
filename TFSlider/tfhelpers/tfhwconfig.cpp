#include "Arduino.h"
#include "FS.h"
#include "../tfheader.h"
#include "tfhwconfig.h"

TFHwConfig::TFHwConfig(String filename) {
	_filename = filename;
}

void TFHwConfig::begin() {
    Serial.println("TFConfig init");
    
	if (!SPIFFS.begin()){
	 #ifdef TF_DEBUG
		Serial.println("FS not ready...");
	 #endif
	}
	TFHwConfig::load();
}

File TFHwConfig::openFile(String filename, char* type) {
	File f = SPIFFS.open(filename, type);
	if (!f) {
		#ifdef TF_DEBUG 
			Serial.println("[TFHwConfig::openFile] Cant open "+filename+" file for "+String(type)+"... ");
		#endif
	}
	return f;
}

bool TFHwConfig::load() {
	File f = openFile("/"+_filename, "r"); 
	if (!f) { return false;}
	String line;
	char *seq;
	while ( (line = f.readStringUntil('\n')) != NULL) {
		seq = strtok(&line[0],"=");
		if (seq != NULL) {
			//hw config
			if (String(seq) == "HWS") {
				seq = strtok(NULL,"=");
				if (seq != NULL) { 
					initHW(atoi(seq));
				}
			}
			//hw pixel segments
			else if (String(seq) == "HWPS") {
				seq = strtok(NULL,"=");
				if (seq != NULL) { 
				   initPixelSegments(atoi(seq));
				}
			}
			//hw config
			else if (String(seq) == "HW") {
				seq = strtok(NULL,"=");
				if (seq != NULL) { 
					seq = strtok(seq,",");
					uint8_t index = atoi(seq);
					seq = strtok(NULL,",");
					uint8_t type = atoi(seq);
					uint8_t pin1 = 0;
					uint8_t pin2 = 0;
					uint8_t pin3 = 0;
         
					if (type == 1) {
						seq = strtok(NULL,",");
						pin1 = atoi(seq);
						seq = strtok(NULL,",");
						pin2 = atoi(seq);
						seq = strtok(NULL,",");
						pin3 = atoi(seq);
						setRGB(index, pin1,pin2,pin3, String(strtok(NULL,",")));
					}else if(type == 2) {
						seq = strtok(NULL,",");
						pin1 = atoi(seq);
						setLed(index, pin1, String(strtok(NULL,",")));
					}else if(type == 3) {
						int pixels = 0;
						seq = strtok(NULL,",");
						pin1 = atoi(seq);
						seq = strtok(NULL,",");
						pixels = atoi(seq);
						seq = strtok(NULL,",");
						pin3 = atoi(seq);
						setPixel(index, pin1, pixels, String(strtok(NULL,",")), pin3>0 ? true : false);
					}
				}
			}
			//hw: pixel segments config
			else if (String(seq) == "HWP") {
				seq = strtok(NULL,"=");
				if (seq != NULL) { 
				  seq = strtok(seq,",");
				  uint8_t index = atoi(seq);
				  seq = strtok(NULL,",");
				  uint8_t hwid = atoi(seq);
				  seq = strtok(NULL,",");
				  int from = atoi(seq);
				  seq = strtok(NULL,",");
				  int to = atoi(seq);
				  setPixelSegment(index, hwid, from, to, String(strtok(NULL,",")));
				}
			}
		}
	}
	return true;
}

bool TFHwConfig::write() {
	File f = openFile("/"+_filename,"w"); 
	if (!f) {return false;}
    f.print("HWS="+String(_hws)+"\n");
    for (int i=0; i<_hws; i++) {
  		if(_hw[i].type == 1) {
  			f.print("HW="+String(i)+","+String(_hw[i].type)+","+String(_hw[i].pin1)+","+String(_hw[i].pin2)+","+String(_hw[i].pin3)+","+String(_hw[i].devname)+"\n");
  		}
      else if (_hw[i].type==2) {
       f.print("HW="+String(i)+","+String(_hw[i].type)+","+String(_hw[i].pin1)+","+String(_hw[i].devname)+"\n");
      }
      else if (_hw[i].type==3) {
       f.print("HW="+String(i)+","+String(_hw[i].type)+","+String(_hw[i].pin1)+","+String(_hw[i].param1)+","+String(_hw[i].pin3)+","+String(_hw[i].devname)+"\n");
      }
    }
    f.print("HWPS="+String(_pxsegs)+"\n");
    for (int i=0; i<_pxsegs; i++) {
      f.print("HWP="+String(i)+","+String(_pxseg[i].hwid)+","+String(_pxseg[i].from)+","+String(_pxseg[i].to)+","+String(_pxseg[i].pxname)+"\n");
    }
    f.close();
    return true;
}

void TFHwConfig::initHW(uint8_t count) {
	_rgbs = _leds = _pixels = 0;
	int numBytes = count * sizeof(struct TFConfigHardware);
  if ((_hw = (TFConfigHardware *)malloc(numBytes))) {
      _hws = count;
  }else {
      _hws = 0;
  }
}

void TFHwConfig::setRGB(uint8_t id, uint8_t redPin, uint8_t greenPin, uint8_t bluePin, String name) {
	_hw[id] = TFConfigHardware{};
   name.toCharArray(_hw[id].devname,30);
	_hw[id].pin1 = redPin;
	_hw[id].pin2 = greenPin;
	_hw[id].pin3 = bluePin;
	_hw[id].type = 1;
	pinMode(redPin, OUTPUT);
	pinMode(greenPin, OUTPUT);
	pinMode(bluePin, OUTPUT);
	analogWrite(redPin,0);
	analogWrite(greenPin,0);
	analogWrite(bluePin,0);
	_rgbs++;
}

void TFHwConfig::setLed(uint8_t id, uint8_t pin, String name) {
	_hw[id] = TFConfigHardware{};
  name.toCharArray(_hw[id].devname,30);
	_hw[id].pin1 = pin;
	_hw[id].type = 2;
	pinMode(pin, OUTPUT);
	analogWrite(pin,0);
	_leds++;
}

void TFHwConfig::setPixel(uint8_t id, uint8_t pin, int pixels, String name, bool scpixel) {
	_hw[id] = TFConfigHardware{};
	name.toCharArray(_hw[id].devname,30);
	_hw[id].pin1 = pin;
	_hw[id].type = 3;
    _hw[id].param1 = pixels;
	_hw[id].pin3 = scpixel ? 1 : 0;
	_pixels++;
}

void TFHwConfig::initPixelSegments(uint8_t count) {
  _pxsegs = 0;
  int numBytes = count * sizeof(struct TFConfigPixelSegment);
  if ((_pxseg = (TFConfigPixelSegment *)malloc(numBytes))) {
      _pxsegs = count;
  }else {
      _pxsegs = 0;
  }
}

void TFHwConfig::setPixelSegment(uint8_t id, uint8_t hwid, int from, int to, String name) {
  _pxseg[id].hwid = hwid;
  _pxseg[id].from = from;
  _pxseg[id].to = to;
  name.toCharArray(_pxseg[id].pxname,30);
}


