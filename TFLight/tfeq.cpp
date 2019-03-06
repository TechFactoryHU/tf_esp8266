#include "Arduino.h"
#include "tfeq.h"
TFEq::TFEq(uint8_t bars) : _framesize(0), _frames(NULL), _bdata(NULL), _bars(0), msgeq7(false) {
	int numBytes = bars * sizeof(struct TFEqData);
	if ((_bdata = (TFEqData *)malloc(numBytes))) {
      _bars = bars;
      for (int i=0; i<bars; i++) {
		_bdata[i].beatTime = 200; //ms
		_bdata[i].lastbeat = 0; //ms
      }
	}else {
      _bars = 0;
	}
}

void TFEq::setup() {
	#ifdef TFEQ_MSGEQ7_STROBE && TFEQ_MSGEQ7_RESET
		pinMode(TFEQ_MSGEQ7_STROBE, OUTPUT);
		pinMode(TFEQ_MSGEQ7_RESET, OUTPUT);
		digitalWrite(TFEQ_MSGEQ7_RESET, LOW);
		digitalWrite(TFEQ_MSGEQ7_STROBE, HIGH);
	#endif
}

void TFEq::setAnalogMap(int min, int max) {
	if (min >= 0) { _analogMapMin = min; }
	if (max >= 0) { _analogMapMax = max; }
}

void TFEq::paramsFromString(String p) {
	if (p == NULL) { return; }
	char *tmp;
	char *s = strtok_r(&p[0], ",", &tmp);
	uint8_t index = 0;
	
	while ((s = strtok_r(NULL,",", &tmp)) != NULL) {
		if (index == 0) {
			_analogMapMin = atoi(s);
		}
		else if (index == 1) {
			_analogMapMax = atoi(s);
		}
		else if (index == 2) {
			dropTime = atoi(s);
		}
		else if (index == 3) {
			dropVMax = atoi(s);
		}
		else if (index == 4) {
			dropVLevel = atoi(s);
		}
		index++;
	}
}

String TFEq::paramsToString(void) {
	String s = String(_analogMapMin)+",";
	s += String(_analogMapMax)+",";
	s += String(dropTime)+",";
	s += String(dropVMax)+",";
	s += String(dropVLevel);
	return s;
}

void TFEq::initBeatDetection(int framesize) {
	if (_bars > 0) {
		int numBytes = _bars * framesize * sizeof(uint8_t);
		if ((_frames = (uint8_t *)malloc(numBytes))) {
		  _framesize = framesize;
		}else {
		  _framesize = 0;
		}
	}
}

void TFEq::data(uint8_t bar, uint8_t value) {
	if (bar >= 0 && bar < _bars) {
		_bdata[bar].value = value;
		
		if (_bdata[bar].value>50) {
			if (_bdata[bar].lastbeat + _bdata[bar].beatTime < millis() ) { 
				if (_bdata[bar].max < value) {
					_bdata[bar].beat = 255;
					_bdata[bar].lastbeat = millis();
				}
			}else {
				if (_bdata[bar].max < value && (value - _bdata[bar].max) > 150) { _bdata[bar].beat = 255; _bdata[bar].lastbeat = millis(); }
			}
		}
		
		if (_bdata[bar].max < value) {
			_bdata[bar].max = value;
		}
		
		
		/*if (_framesize>0) {
			this->shiftFrames(bar);
			this->addFrame(bar, value);
		}*/
	}
}

uint8_t TFEq::beat() {
	for (int i=0; i<_bars;i++) {
		if (_bdata[i].beat > 0) {
			return (uint8_t)round(_bdata[i].beat/255*100);
		}
	}
	return 0;
}

uint8_t TFEq::beat(uint8_t bar) {
	if (bar >= 0 && bar < _bars) {
		if (_bdata[bar].beat > 0) {
			return (uint8_t)round(_bdata[bar].beat/255*100);
		}
	}
	return 0;
}

uint8_t TFEq::getBeat(uint8_t bar) {
	if (bar >= 0 && bar < _bars) {
		return _bdata[bar].beat;
	}
}

