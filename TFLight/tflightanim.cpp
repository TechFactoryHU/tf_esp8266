#include "tfheader.h"
#include "tflightanim.h"
#include "tflights.h"

#ifndef TFLIGHT_FRAME_TIME
#define TFLIGHT_FRAME_TIME 30
#endif

TFLightAnim::TFLightAnim() { _hws = 5; program_id = -1; type = 0;}

TFLightAnim::~TFLightAnim() {
	free(hw);
	free(rgb);
	free(pixel);
}

void TFLightAnim::setTFLights(TFLights *l) {
	lights = l;
}

bool TFLightAnim::setName(String name) {
	name.toCharArray(prog_name,30);
	return true;
}

bool TFLightAnim::save() {
	return false;
}

bool TFLightAnim::remove() {
	return false;
}

void TFLightAnim::reset() {
	type = 0;
	if (_pixels>0) { _allocPixel(0); }
	if (_rgbs>0) { _allocRGB(0); }
	if (_anims>0) { _allocAnims(0); }
	if (_colors>0) { _allocColor(0); }
	_allocPixelStatus(0);
	
	clearHws();
	program_id = -1;
	reset_on_end = false;
}

bool TFLightAnim::findHw(uint8_t hwid) {
	for (int i=0; i<_hws; i++) {
		if (hw[i] == hwid+1) { return true; }
	}
	return false;
}

int TFLightAnim::getHw(uint8_t index) {
	if (index >= 0 && index < _hws) {
		return hw[index]>0 ? hw[index]-1 : -1;
	}
	return -1;
}

bool TFLightAnim::addHw(uint8_t hwid) {
	for (int i=0; i<_hws; i++) {
		if (hw[i] == 0) { 
			if (type == 2 && lights->getType(hwid) != 3) {
				return false;
			}
			else if (type == 2)  {
				if (lights->getPixelCount(hwid) > _maxpxcount) {
					_allocPixelStatus(lights->getPixelCount(hwid));
				}
			}else if (type != 2) {
				_allocPixelStatus(0);
			}
			
			if (lights->getType(hwid) == 3) {
				if (lights->isPixelGroup(hwid)) {
					int nhwid = lights->getHwIdByGroup(hwid);
					lights->status[nhwid].mode = 5;
					lights->setColor(RGB{0,0,0}, nhwid);
				}
			}
			
			hw[i] = hwid+1; 
			return true; 
		}
	}
	return false;
}

bool TFLightAnim::remHw(uint8_t hwid) {
	for (int i=0; i<_hws; i++) {
		if (hw[i] == hwid+1) { hw[i] = 0; return true; }
	}
	return false;
}

void TFLightAnim::clearHws() {
	for (int i=0; i<_hws; i++) { 
		hw[i] = 0; 
	}
}

bool TFLightAnim::resetHws() {
	for (int i=0; i<_hws; i++) { 
		if (hw[i]>0) {
			int hwid = hw[i]-1;
			lights->setColor(RGB{0,0,0}, hwid);
			lights->status[hwid].mode = 0;
		}
	}
	return true;
}

int TFLightAnim::hwCount() {
	int c = 0;
	for (int i=0; i<_hws; i++) {
		if (hw[i]>0) { c++; }
	}
	return c;
}

bool TFLightAnim::setColors(uint8_t c) {
	return _allocColor(c); 
}

//t = type, c = count, a = animation count  
bool TFLightAnim::setType(uint8_t t, uint8_t c, uint8_t a) {
	program_id = -1;
	//release all 
	if (!_allocPixel(0)) { 
		#ifdef TF_DEBUG 
			Serial.println("Cant alloc pixels = 0");
		#endif
		return false;
	};
	if (!_allocAnims(0)) {  
		#ifdef TF_DEBUG
			Serial.println("Cant alloc anims = 0");
		#endif
		return false;
	};
	if (!_allocRGB(0)) {  return false; };
	
	type = t;
	if (type == 1) {
		if (!_allocRGB(c)) {
			Serial.println("Cant alloc rgbs = "+String(c));
			return false;
		}
		return true;
	}else if(type == 2) {
		if (!_allocAnims(a)) {
			#ifdef TF_DEBUG
				Serial.println("Cant alloc anims = "+String(a));
			#endif
			return false;
		}
		return _allocPixel(c);
	}
	return false;
}

