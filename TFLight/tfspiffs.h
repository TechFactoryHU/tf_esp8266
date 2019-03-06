#ifndef TFSPIFFS_H
#define TFSPIFFS_H
#include "Arduino.h"
#include "FS.h"
#include "tfheader.h"
#include "helpers.h"

struct TFSPIFFS_FILELIST {
	char url[100];
	char localfile[100];
	int size;
};

class TFSPIFFS_HELPER {
	public:
		TFSPIFFS_HELPER();
		bool begin();
		String load(String url);
		bool write(String localfile, String content);
		bool download(String url, String localfile);
		bool downloadAll(String filelisturl, String localfolder, bool clear = false);
		bool removeAll(String folder);
		void update();
		
		void list(String p, void (*listHelper)(String, String));
		void _dummyListHelper(String, String);
		
	private:
		uint8_t dlstate = 0;
		uint8_t dls = 0;
		TFSPIFFS_FILELIST *dlfiles;
		#ifdef TF_DEBUG
		void log(String line);
		#endif
};


extern TFSPIFFS_HELPER TFSPIFFS;

#endif // TFSPIFFS_H