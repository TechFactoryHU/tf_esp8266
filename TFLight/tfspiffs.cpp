#include "Arduino.h"
#include "FS.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "tfheader.h"
#include "helpers.h"
#include "tfspiffs.h"

TFSPIFFS_HELPER::TFSPIFFS_HELPER() {

}

bool TFSPIFFS_HELPER::begin() {
	if (!SPIFFS.begin()){
		#ifdef TF_DEBUG
			log("Cant init SPI Flash FS!");
		#endif
		return false;
	}
	return true;
}


void TFSPIFFS_HELPER::log(String d) {
	#ifdef TF_DEBUG
	Serial.println("[TFSPIFFS] "+d);	
	#endif
}


String TFSPIFFS_HELPER::load(String url) {
	#ifdef TF_DEBUG
		log("Loading: "+url);
	#endif
	if (WiFi.status() == WL_CONNECTED) {
		HTTPClient http;
		url.trim();
		http.begin(url);
		int hc = http.GET();
		#ifdef TF_DEBUG
			log("Response code: "+String(hc));
		#endif
		
		if (hc == 200) {
			String response = http.getString();
			http.end();
			return response;
		}
	}
	#ifdef TF_DEBUG
		log("Error, cant load url");
	#endif
	return "";
}

bool TFSPIFFS_HELPER::downloadAll(String filelisturl, String localfolder, bool clear) {
	String filelist = load(filelisturl);
	#ifdef TF_DEBUG
		log("downloadAll response:");
		log(filelist);
	#endif
	if (filelist.length()>0) {
		char *seq = strtok(&filelist[0],"\n");
		char *tmp;
		char *r;
		TFSPIFFS_FILELIST tfile = {};
		
		if (dls>0) { free(dlfiles); dls=0; }
		//first line is filescount
		dls = atoi(seq); 
		if (dls > 0) {
			#ifdef TF_DEBUG
			log("Filelist loaded, found "+String(dls)+" files");
			#endif
			int numBytes = dls * sizeof(struct TFSPIFFS_FILELIST);
			if ((dlfiles = (TFSPIFFS_FILELIST *)malloc(numBytes)) == NULL) {
				#ifdef TF_DEBUG
				log("Malloc failed. Cant allocate = "+String(numBytes));
				#endif
				return false;
			}
			
			uint8_t index = 0;
			while((seq = strtok(NULL,"\n")) != NULL) {
				r = strtok_r(seq, ",", &tmp);
				if (r!=NULL) {
					String(r).toCharArray(tfile.url,100);
					#ifdef TF_DEBUG
					log(tfile.url);
					#endif
					r = strtok_r(NULL, ",", &tmp);
					if (r!=NULL) {
						String(localfolder+"/"+String(r)).toCharArray(tfile.localfile,50);
						r = strtok_r(NULL, ",", &tmp);
						if (r!=NULL) {	
							tfile.size = atoi(r);
							if (dls > index) { dlfiles[index] =  tfile; }
							index++;
						}
					}
				}
			}
			
			if (clear) { removeAll(localfolder); }
			dlstate = 1;
			
			return true;
		}else {
			#ifdef TF_DEBUG
			log("Wrong or empty filelist ..."+filelisturl);
			#endif
		}
	}
	return false;
}

void TFSPIFFS_HELPER::update() {
	if (dlstate > 0) {
		if (dlstate-1 < dls) {
			if (!download( String(dlfiles[dlstate-1].url), String(dlfiles[dlstate-1].localfile))) {
				#ifdef TF_DEBUG
				log("Download failed = "+String(dlfiles[dlstate-1].url)+" => "+String(dlfiles[dlstate-1].localfile));
				#endif
			}
			dlstate++;
		}else {
			free(dlfiles);
			dlstate = 0; dls = 0;
		}
	}
}

bool TFSPIFFS_HELPER::download(String url, String localfile) {
	if (WiFi.status() == WL_CONNECTED) {
		#ifdef TF_DEBUG
		log("Downloading "+url+" => "+localfile);
		#endif
		return write(localfile, load(url));
	}
	return false;
}

bool TFSPIFFS_HELPER::write(String file, String content) {
	File f = SPIFFS.open(file, "w");
	if (!f) { return false; }
	f.print(content);
	f.close();
	return true;
}

void TFSPIFFS_HELPER::list(String folder, void (*listHelper)(String, String)) {
	#ifdef TF_DEBUG
	log("List files from = "+folder);
	#endif
	Dir dir = SPIFFS.openDir(folder);
	while (dir.next()) {
		#ifdef TF_DEBUG
		log("Found = "+dir.fileName());
		#endif
	
		listHelper(folder, dir.fileName());
	}
}

void TFSPIFFS_HELPER::_dummyListHelper(String f, String p) {
	#ifdef TF_DEBUG
	log(p);
	#endif
}


bool TFSPIFFS_HELPER::removeAll(String folder) {
	#ifdef TF_DEBUG
	log("Removing all files from "+folder+" folder");
	#endif
	Dir dir = SPIFFS.openDir(folder);
	while (dir.next()) {
		SPIFFS.remove(dir.fileName());
		#ifdef TF_DEBUG
		log(dir.fileName()+" removed.");
		#endif
	}
	return true;
}

TFSPIFFS_HELPER TFSPIFFS;