bool TFLightAnim::addColor(int index, RGB c) {
	if (_colors > 0 && index >= 0 && index < _colors) {
		color[index] = c;
		return true;
	}
	return false;
}

bool TFLightAnim::addRGBProgram(int index, TFLightAnimRGB prog) {
	if (index < _rgbs) {
		rgb[index] = prog;
		return true;
	}
	return false;
}

bool TFLightAnim::addPixelProgram(int index, TFLightAnimPixel prog) {
	if (index < _pixels) {
		bool b[8];
		b[0] = true;
		b[1] = false;
		b[2] = false;
		b[3] = true;
		b[4] = false;
		b[5] = false;
		b[6] = false;
		b[7] = false;
		
		prog.params = _setParams(b);

		//pixel[index] = prog;
		return true;
	}
	return false;
}

bool TFLightAnim::addPixelAnimation(int index, TFLightAnimPixelA a) {
	if (index < _anims) {
		bool b[8];
		b[0] = true;
		b[1] = false;
		b[2] = false;
		b[3] = false;
		b[4] = true;
		b[5] = false;
		b[6] = false;
		b[7] = false;
		
		a.params = _setParams(b);
		anim[index] = a;
		return true;
	}
	return false;
}

void TFLightAnim::update(unsigned long millis){
	if (!startTime||millis<startTime) { startTime = millis; }
	frame = floor((millis - startTime)/TFLIGHT_FRAME_TIME); 
	
	if (type==1) {
		updateRGBs();
	}else if (type == 2) {
		updatePixels();
	}
}

void TFLightAnim::_pix_fadeColor(RGB *o, RGB a, RGB b, float p) {
	o->r = _fadeColorHelper(a.r, b.r, p);
	o->g = _fadeColorHelper(a.g, b.g, p);
	o->b = _fadeColorHelper(a.b, b.b, p);
}

TFLightAnimPixelA TFLightAnim::getAnimation(uint8_t index) {
	if (index >= 0 && index < _anims) {
		return anim[index];
	}
	return TFLightAnimPixelA{};
}

void TFLightAnim::setAnimation(uint8_t index, TFLightAnimPixelA a) {
	if (index >= 0 && index < _anims) {
		anim[index] = a;
	}
}

