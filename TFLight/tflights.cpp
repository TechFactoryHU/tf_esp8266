#include "tfheader.h"
#include "tflights.h"

TFLights::TFLights(String filename) : dirty(false), pixels(NULL) {
	_configfile = filename;
	EQType = 0;
	_autoEqMode = 11;
	_autoEqTreshold = 125;
	_autoEqCTreshold = 125;
	_autoEqRGBId = 1;
	_autoEqLedId = 1;
	_autoEqPixelId = 1;
	_autoColorId = 0;
	_autoEqCHelper = 0;
	_autoEqHelper = 0;
}

TFLights::~TFLights() {
	free(hw);
	free(pxsegs);
	free(status);
}

void TFLights::load(void (*ifconfigempty_callback)(TFLights*)) {
	//TODO: load&save hw config
	ifconfigempty_callback(this);	
}

void TFLights::begin(uint8_t hwcount) {
	int numBytes = hwcount * sizeof(struct TFLightConfig);
	if ((hw = (TFLightConfig *)malloc(numBytes))) {
	  _hws = hwcount;
	}else {
	  _hws = 0;
	}
}

void TFLights::beginPixelGroup(uint8_t segs) {
	_pxsegs = 0;
	int numBytes = segs * sizeof(struct TFLightPixelGroup);
	if ((pxsegs = (TFLightPixelGroup *)malloc(numBytes))) {
	  _pxsegs = segs;
	}else {
	  _pxsegs = 0;
	}
}

void TFLights::setRGB(uint8_t id, uint8_t redPin, uint8_t greenPin, uint8_t bluePin, String name) {
	hw[id] = TFLightConfig{};
    name.toCharArray(hw[id].devname,30);
	hw[id].pin1 = redPin;
	hw[id].pin2 = greenPin;
	hw[id].pin3 = bluePin;
	hw[id].type = 1;
	_rgbs++;
}

void TFLights::setLed(uint8_t id, uint8_t pin, String name) {
	hw[id] = TFLightConfig{};
	name.toCharArray(hw[id].devname,30);
	hw[id].pin1 = pin;
	hw[id].type = 2;
	_leds++;
}

void TFLights::setSwitch(uint8_t id, uint8_t pin, String name) {
	hw[id] = TFLightConfig{};
	name.toCharArray(hw[id].devname,30);
	hw[id].pin1 = pin;
	hw[id].type = 4;
	_sws++;
}

void TFLights::setPixel(uint8_t id, uint8_t pin, int pixels, String name, bool scpixel) {
	hw[id] = TFLightConfig{};
	name.toCharArray(hw[id].devname,30);
	hw[id].pin1 = pin;
	hw[id].type = 3;
    hw[id].param1 = pixels;
	hw[id].pin3   = scpixel ? 1 : 0;
	_pixels++;
}

void TFLights::setPixelGroup(uint8_t id, uint8_t hwid, int from, int to, String name) {
  pxsegs[id].hwid = hwid;
  pxsegs[id].start = from;
  pxsegs[id].end= to;
  pxsegs[id].gid = id;
  name.toCharArray(pxsegs[id].pxname,30);
}

void TFLights::setup() {
    if (_pixels > 0) {
		pixels = new Adafruit_NeoPixel[_pixels];
    }
    int numBytes = (_hws+_pxsegs) * sizeof(struct TFLightState);
	if ((status = (TFLightState *)malloc(numBytes))) {
	}else {
	   return;
	}
    uint8_t pxid = 0;
    for (int i=0; i<_hws; i++) {
		//rgb strip
		if (hw[i].type == 1) {
			pinMode(hw[i].pin1, OUTPUT);
			pinMode(hw[i].pin2, OUTPUT);
			pinMode(hw[i].pin3, OUTPUT);
			analogWrite(hw[i].pin1,0);
			analogWrite(hw[i].pin2,0);
			analogWrite(hw[i].pin3,0);
		}
		//led strip
		else if (hw[i].type == 2) {
			pinMode(hw[i].pin1, OUTPUT);
			analogWrite(hw[i].pin1,0);
		}
		//neopixel strip
		else if (hw[i].type == 3) {
			pixels[pxid].updateType(NEO_GRB + NEO_KHZ800);
			pixels[pxid].setPin(hw[i].pin1);
			pixels[pxid].updateLength(hw[i].pin3 >0 ? hw[i].param1+1 : hw[i].param1);
			pixels[pxid].begin();
			hw[i].pin2 = pxid;
			pxid++;
		}
		//switch (relay)
		else if (hw[i].type == 4) {
			pinMode(hw[i].pin1, OUTPUT);
			analogWrite(hw[i].pin1,0);
		}
		
		status[i] = TFLightState{0,0,0,0,0};
		status[i].color = RGB{0,0,0};
    }
    
    for (int i=0; i<_pxsegs; i++) {
		status[i + _hws] = TFLightState{0,0,0,0,0};
		status[i + _hws].color = RGB{0,0,0};
    }
}

