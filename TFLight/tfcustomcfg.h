#ifndef TFCUSTOMCFG_H
#define TFCUSTOMCFG_H
#include "Arduino.h"
#include "FS.h"
#include "helpers.h"
#include "tfheader.h"

struct TFConfigItem {
	char key[TFCONFIG_ITEM_KLEN];
	char value[TFCONFIG_ITEM_VLEN];
};

class TFCustomCfg {
    private:
		String _filename;
		int _items;
		TFConfigItem *_data;
		bool ref;
		
		File openFile(char* type);
		void log(String msg);
		
		String removeSpecialChars(String in);

	public:
		TFCustomCfg(String filename, int items);
		TFCustomCfg(String filename, int items, struct TFConfigItem *dataref);
		
		void begin();
		bool init(void);
		bool updateSize(int c);
		
		bool load(void);
		bool save(void);
		bool exists(String key);
		bool exists(char* key);
		bool remove(void);
		
		TFConfigItem get(char* key);
		char* getValue(char* key);
		int getInt(char* key);
		String getString(String key);
		String getString(char* key);
		bool getBool(char* key);
		RGB getRGB(char* key);
		RGB* getRGBs(char* key, uint8_t count);
		RGB* getRGBs(String str, uint8_t count);
		
		bool add(TFConfigItem item);
		bool add(String key, String value);
		bool add(String key, RGB color);
		bool add(String key, int value);
};

#endif // TFCUSTOMCFG_H