void TFLightAnim::updatePixels() {
	int p,repeat,led,lastpx,pixelFrame,checkFrame=0;
	float ps = 0;
	
	bool brk;						
	RGB color = RGB{0,0,0};
	bool animParams[8] 	= {false,false,false,false,false,false,false,false};
	bool pixelParams[8]  = {false,false,false,false,false,false,false,false};
	bool pxChanged = false;
	int path_anim  = 0; 
	uint8_t path_anim_lifetime = 10;
	TFLightPixelRange limit;
	
	if (frame != _lastframe) {
		if (frame <= anim_length) {
			for (int i=0; i<_pixels; i++) {
				if (pixel[i].fstart <= frame && pixel[i].fend >= frame) {
					path_anim = 0;
					limit.from = limit.to = 0;
					pxChanged = false;
					if (pixel[i].frames==0) { pixel[i].frames = pixel[i].fend - pixel[i].fstart; }
					pixelFrame = frame-pixel[i].fstart;	
					limit.from = 0; limit.to = 0; limit.is_percentage = false;	
					//checkFrame = pixelFrame % pixel[i].frames;
					//float pixelProgress = ((float)(pixelFrame)/(float)( pixel[i].fend - pixel[i].fstart));	

					_getParams(pixel[i].params, pixelParams);
					
					if (pixelFrame == 0) {
						if (pixelParams[0]) {
							_resetPixelsStatus();
							pxChanged = true;
						}if (pixelParams[3]) {
							pixel[i].position=0;
							pxChanged = true;
						}
					}
					//if (frame == 0) { pixel[i].position = 0; } 
							
					if ((TMPPixelCount == 0 && TMPPixelRangesCount == 0) || TMPPixelLastId != i) {					
						//parse pixel-ids to array
						_pixelHelperString(pixel[i].pixels);
						TMPPixelLastId = i;
						if (TMPPixelCount == 0 && TMPPixelRangesCount == 0) { continue; }
					}
					
					//Animations
					for (int a=0; a<5; a++) {
						if (pixel[i].anim[a]>0) {
							brk = false;
							TFLightAnimPixelA anim = getAnimation(pixel[i].anim[a]-1);
							_getParams(anim.params, animParams);
							
							if (animParams[3]) { //loop
								repeat = (int)floor(pixelFrame/anim.fend);
							}else { repeat = 0; }
							
							if ((!animParams[3] && anim.fstart <= pixelFrame && anim.fend >= pixelFrame) ||
								(animParams[3] && (repeat*anim.fend)+anim.fstart <= pixelFrame && (repeat+1)*anim.fend >= pixelFrame)) {
								
								checkFrame = pixelFrame - ((repeat*anim.fend)+anim.fstart);
								//anim %
								ps = ((float)(pixelFrame - ((repeat*anim.fend)+anim.fstart))/(float)(anim.fend-anim.fstart));	
								
								if (checkFrame == 0 && animParams[0]) {
									_resetPixelsStatus();
									pxChanged = true;
									//lights->setPixelsColor(RGB{0,0,0},hw[h]-1);
								}
							
								//Serial.println("Anim:"+String(anim.action)+"@"+String(checkFrame)+" : "+String(p));
								//actions
								//turn off
								if (anim.action == 0) {
									color = RGB{0,0,0};
									pxChanged = true;
								}
								else if(anim.action == 1) {
									color = getColor(anim.paramb1);
								}
								//blink
								else if(anim.action == 2) {
									if (ps>0.5) { color = getColor(anim.paramb2); pxChanged = true; }
									else { color = getColor(anim.paramb1); pxChanged = true; }
								}
								//fade
								else if(anim.action == 3) {
									_pix_fadeColor(&color, getColor(anim.paramb1), getColor(anim.paramb2), ps);
									pxChanged = true;
								}
								//rainbow color
								else if(anim.action == 4) {
									if (anim.parami1>0) {
										repeat = (int)floor(checkFrame/anim.parami1);
										if (repeat != anim.parami2) {
											if (anim.paramb1 == 255) { anim.paramb1 = 0; }
											else { anim.paramb1++; }
											color = colorWheel(anim.paramb1);
											anim.parami2 = repeat;
											setAnimation(pixel[i].anim[a]-1, anim);
										}
									}
								}
								
								//move
								else if(anim.action == 20) {
									repeat = (int)floor(checkFrame/anim.parami1);
									if (repeat != anim.parami2) {
										//no path animation
										if (!animParams[4]) {
											//turn off all pixels
											_resetPixelsStatus();
										}
										if (anim.paramb1>0) { 
											pixel[i].position += anim.paramb1; 
											if (pixel[i].position > _maxpxcount) {
												pixel[i].position = 0;
											}
											//path animation
											if (animParams[4]) {
												path_anim = anim.paramb1;
												path_anim_lifetime = anim.paramb3?anim.paramb3:10;
											}
										}
										else if (anim.paramb2>0) { 
											pixel[i].position -= anim.paramb2; 
											if (pixel[i].position < 0) {
												pixel[i].position = _maxpxcount;
											}
											//path animation
											if (animParams[4]) {
												path_anim = -anim.paramb2;
												path_anim_lifetime = anim.paramb3?anim.paramb3:10;
											}
										}
										anim.parami2 = repeat;
										//set animation
										setAnimation(pixel[i].anim[a]-1, anim);

										for (p=0; p<_maxpxcount; p++) {
											if (_getParam(pxstatus[p].params,0)) {
												pxstatus[p].params = 0;
												pxstatus[p].color = RGB{0,0,0};
											}
										}
										
										pxChanged = true;	
									}
								}
								//limit (for move & repeat)
								else if(anim.action == 21) {
									//limit by percentage
									if (anim.paramb1==1) {
										limit.from = floor((float)anim.paramb2/100 * _maxpxcount);
										limit.to   = floor((float)anim.paramb3/100 * _maxpxcount);
									}
									else {
										limit.from  = anim.parami1;
										limit.to 	= anim.parami2;
									}
								}
							}//anim active
						}
					}//anim loop end
					
					if (pxChanged) {
						lastpx = 0;
						repeat = 0;
						//pixels loop (eg. 0-100)
						for (p=0; p<_maxpxcount; p++) {
							//reset pixels
							/*if(path_anim!=0) {
								if (_getParam(pxstatus[p].params,0)) {
									pxstatus[p].params = 1;
									pxstatus[p].color = RGB{0,0,0};
								}
							}*/
						
							//affected pixels loop (eg. 0,1,2,6,8 ...) 
							
							for (int px=0;px<TMPPixelCount;px++) {
								if (TMPPixels[px] == p-(lastpx*repeat)) {
									led = p+pixel[i].position;
									if (!pixelParams[1]) {
										if (led < 0 || led > _maxpxcount) {
											break; 
										} 
									}else {
										if (led < 0 || led > _maxpxcount) {
											break; 
											led = led < 0 ? _maxpxcount+led : led-_maxpxcount;
										}
									}
									
									if (limit.from||limit.to) {
										if (limit.from > led || limit.to < led) {
											break;
										}
									}
									
									pxstatus[led].color = color;
									pxstatus[led].params = 1;
									//lights->setPixelColor(color, led, hw[h]-1);
									if (px+1 != TMPPixelCount) { break; }
								}
								else if (TMPPixels[px] > p-(lastpx*repeat)) { break; }
								
								if (px+1 == TMPPixelCount) {
									lastpx = TMPPixels[px] + (pixelParams[2] ? 0 : 1);
									//repeat pattern
									if (!pixelParams[1]) {
										brk = true;
										break;
									}else {
										repeat++;
									}
								}
							}
							
							//pixel ranges
							if (!pixelParams[1]) {
								led = p + pixel[i].position;
								for (int r=0;r<TMPPixelRangesCount;r++) {
									if (
										(TMPPixelRanges[r].is_percentage && round(TMPPixelRanges[r].from/100*_maxpxcount)<=led||led<=round(TMPPixelRanges[r].to/100*_maxpxcount))
										|| (TMPPixelRanges[r].from<=led||led<=TMPPixelRanges[r].to)
									) {
										if (limit.from||limit.to) {
											if (limit.from > led || limit.to < led) {
												continue;
											}
										}
										if (led < 0 || led > _maxpxcount) {
											break; 
										}
										pxstatus[led].color = color;
										pxstatus[led].params = 1;
									}
								}
							}
							
							if (brk) { break; }
						}
						
						//path anim
						if (path_anim!=0) {
							for (int p=0; p<_maxpxcount; p++) {
								if (_getParam(pxstatus[p].params,0) && (pxstatus[p].color.r > 0 || pxstatus[p].color.g > 0 || pxstatus[p].color.b > 0)) {
									int l = p;
									while(true) {
										if (path_anim>0){l--;}else {l++;}
										if (l<0||l>=_maxpxcount) { break; }
										if (p-path_anim > l || p+path_anim < l) { break; }
										if (!_getParam(pxstatus[l].params,0) && !_getParam(pxstatus[l].params,1)) {
											pxstatus[l].color = pxstatus[p].color;
											pxstatus[l].params = _setParam(1, true, 0);
											pxstatus[l].lifetime = path_anim_lifetime;
										}else {
											break;
										}
									}
								}else if(!_getParam(pxstatus[p].params,0)&&!_getParam(pxstatus[p].params,1)) {
									pxstatus[p].color = RGB{0,0,0};
									pxstatus[p].params = 4;
								}
							}
						}
					
						//show pixels
						for (int a=0; a<_maxpxcount; a++) {
							//_getParams(pxstatus[a].params, animParams);
							//set
							if (_getParam(pxstatus[a].params,0)||_getParam(pxstatus[a].params,2)) {
								for (int h=0; h<_hws; h++) {
									if (hw[h]>0) {
										lights->setPixelColor(pxstatus[a].color, a, hw[h]-1);
									}
								}
								pxstatus[a].params = 0;
							}
							//fade out
							else if (_getParam(pxstatus[a].params,1)) {
								if (pxstatus[a].color.r > 0 || pxstatus[a].color.g > 0 || pxstatus[a].color.b > 0) {
									pxstatus[a].color.r = constrain(pxstatus[a].color.r-pxstatus[a].lifetime, 0, 255);
									pxstatus[a].color.g = constrain(pxstatus[a].color.g-pxstatus[a].lifetime, 0, 255);
									pxstatus[a].color.b = constrain(pxstatus[a].color.b-pxstatus[a].lifetime, 0, 255);
									for (int h=0; h<_hws; h++) {
										if (hw[h]>0) {
											lights->setPixelColor(pxstatus[a].color, a, hw[h]-1);
										}
									}
								}else {
									pxstatus[a].params = 0;
								}
							}
						}
					}
					//if (pixel[i].position != 0 && pixel[i].position % lastpx == 0) { pixel[i].position = 0; }
				}
			}
			
		}else {
			startTime = 0;
		}
		_lastframe = frame;
	}
}