bool TFLights::hasPixelGroup(uint8_t hwid) {
	if (_pxsegs>0) {
		for (int i=0; i<_pxsegs;i++) {
			if (pxsegs[i].hwid == hwid) {
				return true;
			}
		}
	}
	return false;
}

int TFLights::getPixelGroupCount() {
	return _pxsegs;
}

int TFLights::getPixelCount(uint8_t hwid) {
	if (hwid >= 0 && hwid < _hws) {
		if (hw[hwid].type == 3) {
			return hw[hwid].param1;
		}
	}else if(hwid >= _hws && hwid < _hws+_pxsegs) {
		return pxsegs[hwid].end - pxsegs[hwid].start;
	}
	return -1;
}

int TFLights::getPixelGroupCount(uint8_t hwid) {
	int c = 0;
	if (_pxsegs>0) {
		for (int i=0; i<_pxsegs;i++) {
			if (pxsegs[i].hwid == hwid) {
				c++;
			}
		}
	}
	return c;
}

int TFLights::getType(uint8_t hwid) {
	if (hwid >=0 && hwid < _hws) {
		return hw[hwid].type;
	}else if (hwid >= _hws && hwid < _hws+_pxsegs) {
		return 3;
	}
	return 0;
}

void TFLights::setStatus(uint8_t hwid, TFLightState s) {
	if (hwid >= 0 && hwid < _hws+_pxsegs) {
		status[hwid] = s;
	}
}

void TFLights::setStatus(uint8_t hwid, uint8_t mode, uint8_t type) {
	if (hwid >= 0 && hwid < _hws+_pxsegs) {
		setStatus(hwid, mode, type, 0, 0, 0, 0, {0,0,0});
	}
}


void TFLights::setStatus(uint8_t hwid, uint8_t mode, uint8_t type = 0, uint8_t p1 = 0, uint8_t p2 = 0, uint8_t p3 = 0, uint8_t p4 = 0,RGB color = {0,0,0}) {
	if (hwid >= 0 && hwid < _hws+_pxsegs) {
		status[hwid].mode = mode;
		status[hwid].type = type;
		status[hwid].param1 = p1;
		status[hwid].param2 = p2;
		status[hwid].param3 = p3;
		status[hwid].param4 = p4;
		status[hwid].color = color;
	}
}

void TFLights::setColor(RGB color, uint8_t hwid) {
	if (hwid >= 0 && hwid < _hws) {
		if (hw[hwid].type == 1 || hw[hwid].type == 2 || hw[hwid].type == 4) {
			status[hwid].color = color;
			if (PWMRANGE != 255) {
				int r = map(color.r, 0, 255, 0, PWMRANGE); 
				int g = map(color.g, 0, 255, 0, PWMRANGE); 
				int b = map(color.b, 0, 255, 0, PWMRANGE); 
				setRawColor(r,g,b,hwid);
			}else {
				setRawColor(color.r, color.g, color.b, hwid);
			}
		}else {
			status[hwid].color = color;
			setPixelsColor(color, hwid);
		}
	}else if (hwid >= _hws && hwid < _hws+_pxsegs) {
		status[hwid].color = color;
		setPixelsGroupColor(color, hwid - _hws);
	}
}

void TFLights::setRawColor(int r, int g, int b, uint8_t hwid) {
	if (hwid >= 0 && hwid < _hws) {
		if (hw[hwid].type == 1) {
			analogWrite(hw[hwid].pin1, r);
			analogWrite(hw[hwid].pin2, g);
			analogWrite(hw[hwid].pin3, b);
		}else if(hw[hwid].type == 2) {
			analogWrite(hw[hwid].pin1, r);
		}else if (hw[hwid].type == 3) {
			setPixelsColor(RGB{r,g,b}, hwid);
		}else if (hw[hwid].type == 4) {
			analogWrite(hw[hwid].pin1, r == PWMRANGE ? PWMRANGE : 0);
		}
	//pixelgroup
	}else if (hwid >= _hws && hwid < _hws+_pxsegs) {
		setPixelsGroupColor(RGB{r,g,b}, hwid - _hws);
	}
}

