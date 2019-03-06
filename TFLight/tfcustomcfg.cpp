#include "Arduino.h"
#include "FS.h"
#include "tfheader.h"
#include "tfcustomcfg.h"

TFCustomCfg::TFCustomCfg(String filename, int items) {
	_filename 	= filename;
	ref 	    = false;
	updateSize(items);
}

TFCustomCfg::TFCustomCfg(String filename, int items, struct TFConfigItem* dataref) {
	_filename = filename;
	_items = items;
	_data = dataref;
	ref = true;
}

void TFCustomCfg::begin() {
	init();
}

bool TFCustomCfg::updateSize(int c) {
	if (ref) { return false; }
	int numBytes = c * sizeof(struct TFConfigItem);
	if (_items == 0) {
		if ((_data = (TFConfigItem *)malloc(numBytes))) {
			_items = c;
			return true;
		}else {
			_items = 0;
		}
	}else {
		if ((_data = (TFConfigItem *)realloc(_data, numBytes))) {
			_items = c;
			return true;
		}else {
			_items = 0;
		}	
	}
	return false;
}

File TFCustomCfg::openFile(char* type) {
	File f = SPIFFS.open("/"+_filename, type);
	if (!f) {
		#ifdef TF_DEBUG 
			log("Cant open "+_filename+" file for "+String(type));
		#endif
	}
	return f;
}

void TFCustomCfg::log(String d) {
	#ifdef TF_DEBUG
		Serial.println("[Cfg] "+d);
	#endif	
};

bool TFCustomCfg::init() {
	if (!SPIFFS.begin()){
		#ifdef TF_DEBUG
			log("Cant init SPI Flash FS!");
		#endif
		return false;
	}
	return true;
}

bool TFCustomCfg::load() {
	#ifdef TF_DEBUG
		log("Loading "+_filename);
	#endif
	File f = openFile("r");
	if (f) {
		String line;
		char *seq;
		TFConfigItem tmp;
		while ( (line = f.readStringUntil('\n')) != NULL) {
			seq = strtok(&line[0],"=");
			if (seq != NULL) {
				String(seq).toCharArray(tmp.key, TFCONFIG_ITEM_KLEN);
				seq = strtok(NULL,"=");
				if (seq != NULL) {
					String(seq).toCharArray(tmp.value,TFCONFIG_ITEM_VLEN);
				}
				add(tmp);
			}
		}
		return true;
	}
	return false;
}

bool TFCustomCfg::save() {
	if (_items > 0) {
		File f = openFile("w");
		if (!f) {return false;}
		for (int i=0; i<_items;i++) {
			f.print(String(_data[i].key)+"="+String(_data[i].value)+"\n");
		}
		f.close();
		return true;
	}
	return false;
}


bool TFCustomCfg::remove() {
	return SPIFFS.remove("/"+_filename);
}

bool TFCustomCfg::add(TFConfigItem item) {
	int index = 0;
	for (int i=0; i<_items; i++) {
		if (strcmp(_data[i].key, item.key)==0) {
			_data[i] = item;
			return true;
		}
	}
	for (int i=0; i<_items; i++) {
		if ((_data[i].key != NULL) && _data[i].key[0] == '\0') {
			_data[i] = item;
			return true;
		}
	}
	#ifdef TF_DEBUG
		log("Out of space, cant add: "+String(item.key)+" = "+String(item.value));
	#endif
	return false;
}

bool TFCustomCfg::add(String key, String value) {
	TFConfigItem tmp;
	key.toCharArray(tmp.key, TFCONFIG_ITEM_KLEN);
	value = removeSpecialChars(value);
	value.toCharArray(tmp.value,TFCONFIG_ITEM_VLEN);
	return add(tmp);
}

bool TFCustomCfg::add(String key, RGB c) {
	return add(key ,String(c.r)+","+String(c.g)+","+String(c.b));
}

bool TFCustomCfg::add(String key, int value) {
	TFConfigItem tmp;
	key.toCharArray(tmp.key, TFCONFIG_ITEM_KLEN);
	String(value).toCharArray(tmp.value,TFCONFIG_ITEM_VLEN);
	return add(tmp);
}


String TFCustomCfg::removeSpecialChars(String in) {
    in.replace("=","");
    in.replace("\n","");
    in.replace("\r","");
    return in;
}

bool TFCustomCfg::exists(String key) {
	return exists(&key[0]);
}

bool TFCustomCfg::exists(char* key) {
	for (int i=0; i<_items; i++) {
		if (strcmp(_data[i].key,key)==0) {
			return true;
		}
	}
	return false;
}

TFConfigItem TFCustomCfg::get(char* key) {
	for (int i=0; i<_items; i++) {
		if (strcmp(_data[i].key,key)==0) {
			return _data[i];
		}
	}
	return TFConfigItem{};
}

char* TFCustomCfg::getValue(char* key) {
	for (int i=0; i<_items; i++) {
		if (strcmp(_data[i].key,key)==0) {
			return _data[i].value;
		}
	}
	return "";
}

int TFCustomCfg::getInt(char* key) {
	return atoi(getValue(key));
}

String TFCustomCfg::getString(String key) {
	return getString(&key[0]);
}

String TFCustomCfg::getString(char* key) {
	return String(getValue(key));
}

bool TFCustomCfg::getBool(char* key) {
	char *s = getValue(key);
	if (s == NULL || atoi(s) == 0) { return false; }
	return true;
}

RGB TFCustomCfg::getRGB(char* key) {
	char *s = getValue(key);
	RGB c = {0,0,0};
	s = strtok(s,",");
	if (s!=NULL) { c.r = atoi(s); }
	s = strtok(NULL,",");
	if (s!=NULL) { c.g = atoi(s); }
	s = strtok(NULL,",");
	if (s!=NULL) { c.b = atoi(s); }
	return c;
}

RGB* TFCustomCfg::getRGBs(char* key, uint8_t count) {
	char *s = getValue(key);
	char *r;
	char *tmp;
	RGB *c = new RGB[count];
	s = strtok(s,";");
	int index = 0;
	while(s != NULL && index < count) {
		c[index] = {0,0,0};
		r = strtok_r(s, ",", &tmp);
		if (r!=NULL) { c[index].r = atoi(r); }
		r = strtok_r(NULL, ",", &tmp);
		if (r!=NULL) { c[index].g = atoi(r); }
		r = strtok_r(NULL, ",", &tmp);
		if (r!=NULL) { c[index].b = atoi(r); }
		
		s = strtok(NULL,";");
		index++;
	}
	
	return c;
}

RGB* TFCustomCfg::getRGBs(String str, uint8_t count) {
	return getRGBs(&str[0], count);
}