void TFLightAnim::_resetPixelsStatus() {
	_setPixelsStatus(RGB{0,0,0}, 4);
}

void TFLightAnim::_setPixelsStatus(RGB color, unsigned char c) {
	for (int i=0; i<_maxpxcount; i++) {
		pxstatus[i] = TFLightAnimPixelS{color,c};
	}
}

unsigned char TFLightAnim::_setParams(bool b[8]) {
	unsigned char c = 0;
	for (int i=0; i< 8; i++) {
		if (b[i] == true) { c |= 1 << i; }
	}
	return c;
}

unsigned char TFLightAnim::_setParam(int index, bool value, unsigned char r) {
	unsigned char c = 0;
	for (int i=0; i< 8; i++) {
		if ((index == i && value) || (index != i && ((r & (1<<i)) != 0 ))) {
			c |= 1 << i;
		}
	}
	return c;
}

bool TFLightAnim::_getParam(unsigned char c, int i) {
	return (c & (1<<i)) != 0;
}

void TFLightAnim::_getParams(unsigned char c, bool *r) {
	for (int i=0; i< 8; i++) {
		if ((c & (1<<i)) != 0) { r[i] = true; }else { r[i] = false; }
	}
}

void TFLightAnim::updateRGBs() {
	if (frame != _lastframe) {
		if (aindex > -1 && aindex < _rgbs) {
			if (rgb[aindex].random != 0 && frame == 0) {
				if (random(0,rgb[aindex].random+1) != 1) {
					nextRGBProgram();
					return;
				}
			}
			int anim_frames = rgb[aindex].fade;

			if (rgb[aindex].hold_p == 1 || rgb[aindex].hold_p == 3) { anim_frames += rgb[aindex].hold; }
			else if(rgb[aindex].hold_p == 2) { anim_frames += rgb[aindex].hold*2; }		
			
			if (frame <= anim_frames) {
				if (frame == 0) {
					//set color
					setDevColor(getColor(rgb[aindex].start_color));
				}
				float p = 0;
				if (rgb[aindex].hold>0 ) {
					if (rgb[aindex].hold_p == 1) {
						if (frame > rgb[aindex].hold) {
							p = ((float)(frame-rgb[aindex].hold)/(float)rgb[aindex].fade);	
						}
					}else if(rgb[aindex].hold_p == 2) {
						if (frame > rgb[aindex].hold && frame < rgb[aindex].hold+rgb[aindex].fade) {
							p = ((float)(frame-rgb[aindex].hold)/(float)rgb[aindex].fade);	
						}
					}
					else if(rgb[aindex].hold_p == 3) {
						if (frame <= rgb[aindex].fade) {
							p = ((float)frame/(float)rgb[aindex].fade);	
						}
					}
				}else {
					p = ((float)frame/(float)rgb[aindex].fade);	
				}
				
				if (p>0) {
					fadeColor(getColor(rgb[aindex].start_color),getColor(rgb[aindex].end_color), p);	
				}
			}
			
			if (frame >= anim_frames) {
				//next frame
				nextRGBProgram();
			}
		}
		_lastframe = frame;
	}
}