void TFLights::setBrightness(float p, uint8_t hwid) {
	if (hwid >= 0 && hwid < _hws) {
		if (hw[hwid].type == 1 || hw[hwid].type == 3) {
			 int r =  constrain(floor(status[hwid].color.r * p / 100),0,255);
             int g =  constrain(floor(status[hwid].color.g * p / 100),0,255);
             int b =  constrain(floor(status[hwid].color.b * p / 100),0,255);
             setRawColor(r,g,b,hwid);
		}
		else if(hw[hwid].type == 2) {
			int r = constrain(floor(255 * p/100),0,255);
			if (PWMRANGE != 255) { 
				r = map(r, 0, 255, 0, PWMRANGE); 
			}
			setRawColor(r,0,0,hwid);
		}
		else if(hw[hwid].type == 4) {
			setRawColor(p==100?PWMRANGE:0,0,0,hwid);
		}
	//pixelgroup
	}else if (hwid >= _hws && hwid < _hws+_pxsegs) {
		 int r =  constrain(floor(status[hwid].color.r * p / 100),0,255);
         int g =  constrain(floor(status[hwid].color.g * p / 100),0,255);
         int b =  constrain(floor(status[hwid].color.b * p / 100),0,255);
		 setPixelsGroupColor(RGB{r,g,b}, hwid - _hws);
	}
}

void TFLights::setPixelsColor(RGB color, uint8_t hwid) {
	if (hwid >= 0 && hwid < _hws) {
		if (hw[hwid].type == 3) {
			status[hwid].color = color;
			for (int i=0; i< hw[hwid].param1; i++) {
				pixels[hw[hwid].pin2].setPixelColor(hw[hwid].pin3 > 0 ? i+1 : i, color.r, color.g, color.b); 
			}
		}	
		dirty = true;
	}else if(hwid > _hws && hwid < _hws+_pxsegs) {
		setPixelsGroupColor(color, hwid-_hws);
	}
}

void TFLights::setPixelsColor(RGB color, uint8_t hwid, uint8_t groupid) {
	if (hwid >= 0 && hwid < _hws) {
		if (hw[hwid].type == 3) {
			status[_hws + groupid].color = color;
			setPixelsGroupColor(color,groupid);
		}
	}
}

void TFLights::setPixelsColor(RGB color, uint8_t hwid, String pixels, char *delimiter) {
	if (hwid >= 0 && hwid < _hws) {
		if (hw[hwid].type == 3) {
			char pxs[pixels.length()+1];
			pixels.toCharArray(pxs, pixels.length()+1);
			status[hwid].color = color;
			_setPixelHelper(color, hwid, pxs, delimiter);
		}
	}
}

void TFLights::_setPixelHelper(RGB color,  uint8_t hwid, char *pixchr, char *delimiter) {
	char* c;   
	char* tmp;
	char* _px_tmp;
	c = strtok_r(pixchr, delimiter, &_px_tmp);
	while (c != NULL) {
		if( strstr(c,"-") != NULL) {
			c = strtok_r(c,"-",&tmp);
			int from = atoi(c);
			c = strtok_r(NULL,"-",&tmp);
			int to = atoi(c);
			if (from < to && from >= 0) {
				for (int i=from;i<to;i++) {
					setPixelColor(color, i, hwid);
				}
			}
		}else {
			setPixelColor(color, atoi(c), hwid);
		}
		c = strtok_r(NULL, ",",  &_px_tmp);
	}
}

void TFLights::setPixelsGroupColor(RGB color, uint8_t groupid) {
	TFLightPixelGroup s = getPixelGroup(groupid);
	uint8_t hwid = s.hwid;
	for (int i = s.start; i <= s.end; i++) {
		pixels[hw[hwid].pin2].setPixelColor(hw[hwid].pin3 >0 ? i+1 : i, color.r, color.g, color.b); 
	}
	dirty = true;
}


