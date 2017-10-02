#include "Arduino.h"
#include "FS.h"
#include "tfcustomcfg.h"
#include "../tfheader.h"

TFCustomCfg::TFCustomCfg(String filename) {
	_filename 	= filename;
}

TFCustomCfg::TFCustomCfg(String filename, unsigned int items, struct TFConfigItem* dataref) {
	_filename = filename;
	_items = items;
	_data = dataref;
}

TFCustomCfg::TFCustomCfg(String filename, unsigned int items) {
	_filename 	= filename;
	_items 		= items;
	updateSize();
}

void TFCustomCfg::begin() {
	init();
}

void TFCustomCfg::updateSize() {
	int numBytes = _items * sizeof(struct TFConfigItem);
	if ((_data = (TFConfigItem *)malloc(numBytes))) {
	}else {
		_items = 0;
	}
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
			#ifdef TF_DEBUG
				log("Parsing: "+line);
			#endif
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
			#ifdef TF_DEBUG
				log("Writing: "+String(_data[i].key)+"="+String(_data[i].value));
			#endif
			f.print(String(_data[i].key)+"="+String(_data[i].value)+"\n");
		}
		f.close();
		return true;
	}
	return false;
}

bool TFCustomCfg::add(TFConfigItem item) {
	int index = 0;
	for (int i=0; i<_items; i++) {
		if (strcmp(_data[i].key, item.key)==0) {
			#ifdef TF_DEBUG
				log("Updating : "+String(item.key)+" to "+String(item.value));
			#endif
			_data[i] = item;
			return true;
		}
	}
	
	for (int i=0; i<_items; i++) {
		if ((_data[i].key != NULL) && _data[i].key[0] == '\0') {
			#ifdef TF_DEBUG
				log("Add new entry: "+String(item.key)+" = "+String(item.value));
			#endif
			_data[i] = item;
			return true;
		}
	}
	#ifdef TF_DEBUG
		log("Add failed for: "+String(item.key)+" = "+String(item.value));
	#endif
	return false;
}

bool TFCustomCfg::add(String key, String value) {
	TFConfigItem tmp;
	key.toCharArray(tmp.key, TFCONFIG_ITEM_KLEN);
	value.toCharArray(tmp.value,TFCONFIG_ITEM_VLEN);
	return add(tmp);
}

bool TFCustomCfg::exists(char* key) {
	for (int i=0; i<_items; i++) {
		if (strcmp(_data[i].key,key)==0) {
			return true;
		}
	}
	return false;
}

/*
TFConfigItem TFCustomCfg::get(String key) {
	for (int i=0; i<_items; i++) {
		if (String(_data[i].key) == key) {
			return _data[i];
		}
	}
	return TFConfigItem{};
}
*/
TFConfigItem TFCustomCfg::get(char* key) {
	for (int i=0; i<_items; i++) {
		if (strcmp(_data[i].key,key)==0) {
			return _data[i];
		}
	}
	return TFConfigItem{};
}
/*
char* TFCustomCfg::getValue(String key) {
	for (int i=0; i<_items; i++) {
		if (String(_data[i].key) == key) {
			return _data[i].value;
		}
	}
	return 0;
}
*/

char* TFCustomCfg::getValue(char* key) {
	for (int i=0; i<_items; i++) {
		if (strcmp(_data[i].key,key)==0) {
			return _data[i].value;
		}
	}
	return "";
}

/*
int TFCustomCfg::getInt(String key) {
	char *s = getValue(key);
	if (s[0] == 0) { return -1; }
	return atoi(s);
}
*/

int TFCustomCfg::getInt(char* key) {
	return atoi(getValue(key));
}
/*
String TFCustomCfg::getString(String key) {
	return String(getValue(key));
}*/

String TFCustomCfg::getString(char* key) {
	return String(getValue(key));
}
/*
bool TFCustomCfg::getBool(String key) {
	char *s = getValue(key);
	if (s[0] == 0 || atoi(s) == 0) { return false; }
	return true;
}
*/
bool TFCustomCfg::getBool(char* key) {
	char *s = getValue(key);
	if (s == NULL || atoi(s) == 0) { return false; }
	return true;
}