void TFLightAnim::setDevColor(RGB c) {
	int r,g,b = 0;

	if (PWMRANGE != 255) {
		r = map(c.r, 0, 255, 0, PWMRANGE); 
		g = map(c.g, 0, 255, 0, PWMRANGE); 
		b = map(c.b, 0, 255, 0, PWMRANGE); 
	}else { r = c.r; g = c.g; b = c.b; }
	
	for (int i=0; i<_hws; i++) {
		if (hw[i]>0) {
			if (lights->getType(hw[i]-1) == 3) {
				lights->setPixelsColor(c, hw[i]-1);
			}else {
				lights->setRawColor(r, g, b, hw[i]-1);
			}
		}
	}
}


RGB TFLightAnim::colorWheel(uint8_t pos) {
	pos = 255-pos;
	
	if (pos < 85) {
		return RGB{255 - pos * 3,0,pos*3};
	}
	else if(pos < 170) {
		return RGB{0,pos * 3,255 - pos * 3};
	}
	else  {
		pos -= 170;
		return RGB{pos * 3, 255 - pos * 3, 0};
	}
}

void TFLightAnim::fadeColor(RGB a, RGB b, float p) {
	RGB c;
	c.r = _fadeColorHelper(a.r, b.r, p);
	c.g = _fadeColorHelper(a.g, b.g, p);
	c.b = _fadeColorHelper(a.b, b.b, p);
	setDevColor(c);
}