void TFLights::setPixelColor(RGB color, int pixel, uint8_t hwid) {
	if (hwid >= 0 && hwid < _hws) {
		if (hw[hwid].type == 3) {
			pixels[hw[hwid].pin2].setPixelColor(hw[hwid].pin3 >0 ? pixel+1 : pixel, color.r, color.g, color.b); 
		}	
		dirty = true;
	}else if(hwid >= _hws && hwid < _hws+_pxsegs) {
		TFLightPixelGroup grp = getPixelGroup( hwid - _hws);
		pixels[hw[grp.hwid].pin2].setPixelColor(hw[grp.hwid].pin3 >0 ? grp.start + pixel+1 : grp.start + pixel, color.r, color.g, color.b); 
	}
}

RGB TFLights::getPixelColor(uint8_t hwid, int pixel) {
	if (hwid >= 0 && hwid < _hws) {
		if (hw[hwid].type == 3) {
			uint32_t color = pixels[hw[hwid].pin2].getPixelColor(hw[hwid].pin3 > 0 ? pixel + 1 : pixel);
			return RGB {(uint8_t)(color >> 16), (uint8_t)(color >>  8), (uint8_t)(color >>  0)};
		}
	}
	return RGB{0,0,0};
}

TFLightPixelGroup TFLights::getPixelGroup(uint8_t groupid) {
	if (groupid >= 0 && groupid < _pxsegs) {
		return pxsegs[groupid];
	}
	
	TFLightPixelGroup c = {};
	c.hwid = 0;
	c.start = 0;
	c.end = 0;
	return c;
}

TFLightPixelGroup TFLights::getPixelGroup(uint8_t hwid, uint8_t index) {
	uint8_t in = 0;
	if (index >= 0 && index < _pxsegs) {
		for (int i=0; i<_pxsegs;i++) {
			if (pxsegs[i].hwid == hwid) {	
				if (in == index) {
					return pxsegs[i];
				}
				in++;
			}
		}
	}
	
	TFLightPixelGroup c = {};
	c.hwid = 0;
	c.start = 0;
	c.end = 0;
	return c;
}

void TFLights::EQDisplay(TFEq *eq) {
	eq->setMSGEQ(false);
	if (eq->hasMSGEQ() && EQType == 1) {
		eq->setMSGEQ(true);
	}
	
	for (int h=0; h < _hws; h++) {
		if (status[h].mode == 3||status[h].mode == 4) {
			if (status[h].param1 >= 0 && status[h].param1 < eq->bars()) {
				//rgb
				if (hw[h].type == 1) {
					TFLights::setRGBEq(h, eq->get(status[h].param1));
				}
				//led
				else if (hw[h].type == 2) {
					TFLights::setRGBEq(h, eq->get(status[h].param1));
				}
				//pixel
				else if (hw[h].type == 3) {
					if (status[h].type >= 4) {
						TFLights::setPixelEq(h, eq->get(status[h].param1));
					}else {
						TFLights::setRGBEq(h, eq->get(status[h].param1));
					}
				}
			}
		}
	}
	
	int h=0;
	int hwid=0;
	
	for (int p=0; p < _pxsegs; p++) {
		h = _hws + p;
		hwid = pxsegs[p].hwid;
		if ((status[h].mode == 3 || status[h].mode == 4) && status[hwid].mode == 5) {
			if (status[h].param1 >= 0 && status[h].param1 < eq->bars()) {
				if (status[h].type >= 4) {
					TFLights::setPixelEq(h, eq->get(status[h].param1));
				}else {
					TFLights::setRGBEq(h, eq->get(status[h].param1));
				}
			}
		}
	}
}

void TFLights::setEQColors(RGB *colors, uint8_t size) {
	EQColors = colors;
	_eqcolors = size;
}

void TFLights::setAutoEQMode(uint8_t autocolor, uint8_t autotype) {
	_autoEqMode = (autotype * 10) + autocolor;
}

int TFLights::getAutoEQMode() {
	return _autoEqMode;
}

