#ifndef TFCUSTOMCFG_H
#define TFCUSTOMCFG_H
#include "Arduino.h"
#include "FS.h"
#include "../tfheader.h"

struct TFConfigItem {
	char key[TFCONFIG_ITEM_KLEN];
	char value[TFCONFIG_ITEM_VLEN];
};

class TFCustomCfg {
    private:
		String _filename;
		int _items;
		TFConfigItem *_data;
		
		File openFile(char* type);
		void updateSize(void);
		void log(String msg);

	public:
		TFCustomCfg(String filename);
		TFCustomCfg(String filename, unsigned int items, struct TFConfigItem *dataref);
		TFCustomCfg(String filename, unsigned int items);
		
		void begin();
		bool init(void);
		
		bool load(void);
		bool save(void);
		bool exists(char* key);
		
		//TFConfigItem get(String key);
		TFConfigItem get(char* key);
		
		//char* getValue(String key);
		char* getValue(char* key);
		
		//int getInt(String key);
		int getInt(char* key);
		//String getString(String key);
		String getString(char* key);
		//bool getBool(String key);
		bool getBool(char* key);
		
		bool add(TFConfigItem item);
		bool add(String key, String value);
};

#endif // TFCUSTOMCFG_H

