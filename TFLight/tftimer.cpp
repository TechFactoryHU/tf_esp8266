#include "FS.h"
#include <esp8266wifi.h>
#include <esp8266httpclient.h>
#include "tfheader.h"
#include "helpers.h"
#include "tfcustomcfg.h"
#include "tftimer.h"

TFTimer::TFTimer(TFCustomCfg *config) {
	_cfg = config;
	enabled = false;
}

void TFTimer::jobHandler(void (*callback)(TFTIMER_ITEM)) {
	jobHandlerCallback = callback;
}

void TFTimer::start() {
	if (_cfg->exists("NTS") && _cfg->exists("TIM")) {
		if (_cfg->getBool("TIM")) {
			#ifdef TFTIMER_TIMERS
			enabled = true;
			#endif
		}
	}
}

void TFTimer::stop() {
	enabled = false;
}

bool TFTimer::load() {
	#ifdef TFTIMER_TIMERS
	File f = SPIFFS.open(F("timers.dat"), "r");
	if (!f) { return false; }
	
	String line;
	char *seq;
	int index = 0;
	
	while ( (line = f.readStringUntil('\n')) != NULL) {
		seq = strtok(&line[0],"=");
		if (seq != NULL) {
			if(String(seq) == "T") {
				if (index < TFTIMER_TIMERS) {
					seq = strtok(NULL,"=");
					if (seq != NULL) {
						seq = strtok(seq,",");
						if (seq != NULL) { timers[index].d = atoi(seq); }else { continue; }
						
						seq = strtok(NULL,",");
						if (seq != NULL) { timers[index].h = atoi(seq); }else { continue; }
						
						seq = strtok(NULL,",");
						if (seq != NULL) { timers[index].m = atoi(seq); }else { continue; }
						
						seq = strtok(NULL,",");
						if (seq != NULL) { timers[index].action = atoi(seq); }else { continue; }
						
						seq = strtok(NULL,",");
						if (seq != NULL) { timers[index].action_value = atoi(seq); }else { continue; }
						
						seq = strtok(NULL,",");
						if (seq != NULL) { timers[index].color.r = atoi(seq); }
						seq = strtok(NULL,",");
						if (seq != NULL) { timers[index].color.g = atoi(seq); }
						seq = strtok(NULL,",");
						if (seq != NULL) { timers[index].color.b = atoi(seq); }
						
						seq = strtok(NULL,",");
						if (seq != NULL) { 
							String(seq).toCharArray(timers[index].hwids,10);
						}
						
						index++;
					}else { break; }
				}else { break; }
			}
		}
	}
	
	f.close();
	#endif
}

bool TFTimer::set(int index, TFTIMER_ITEM item) {
	if (index >= 0 && index <TFTIMER_TIMERS) {
		timers[index] = item;
		return true;
	}
	return false;
}

bool TFTimer::save() {
	File f = SPIFFS.open(F("timers.dat"), "w");
	if (!f) { return false; }
	for (int i=0; i<TFTIMER_TIMERS; i++) {
		f.print("T="+String(timers[i].d));
		f.print(","+String(timers[i].h)+","+String(timers[i].m)+","+String(timers[i].action));
		f.print(","+String(timers[i].action_value)+","+String(timers[i].color.r)+","+String(timers[i].color.g)+","+String(timers[i].color.b));
		f.print(","+String(timers[i].hwids));
		f.print("\n");
	}
	f.close();
	return true;
}

TFTIME TFTimer::getTime() {
	TFTIME currTime;
	if (lastrun > 0 && timestamp > 0) {
		unsigned long currts = (timestamp * 1000) + (millis()-lastupdate); 
		unsigned long r = 0;
		int currday  = (int) floor( currts / 86400000);
		r = currts % 86400000;
		int currhour = (int)floor(r / 3600000);
		r -= currhour*3600000;
		int currmin  = (int)floor (r / 60000);
		r -= currmin*60000;
		int currsec  =  (int)floor(r/1000);
		currTime = {d:(byte)currday, h:(byte)currhour, m:(byte)currmin, s:(byte)currsec, ts:currts};
	}
	return currTime;
}

void TFTimer::sleep(int intv) {
	if (enabled) {
		lastrun = millis() + intv;
	}
}

void TFTimer::update() {
	if (enabled) {
		updateTime(false);
		if (lastrun + 20000 > millis()) {
			return;
		}
		
		lastrun = millis();
		TFTIME ctime = getTime();
		
		if (ctime.ts>0) {
			if (ltime.ts > 0 && ltime.h==ctime.h && ltime.m == ctime.m) { return; }
			
			for (int i = 0; i < TFTIMER_TIMERS; i++) {
				if (timers[i].d > 0) {
					if (timers[i].d == 10 || timers[i].d == ctime.d || (timers[i].d == 8 && ctime.d > 0 && ctime.d < 6) || (timers[i].d == 9 && ctime.d > 5 && ctime.d < 8) || (timers[i].d > 10 && (ctime.d % (timers[i].d-10)) == 0)) {
						if (timers[i].h == ctime.h || (timers[i].h > 100 && (ctime.h % (timers[i].h-100)) == 0)) {
							 if (timers[i].m == ctime.m || (timers[i].m > 100 && (ctime.m % (timers[i].m-100)) == 0)) {
								
								if (jobHandlerCallback != NULL) {
									jobHandlerCallback(timers[i]);
								}
							 }
						}
					}
				}
			}
			
			ltime = ctime;
		}
	}
}

bool TFTimer::updateTime(bool forced = false) {
	if (enabled) {
	
		if (millis() < lastupdate) {
			lastupdate = 0;
		}
		
		if (WiFi.status() == WL_CONNECTED) {
			if (forced || (lasterror > 0 && lasterror < millis()) || lastupdate == 0 || lastupdate + TFTIMER_UPD_INTERVAL < millis()) {
				#ifdef TF_DEBUG 
					Serial.println("[TIME] Updating time from server");
				#endif
				HTTPClient http;
				String url = _cfg->getString("NTS");
				url.trim();
				http.begin(url);
				int httpCode = http.GET();
				
				if(httpCode > 0) {
					String payload = http.getString();
					timestamp = payload.toInt();
					
					#ifdef TF_DEBUG 
						 Serial.println("[TIME] Timestamp is : "+String(timestamp));
					#endif
					lastupdate = millis();
					lasterror = 0;
					http.end();
					
					return true;
					
				}else {
					#ifdef TF_DEBUG 
						 Serial.println("[TIME] Cant update time from "+_cfg->getString("NTS"));
						 Serial.print("[TIME] HTTP code = ");
						 Serial.print(httpCode);
						 Serial.println("");
					#endif
					
					lasterror = millis() + 60000;
					lastupdate = millis();
				}
				
				http.end();
			}
		}
	}
	return false;
}