void TFLights::autoEQMode(TFEqData eq) {
	if (eq.beat == 255 && _autoEqMode > 0) {  
		if (_autoEqHelper >= _autoEqTreshold) {
			_autoEqHelper = 0;
		
			if (floor(_autoEqMode/10) == 1) {
				if (_autoEqRGBId < 3) { _autoEqRGBId++; }
				else { _autoEqRGBId = 1; } 
				if (_autoEqPixelId < 12) { _autoEqPixelId++; }
				else { _autoEqPixelId = 1; } 
				if (_autoEqLedId < 3) { _autoEqLedId++; }
				else { _autoEqLedId = 1; } 
			}
			else if (floor(_autoEqMode/10) == 2) {
				 _autoEqRGBId = random(1,4);
				 _autoEqLedId = random(1,4);
				 _autoEqPixelId = random(1,14);
			}
			
			for (int i=0; i<_hws;i++) {
				if (status[i].mode == 4 || status[i].mode == 5) {
					if (hw[i].type == 1) {
						status[i].type = _autoEqRGBId != 0 ? _autoEqRGBId : 1;
					}
					else if (hw[i].type == 2) {
						status[i].type = _autoEqLedId != 0 ? _autoEqLedId : 1;
					}
					else if(hw[i].type == 3) {
						if (hasPixelGroup(i)) {
							if (status[i].param4 == 1) {
								if (_autoEqPixelId == 12) {
									status[i].mode = 4;
								}
								else if (random(0,2) == 1) {
									status[i].mode = 5;
								}
								else if (random(0,2) == 1) {
									status[i].mode = 4;
								}
							}
							else if (status[i].param4 == 2) {
								if (_autoEqPixelId == 12 || _autoEqPixelId < 4) {
									status[i].mode = 4;
								}else {
									status[i].mode = 5;
								}
							}
							
							for (int g = 0; g < _pxsegs; g++) {
								if (pxsegs[g].hwid == i) {
									if (status[i].mode == 4) {
										status[_hws+g].mode = 0;	
									}else {
										status[_hws+g].mode = 4;
										status[_hws+g].type = _autoEqPixelId != 0 ? _autoEqPixelId : 1;	
									}
								}
							}
						}
						
						if (status[i].mode == 4) {
							status[i].type = _autoEqPixelId != 0 ? _autoEqPixelId : 1;
						}
					}
				}
			}
		}else {
			_autoEqHelper++;	
		}
		
		if (_autoEqCHelper >= _autoEqCTreshold) {
			_autoEqCHelper=0;
			if (_autoEqMode%10 == 1 || _autoEqMode%10 == 2) {
				if (_autoEqMode%10 == 1) {
					if (_autoColorId < _eqcolors-1) {
						_autoColorId++;
					}else { _autoColorId = 0; }
				}else {
					_autoColorId = random(0,_eqcolors);
				}
			}
		}else {
			_autoEqCHelper++;
		}
	}
}

RGB TFLights::getEQColor(uint8_t index, uint8_t sub) {
	int i = (TFLIGHT_EQCOLORPALETTE_SIZE*index) + sub;
	if (i >=0 && i < _eqcolors*TFLIGHT_EQCOLORPALETTE_SIZE) {
		return EQColors[i]; 	
	}
	return RGB{0,0,0};
}

void TFLights::setRGBEq(uint8_t h, TFEqData eq) {
	RGB color = {0,0,0};

	//auto color
	if (status[h].param2 >= _eqcolors) {
		color = getEQColor(_autoColorId, status[h].param3);
	//fixed color
	}else {
		color = getEQColor(status[h].param2, status[h].param3);
	}
	//off
	if(status[h].type == 0) {
		color.r = color.g = color.b = 0;
	}
	//blink beat
    else if(status[h].type == 1) {
		if (eq.beat != 255) {  
			color.r = color.g = color.b = 0;
		}
	}
	//blink_beat_rev
	else if(status[h].type == 2) {
		if (eq.beat > 250) {  
			color.r = color.g = color.b = 0;
		}
	}
	//by volume
	else if(status[h].type == 3) {
		color.r = floor((float)color.r * ((float)eq.value/255));
		color.g = floor((float)color.g * ((float)eq.value/255));
		color.b = floor((float)color.b * ((float)eq.value/255));
	}

	if (status[h].color.r != color.r || status[h].color.g != color.g || status[h].color.b != color.b) {
		status[h].color = color;
		setColor(color, h);
	}
}

