#include "tfheader.h"
#include "tflights.h"
#include "tflightanim.h"
#include "tflightprogram.h"

TFLightProgram::TFLightProgram() {}

bool TFLightProgram::load(uint8_t type, int program_id, TFLightAnim* a) {
	File f = SPIFFS.open(_getfolder(type)+String(program_id), "r");
	if (!f) { return false; }
	
	String line;
	char *seq;
	while ( (line = f.readStringUntil('\n')) != NULL) {
		seq = strtok(&line[0],"=");
		if (seq != NULL) {
			//name
			if (String(seq) == "N") { 
				seq = strtok(NULL,"=");
				a->setName(String(seq));
		
			//timestamp
			}else if (String(seq) == "T") { 
				seq = strtok(NULL,"=");
				/*if (seq != NULL) {
					a->setType(atoi(seq));
				}*/
			//color count
			}else if(String(seq) == "C") {
				seq = strtok(NULL,"=");
				if (seq != NULL) {
					a->setColors(atoi(seq));
				}
			//program type 	
			}else if(String(seq) == "P") {
				seq = strtok(NULL,"=");
				if (seq != NULL) {
					seq = strtok(seq,",");
					int type = atoi(seq);
					if (type == 1) {
						seq = strtok(NULL,",");
						if (!a->setType(type, atoi(seq), 0)) {
							return false;
						}
						a->setPid(program_id);
					}else {
						seq = strtok(seq,",");
						if (!a->setType(type, 0, atoi(seq))) {
							return false;
						}
						a->setPid(program_id);
					}	
				}
			//C[0], C[1] ... colors	
			}else if(String(seq[0]) == "C" && String(seq[1]).toInt() >= 0) {
				if (a->get_colors() > 0) {
					int index = String(seq[1]).toInt();
					if (a->get_colors() > index && index > 0) {
						RGB tmp;
						seq = strtok(NULL,"=");
						if (seq != NULL) {
							seq = strtok(seq,",");
							if (seq != NULL) { tmp.r = atoi(seq); }
							seq = strtok(NULL,",");
							if (seq != NULL) { tmp.g = atoi(seq); }
							seq = strtok(NULL,",");
							if (seq != NULL) { tmp.b = atoi(seq); }
							a->addColor(index,tmp);
						}
					}	
				}
			//P[0], P[1] .... program data	
			}else if(String(seq[0]) == "P" && String(seq[1]).toInt() >= 0) {
				
				int index = String(seq[1]).toInt();
				seq = strtok(NULL,"=");
				if (seq != NULL) {
					if (a->getType()==1) {
						if (index >= 0 && index < a->get_rgbs()) {
							TFLightAnimRGB tmp;
							seq = strtok(seq,";");
							if (seq != NULL) {  tmp.start_color = atoi(seq); }
							seq = strtok(NULL,";");
							if (seq != NULL) {  tmp.end_color = atoi(seq); }
							seq = strtok(NULL,";");
							if (seq != NULL) {  tmp.fade = atoi(seq); }
							seq = strtok(NULL,";");
							if (seq != NULL) {  tmp.hold = atoi(seq); }
							seq = strtok(NULL,";");
							if (seq != NULL) {  tmp.random = atoi(seq); }
							seq = strtok(NULL,";");
							if (seq != NULL) {  tmp.repeat = atoi(seq); }
							seq = strtok(NULL,";");
							if (seq != NULL) {  tmp.repeat_v = atoi(seq); }
							seq = strtok(NULL,";");
							if (seq != NULL) {  tmp.hold_p = atoi(seq); }
							a->addRGBProgram(index, tmp);	
						}
					}else {
						if (index >= 0 && index < a->get_pixels()) {
							TFLightAnimPixel tmp1;
							/*seq = strtok(seq,";");
							if (seq != NULL) {  tmp1.fstart = atoi(seq); }
							seq = strtok(NULL,";");
							if (seq != NULL) {  tmp1.fend = atoi(seq); }
							seq = strtok(NULL,";");
							if (seq != NULL) {  tmp1.move = atoi(seq); }
							seq = strtok(NULL,";");
							if (seq != NULL) {  tmp1.color1 = atoi(seq); }
							seq = strtok(NULL,";");
							if (seq != NULL) {  tmp1.color2 = atoi(seq); }
							seq = strtok(NULL,";");
							if (seq != NULL) {  tmp1.pixels = seq; }
							seq = strtok(NULL,";");
							if (seq != NULL) {  tmp1.param_move = atoi(seq); }
							seq = strtok(NULL,";");
							if (seq != NULL) {  tmp1.param_eff_1 = atoi(seq); }*/
							//a->addPixelProgram(index, tmp1);
						}
					}
				}
			}
		}
	}
}