int TFLightAnim::_fadeColorHelper(int a, int b, float p) {
   int diff = a - b;
   return diff != 0 ? constrain( ceil(a - (diff * p)), 0, 255) : a;
}

void TFLightAnim::nextRGBProgram() {
	if (aindex > -1) {
		if (rgb[aindex].repeat != 0) {
			if (rgb[aindex].repeat_v > 0) {
				rgb[aindex].repeat_v--;
				startTime = 0;
				return;
			}else if(rgb[aindex].repeat_v == 0) {
				rgb[aindex].repeat_v = -1;
			}
			else {
				rgb[aindex].repeat_v = rgb[aindex].repeat>0? rgb[aindex].repeat : random(1,abs(rgb[aindex].repeat));
				startTime = 0;
				return;
			}
		}else { rgb[aindex].repeat_v = -1; }
	}
	
	if (aindex+1 < _rgbs) { aindex++; }
	else { 
		if (reset_on_end) {
			reset();
		}else {
			aindex = 0; 
		}
	}
	startTime = 0;
}

RGB TFLightAnim::getColor(uint8_t id) {
	if (id > 0 && id < _colors) {
		return color[id];
	}
	return RGB{0,0,0};
}

bool TFLightAnim::_allocColor(uint8_t num) {
	if (num == 0) {
		free(color);
		_colors = 0;
	}else {
		int numBytes = num * sizeof(struct RGB);
		if (_colors > 0) {
			if ((color = (RGB *)realloc(rgb, numBytes))) {
			  _colors = num;
			}else {
			  _colors = 0;
			   return false;
			}
		}else {
			if ((color = (RGB *)malloc(numBytes))) {
			  _colors = num;
			}else {
			  _colors = 0;
			   return false;
			}
		}
	}
	return true;
}

bool TFLightAnim::_allocRGB(uint8_t num) {
	if (num == 0) {
		if (_rgbs > 0) {
			free(rgb);
			_rgbs = 0;
		}
	}else {
		int numBytes = num * sizeof(struct TFLightAnimRGB);
		if (_rgbs > 0) {
			if ((rgb = (TFLightAnimRGB *)realloc(rgb, numBytes))) {
			  _rgbs = num;
			}else {
			  _rgbs = 0;
			   return false;
			}
		}else {
			if ((rgb = (TFLightAnimRGB *)malloc(numBytes))) {
			  _rgbs = num;
			}else {
			  _rgbs = 0;
			   return false;
			}
		}
	}
	return true;
}

bool TFLightAnim::_allocPixel(uint8_t num) {
	if (num == 0) {
		if (_pixels > 0) {
			free(pixel);
			_pixels = 0;
		}
	}else {
		int numBytes = num * sizeof(struct TFLightAnimPixel);
		if (_pixels > 0) {
			if ((pixel = (TFLightAnimPixel *)realloc(pixel, numBytes))) {
			  _pixels = num;
			}else {
			  _pixels = 0;
			   return false;
			}
		}else {
			if ((pixel = (TFLightAnimPixel *)malloc(numBytes))) {
			  _pixels = num;
			}else {
			  _pixels = 0;
			   return false;
			}
		}
	}
	return true;
}