void TFLights::setPixelEq(uint8_t hwid, TFEqData eq) {
	TFLightPixelGroup grp;
	
	if (hwid >= _hws) {
		grp = getPixelGroup(hwid-_hws);
	}else {
		grp.hwid  = hwid;
		grp.gid   = 0;
		grp.start = 0;
		grp.end   = hw[hwid].param1;
	}

	int displaytype = status[hwid].type - 4;
	
	if (grp.end > 0) {
	    if (displaytype >=0 && displaytype <= 3) {
			_pixelEqbar(grp, hwid, eq.value,  displaytype==1||displaytype==3 ? eq.max : 0, displaytype==2||displaytype==3 ? true :false);
		}
		else if (displaytype >=4 && displaytype <= 7) {
			TFLightPixelGroup grp1 = {};
			TFLightPixelGroup grp2 = {};
			grp1.hwid = grp.hwid;
			grp2.hwid = grp.hwid;
			
			int affpixels   = grp.end - grp.start;
			
			grp1.start  = grp.start;
			grp1.end    = grp.start+floor(affpixels/2);
			
			grp2.start  = grp1.end+1;
			grp2.end	= grp.end;
			
			if (displaytype == 4 || displaytype == 5) {
				_pixelEqbar(grp1, hwid, eq.value, displaytype==5 ? eq.max : 0, false);
				_pixelEqbar(grp2, hwid, eq.value, displaytype==5 ? eq.max : 0, true);
			}
			else if (displaytype == 6 || displaytype == 7) {
				_pixelEqbar(grp1, hwid, eq.value, displaytype==7 ?  eq.max : 0, true);
				_pixelEqbar(grp2, hwid, eq.value, displaytype==7 ?  eq.max : 0, false);
			}
		}
		else if (displaytype == 8) {
			RGB tmp; 
			int pxid;
			
			for (int i=grp.end-1; i>=grp.start; i--) {
				pxid = i;
				tmp = TFLights::getPixelColor(hwid,pxid);
				pxid += 1;
				if (pxid <= grp.end && pxid >= grp.start) {
					setPixelColor(tmp,  pxid, grp.hwid);
				}
			}
			
			if (eq.value>0) {
				if (status[hwid].param2>=_eqcolors) {
					tmp = getEQColor(_autoColorId,status[hwid].param3);
				}else {
					tmp = getEQColor(status[hwid].param2,status[hwid].param3);
				}
			
				if (eq.beat==255) {
					setPixelColor(tmp, grp.start, grp.hwid);
				}else {
					tmp.r = floor((float)tmp.r * ((float)eq.value/255));
					tmp.g = floor((float)tmp.g * ((float)eq.value/255));
					tmp.b = floor((float)tmp.b * ((float)eq.value/255));
					setPixelColor(tmp,  grp.start, grp.hwid);
				}
			}else {
				setPixelColor(RGB{0,0,0}, grp.start, grp.hwid);
			}
		}
	}
}

void TFLights::_pixelEqbar(TFLightPixelGroup grp, uint8_t stid, uint8_t value, uint8_t max, bool reverse) {
					
	int affpixels   = grp.end - grp.start;
	uint8_t pxval 	 = map(value, 0, 255, 0, affpixels);
	uint8_t pxmaxval = map(max, 0, 255, 0, affpixels);
	uint8_t p_change = (int)floor(affpixels/3);
	if (p_change==0) { p_change = 1; }
	int colorindex = 0;
	
	//auto color
	if (status[stid].param2>=_eqcolors) {
		colorindex = _autoColorId;
	//fixed color
	}else {
		colorindex = status[stid].param2;
	}
	
	int pxid=0;
	for (int i=grp.start; i<=grp.end; i++) {
		pxid = reverse ? grp.start + grp.end - i : i;
		//if (hw[grp.hwid].pin3 > 0) {	pxid+=1;	}
		
		if (value>0) {
			if ((i-grp.start) <= pxval) {
				setPixelColor(	getEQColor(colorindex, status[stid].param3 == 0 ? floor((i-grp.start)/p_change) : status[stid].param3), pxid, grp.hwid);
			}else {
				setPixelColor(RGB{0,0,0}, pxid, grp.hwid);
			}
		}else { 
			setPixelColor(RGB{0,0,0}, pxid, grp.hwid); 
		}
		
		if (max > 0) {
			if (pxmaxval > 0 && pxmaxval == (i-grp.start)) {
				setPixelColor(getEQColor(colorindex, TFLIGHT_EQCOLORPALETTE_SIZE-1), pxid, grp.hwid);
			}
		}
	}
}

void TFLights::update() {
	if (dirty) {
		for (int i=0; i< _hws; i++) {
			if (hw[i].type == 3) {  
				pixels[hw[i].pin2].show();
			}
		}
		dirty = false;
	}
}