bool TFLightProgram::save(TFLightAnim* a) {
	int prog_id = a->getPid();
	if (prog_id == -1) { 
		prog_id = _nextprogid(a->getType()); 
	}
	if (prog_id == -1) { return false; }
	

	String delimiter = ";";
	File f = SPIFFS.open(_getfolder(a->getType())+String(prog_id), "w");
	if (!f) { return false; }
	
	f.print("N="+a->getName()+"\n");
	f.print("T="+String(a->getType())+"\n");
	f.print("C="+String(a->get_colors())+"\n");
	
	for (int i=0; i<a->get_colors(); i++) {
		f.print("C"+String(i)+"="+String(a->getColor(i,0))+","+String(a->getColor(i,1))+","+String(a->getColor(i,2))+"\n");
	}
	
	String value;
	if (a->getType() == 1) { 
		f.print("P="+String(a->get_rgbs())+"\n");
		for (int i=0; i<a->get_rgbs(); i++) {
			/*value = String(rgb[i].start_color); value += delimiter;
			value += String(rgb[i].end_color);  value += delimiter;
			value += String(rgb[i].fade);  value += delimiter;
			value += String(rgb[i].hold);  value += delimiter;
			value += String(rgb[i].random);  value += delimiter;
			value += String(rgb[i].repeat);  value += delimiter;
			value += String(rgb[i].repeat_v);  value += delimiter;
			value += String(rgb[i].hold_p);/*
			f.print("P"+String(i)+"="+value+"\n");
		}
	}
	
	if (a->getType() == 2) { 
		f.print("P="+String(a->get_pixels())+"\n");
		for (int i=0; i<a->get_pixels(); i++) {
			
			value = String(pixel[i].fstart);  		value += delimiter;
			value += String(pixel[i].fend);  		value += delimiter;
			/*value += String(pixel[i].move); 		value += delimiter;
			value += String(pixel[i].color1);  		value += delimiter;
			value += String(pixel[i].color2);  		value += delimiter;
			value += pixel[i].pixels;		    	value += delimiter;
			value += String(pixel[i].param_move);   value += delimiter;
			value += String(pixel[i].param_eff_1);*/
			f.print("P"+String(i)+"="+value+"\n");
		}
	}
	
	f.close();
	return true;
}

int TFLightProgram::saveTo(uint8_t ty, int program_id, String data, bool append = false) {
	if (program_id == -1) {
		program_id = _nextprogid(ty);
	}
	if (program_id == -1) {
		return -1;
	}else {
		File f = SPIFFS.open(_getfolder(ty)+program_id, append?"a":"w");
		if (!f) { return false; }
		f.print(data);
		f.close();
		return program_id;
	}
}

bool TFLightProgram::remove(uint8_t type, int program_id) {
	return SPIFFS.remove(_getfolder(type)+String(program_id));
}

TFLightsProgList TFLightProgram::getList(uint8_t ty, int start){
	String line;
	char *seq;
	
	TFLightsProgList result;
	result.count = 0;
	TFLightsProgItem tmp;
	
	Dir dir = SPIFFS.openDir(_getfolder(ty));
	File f;
	uint8_t index = 0;
	int x;
	bool found = false;
	
	while (dir.next()) {
		x = _file2int(dir.fileName());
		if (x>=0&&x<TFLIGHTANIM_SAVED_PROG_COUNT && index >= start) {
			found = false;
			f = dir.openFile("r");
			while ((line = f.readStringUntil('\n')) != NULL) {
				seq = strtok(&line[0],"=");
				if (seq != NULL) {
					//name
					if (String(seq) == "N") { 
						seq = strtok(NULL,"=");
						String(seq).toCharArray(tmp.name,30);
						tmp.program_id = x;
					}
					else if (String(seq) == "P") { 
						seq = strtok(NULL,"=");
						if (seq != NULL) {
							seq = strtok(seq,",");
							tmp.program_type = atoi(seq);
						}
					}
					
					if (tmp.program_id >= 0 && tmp.program_type>0) {
						result.items[result.count] = tmp;
						result.count++;
						found = true;
						break;
					}
				}
			}
			f.close();
			
			if (!found) {
				#ifdef TF_DEBUG
				Serial.println("[TFLP] Removing "+ dir.fileName());
				#endif
				SPIFFS.remove(dir.fileName());
			}
			
			if (result.count >= TFLIGHTANIM_PROGLIST_LIMIT) {
				break;
			}
		}
		if (index >= start+TFLIGHTANIM_PROGLIST_LIMIT) {
			break;
		}
		index++;
	}
	return result;
}

int TFLightProgram::_nextprogid(uint8_t ty) {
	byte checked[TFLIGHTANIM_SAVED_PROG_COUNT];
	Dir dir = SPIFFS.openDir(_getfolder(ty));
	int x = 0;
	//collecting data from dir
	while (dir.next()) {
		x = _file2int(dir.fileName());
		if (x>=0&&x<TFLIGHTANIM_SAVED_PROG_COUNT) {
			checked[x] = 1;
		}
	} 
	//search for first available fileindex
	for (x=0;x<TFLIGHTANIM_SAVED_PROG_COUNT;x++) {
		if (checked[x]!=1) {
			return x;
		}
	}
	return -1;
}


String TFLightProgram::_getfolder(uint8_t type) {
	String folder = "/anim/";
	/*if (type==1) { folder += "rgbs/"; }
	if (type==2) { folder += "pixels/"; }*/
	return folder;
}

int TFLightProgram::_file2int(String f) {
	f.replace(_getfolder(1), "");
	//f.replace(_getfolder(2), "");
	return f.toInt();
}