void TFEq::addFrame(uint8_t bar, uint8_t value) {
	int frameid = (bar-1) * _framesize;
	_frames[frameid] = value;
}

void TFEq::shiftFrames(uint8_t bar) {
	int to = bar*_framesize;
	for (int i = to-1; i > (bar-1)*_framesize; i--) {
		_frames[i] = _frames[i-1];
	}
}

int TFEq::average(uint8_t bar) {
	if (bar >= 0 && bar < _bars && _framesize > 0) {
		int avg = 0;
		for (int i= (bar-1)*_framesize; i< bar*_framesize; i++) {
			avg += _frames[i];
		}
		avg = (int)floor(avg/_framesize);
		return avg;
	}
	return 0;
}

bool TFEq::hasMSGEQ() {
	#ifdef TFEQ_MSGEQ7_STROBE && TFEQ_MSGEQ7_RESET
	return true;
	#else
	return false;
	#endif
}

void TFEq::setMSGEQ(bool status) {
	msgeq7 = status;
}

void TFEq::readMSGEQ() {
	#ifdef TFEQ_MSGEQ7_STROBE && TFEQ_MSGEQ7_RESET
	digitalWrite(TFEQ_MSGEQ7_STROBE, LOW);
	digitalWrite(TFEQ_MSGEQ7_RESET, HIGH);
	delayMicroseconds(30);
	digitalWrite(TFEQ_MSGEQ7_RESET, LOW);
	int aread;
	uint8_t data[7];
	uint8_t x = ceil(7/_bars);
	for (int i=0; i<7; i++) {
		digitalWrite(TFEQ_MSGEQ7_STROBE, HIGH);
		digitalWrite(TFEQ_MSGEQ7_STROBE, LOW);
		delayMicroseconds(36);
		aread = map( analogRead(A0), _analogMapMin, _analogMapMax, 0, 255);
		data[i] = aread < 0 ? 0 : aread;
		delay(0);    
	}
	TFEq::data(0,data[0]);
	TFEq::data(1,data[3]);
	TFEq::data(2,data[6]);
	#endif
}

void TFEq::update(unsigned long ts) {
	#ifdef TFEQ_MSGEQ7_STROBE && TFEQ_MSGEQ7_RESET
	if (msgeq7) {
		if (_eqts+eqInterval < ts) {
			TFEq::readMSGEQ();
			_eqts = ts;
		}
	}
	#endif
	
	if ((dropTime > 0 && _ts+dropTime < ts) ||
		(beatDropTime > 0 && _beatts+beatDropTime < ts)
	) { 
		for (int i=0; i<_bars;i++) {
			if (dropTime > 0 && _ts+dropTime < ts) {
				if (_bdata[i].max > 0) {
					_bdata[i].max = constrain(_bdata[i].max-dropVMax, 0, 255);
					if (_bdata[i].max == 0) { _bdata[i].max = 255; }
				}
				
				if (_bdata[i].value > 0) {
					_bdata[i].value = constrain(_bdata[i].value-dropVLevel, 0, 255);
				}
				
				if (_bdata[i].beat > 0) {
					if (_bdata[i].lastbeat + beatDropTime < ts) {
						_bdata[i].beat = constrain(_bdata[i].beat-dropVBeat, 0, 255);
					}
				}
				
			_ts = ts;
			}
			
			/*if (_bdata[i].lastbeat + _bdata[i].beatTime > ts) {
				if (_bdata[i].beat > 0) {
					_bdata[i].beat = constrain(_bdata[i].beat-dropVBeat, 0, 255);
				}
			}*/
			
			/*if (beatDropTime > 0 && _beatts+beatDropTime < ts) {
				if (_bdata[i].beat > 0) {
					_bdata[i].beat = constrain(_bdata[i].beat-dropVBeat, 0, 255);
				}
				_beatts = ts;
			}*/
		}
		delay(0);
	}
}

TFEqData TFEq::get(uint8_t bar) {
	if (bar >= 0 && bar < _bars) {
		return _bdata[bar];
	}
	return TFEqData{0,0,0};
}

void TFEq::setDrop(int time, uint8_t max, uint8_t level) {
	dropTime = time;
	dropVMax = max;
	dropVLevel = level;
}