bool TFLightAnim::_allocAnims(uint8_t num) {
	if (num == 0) {
		if (_anims > 0) {
			free(anim);
			_anims = 0;
		}
	}else {
		int numBytes = num * sizeof(struct TFLightAnimPixelA);
		if (_anims > 0) {
			if ((anim = (TFLightAnimPixelA *)realloc(pixel, numBytes))) {
			  _anims = num;
			}else {
			  _anims = 0;
			   return false;
			}
		}else {
			if ((anim = (TFLightAnimPixelA *)malloc(numBytes))) {
			  _anims = num;
			}else {
			  _anims = 0;
			   return false;
			}
		}
	}
	return true;
}


void TFLightAnim::_setTMPPixel(int id) {
    if (TMPPixelCount >= 0 && TMPPixelCount < TFLIGHTANIM_MAX_PIXEL_COUNT) {
        TMPPixels[TMPPixelCount] = id;  
        TMPPixelCount++;     
    }
}

int TFLightAnim::_pixelHelperString(String pixels) {
	char pxs[pixels.length()+1];
	pixels.toCharArray(pxs, pixels.length()+1);
	
	return _pixelHelper(pxs);
}

int TFLightAnim::_pixelHelper(char* pixchr) {
	char* c;   
	char* tmp;
	char* _px_tmp;
	TMPPixelRangesCount = TMPPixelCount = 0;
	
	memset(TMPPixels, 0, sizeof(TMPPixels));
	//memset(TMPPixelRanges, 0, sizeof(TMPPixelRanges));

	/*if( strstr(c,"%") != NULL) {
		c = strtok_r(c,"%",&tmp);
		c = strtok_r(c,"x",&tmp);
		TMPPixelPFrom = atoi(c);
		c = strtok_r(NULL,"x",&tmp);
		TMPPixelPLen = atoi(c);
		if (TMPPixelPFrom>(100/TMPPixelPLen)) {
			TMPPixelPFrom = TMPPixelPLen = 0;
		}
	}else {*/
		c = strtok_r(pixchr, ",", &_px_tmp);
		while (c != NULL) {
			if( strstr(c,"-") != NULL) {
				c = strtok_r(c,"-",&tmp);
				if (strstr(c,"%") != NULL ) {
					TMPPixelRanges[TMPPixelRangesCount].from = atoi(c);
					c = strtok_r(NULL,"-",&tmp);
					TMPPixelRanges[TMPPixelRangesCount].to = atoi(c);
					TMPPixelRanges[TMPPixelRangesCount].is_percentage = true;
					
				}else {
					TMPPixelRanges[TMPPixelRangesCount].from = atoi(c);
					c = strtok_r(NULL,"-",&tmp);
					TMPPixelRanges[TMPPixelRangesCount].to = atoi(c);
					TMPPixelRanges[TMPPixelRangesCount].is_percentage = false;
					
					/*
					int from = atoi(c);
					c = strtok_r(NULL,"-",&tmp);
					int to = atoi(c);
					if (from < to && from >= 0) {
						for (int i=from;i<to;i++) {
							_setTMPPixel(i);
						}
					}*/
				}
				TMPPixelRangesCount++;
			}else {
				_setTMPPixel(atoi(c));
			}
			c = strtok_r(NULL, ",",  &_px_tmp);
		}
	/*}*/
	return TMPPixelCount;
}

bool TFLightAnim::_allocPixelStatus(uint8_t num) {
	if (num == 0) {
		if (_maxpxcount > 0) {
			free(pxstatus);
			_maxpxcount = 0;
		}
	}else {
		int numBytes = num * sizeof(struct TFLightAnimPixelA);
		if (_maxpxcount > 0) {
			if ((pxstatus = (TFLightAnimPixelS *)realloc(pixel, numBytes))) {
			  _maxpxcount = num;
			}else {
			  _maxpxcount = 0;
			   return false;
			}
		}else {
			if ((pxstatus = (TFLightAnimPixelS *)malloc(numBytes))) {
			  _maxpxcount = num;
			}else {
			  _maxpxcount = 0;
			   return false;
			}
		}
	}
	return true;
}