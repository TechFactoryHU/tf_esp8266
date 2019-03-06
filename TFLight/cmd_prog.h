/*
*	Light control related commands
*/
if (Network.isCommand("+ANIM-INFO")) {
	return "+ok=+ANIM-INFO:{\"h\": "+String(LightAnim[0].get_hws())+", \"s\":" + TFLIGHT_ANIM_MAX_SLOT + "}";
}
else if (Network.isCommand("+ANIM-STATUS")) {
	int slot_id;
	String json;
	if (pcount==0) {
	   slot_id = -1;
	}else {
	   slot_id = Network.getParam(0);
	}
	//i=slot_id, t=slot_type, a=program_id, 
	if (slot_id == -1) { json = "["; }
	for (int i = 0; i < TFLIGHT_ANIM_MAX_SLOT; i++) {
		if (slot_id == -1 && i>0) {
			json += ",";
		}
		if (i == slot_id || slot_id == -1) {
			json += "{\"i\":"+String(i)+",\"t\":"+ String(LightAnim[i].getType());
			json += ",\"a\":"+String(LightAnim[i].getPid())+",\"h\":[";
			int h = 0;
			for (int x = 0; x < LightAnim[i].get_hws(); x++) {
				int hwid = LightAnim[i].getHw(x);
				if (hwid > -1) {
					if (h>0) { json += ","; }
					json += hwid;
					h++;
				}
			}
			json += "]}";
		}
	}
	if (slot_id == -1) { json += "]"; }
	return "+ok=+ANIM-STATUS:"+json;
}
else if (Network.isCommand("+ANIM-LOAD")) {
	if (pcount >= 3) {
		//slot_id, anim_id, [hw-ids, min 1.]
		int slot_id 		= Network.getParam(0);
		if (slot_id >= 0 && slot_id < TFLIGHT_ANIM_MAX_SLOT) {
			int anim_id 		= Network.getParam(1);
			
			//if (LightAnim[slot_id].getPid() == anim_id) {
			LightAnim[slot_id].reset();
			if (!LightProg.load(0, anim_id, &LightAnim[slot_id])) {
				return "-err=+ANIM-LOAD";
			}
			//}

			for (int i = 2; i < pcount; i++) {
				int hwid = Network.getParam(i);
				if (hwid >= 0 && Lights.getHwCount(true) > hwid) {
					if (!LightAnim[slot_id].findHw(hwid)) {
						//RGB (all HW  type enabled)
						if (LightAnim[slot_id].getType() == 1) {
							Serial.println("RGB type");
							LightAnim[slot_id].addHw(hwid);
						//PiXL-s (only pixels enabled)
						}else if(LightAnim[slot_id].getType() == 2) {
							if (Lights.getType(hwid)==3) {
								LightAnim[slot_id].addHw(hwid);
							}
						}
						
						Lights.status[hwid].mode 	= 2;
						Lights.status[hwid].type 	= anim_id;
						Lights.status[hwid].param1  = slot_id;
					}
				}
			}
			
			return "+ok=+ANIM-LOAD:"+String(slot_id);
		}
	}
	return "-err=+ANIM-LOAD";
}
else if (Network.isCommand("+ANIM-STOP")) {
	int slot_id 		= Network.getParam(0);
	if (slot_id >= 0 && slot_id < TFLIGHT_ANIM_MAX_SLOT) {
		if (LightAnim[slot_id].getType()>0) {
			LightAnim[slot_id].resetHws();
		}
		LightAnim[slot_id].reset();
		return "+ok=+ANIM-STOP:"+String(slot_id);
	}
	return "-err=+ANIM-STOP";
}
else if (Network.isCommand("+ANIM-SETHW")) {
   int slot_id = Network.getParam(0);
   if (slot_id >= 0 && slot_id < TFLIGHT_ANIM_MAX_SLOT) {
	   LightAnim[slot_id].clearHws();
	   for (int i=1;i<pcount;i++) {
		  LightAnim[slot_id].addHw(Network.getParam(i));
	   }
	   return "+ok=+ANIM-SETHW";
   }
   return "-err=+ANIM-SETHW";
}
else if (Network.isCommand("+ANIM-SETUP")) {
   int slot_id = Network.getParam(0);
   if (slot_id >= 0 && slot_id < TFLIGHT_ANIM_MAX_SLOT) {
	   LightAnim[slot_id].setType( Network.getParam(1),  Network.getParam(2));
	   LightAnim[slot_id].setColors( Network.getParam(3));
	   for (int i=1;i<pcount;i++) {
		  LightAnim[slot_id].addHw(Network.getParam(i));
	   }
	   return "+ok=+ANIM-SETUP:"+String(slot_id);
   }
   return "-err=+ANIM-SETUP:"+String(slot_id);
}
else if (Network.isCommand("+ANIM-CLEAR")) {
  int slot_id = Network.getParam(0);
  if (slot_id >= 0 && slot_id < TFLIGHT_ANIM_MAX_SLOT) {
	   LightAnim[slot_id].reset();
  }
  return "+ok=+ANIM-CLEAR:"+String(slot_id);
}
else if (Network.isCommand("+ANIM-SETCOLOR")) {
  int slot_id = Network.getParam(0);
  if (slot_id >= 0 && slot_id < TFLIGHT_ANIM_MAX_SLOT) {
	  int color_count = Network.getParam(1);
	  int offset = Network.getParam(2);
	  for (int i=0; i<color_count; i++) {
		  LightAnim[slot_id].addColor(offset+i,Network.getParamRGB());
	  }  
  }
  return "+ok=+ANIM-SETCOLOR:"+String(slot_id);
}
else if (Network.isCommand("+ANIM-SETRGB")) {
  int slot_id = Network.getParam(0);
  int index = Network.getParam(1);
  if (slot_id >= 0 && slot_id < TFLIGHT_ANIM_MAX_SLOT) {
	  TFLightAnimRGB tmp;
	  tmp.start_color = Network.getParam(2);
	  tmp.end_color = Network.getParam(3);
	  tmp.fade = Network.getParam(4);
	  tmp.hold = Network.getParam(5);
	  tmp.hold_p = Network.getParam(6);
	  tmp.random = Network.getParam(7);
	  tmp.repeat = Network.getParam(8);
	  if (LightAnim[slot_id].addRGBProgram(index, tmp)) {
		return "+ok=+ANIM-SETRGB:"+String(slot_id)+","+String(index);
	  }
  }
  return "-err=+ANIM-SETRGB:"+String(slot_id);
}
else if (Network.isCommand("+ANIM-SAVE")) {
  int slot_id = Network.getParam(0);
  if (slot_id >= 0 && slot_id < TFLIGHT_ANIM_MAX_SLOT) {
	   LightAnim[slot_id].setName(Network.getParamString(1));
	   if (LightAnim[slot_id].save()) {
		  return "+ok=+ANIM-SAVE:"+String(LightAnim[slot_id].getPid());
	   }
  }
  return "-err=+ANIM-SAVE:"+String(slot_id);
}

else if (Network.isCommand("+PLIST")) {
	if (Network.getParam(0) >= 0 && Network.getParam(0) < 3) {
		uint8_t start = Network.getParam(1) * TFLIGHTANIM_PROGLIST_LIMIT;
		TFLightsProgList res = LightProg.getList(Network.getParam(0),start);
		
		String json = "{\"p\":"+String(Network.getParam(1))+",\"n\":";
		if (res.count == TFLIGHTANIM_PROGLIST_LIMIT) { json += String(start+1); }
		else { json += "0"; }
		json += ",\"items\":[";
		for (int i=0; i<TFLIGHTANIM_PROGLIST_LIMIT; i++) {
			if (res.count > i) {
				json += "{\"n\":\""+String(res.items[i].name)+"\",\"i\":"+String(res.items[i].program_id)+",\"t\":"+String(res.items[i].program_type)+"}";
				if (i+1 < res.count) {
					json += ",";
				}
			}
		}
		json += "]}";
		return "+ok=+PLIST:"+json;
	}else {
		return "-err=+PLIST";
	}
}

//save program to file
else if (Network.isCommand("+PSTORE-START")) {
	if (Network.getParam(0) >= 0 && Network.getParam(0) < 3) {
		int id = LightProg.saveTo(Network.getParam(0), Network.getParam(1), "", false);
		if (id == -1) {
			return "-err=+PSTORE-START:No_space_left";
		}else {
			return "+ok=+PSTORE-START:"+String(id);
		}
	}
	return "-err=+PSTORE-START";
}

else if (Network.isCommand("+PSTORE")) {
	if (Network.getParam(0) >= 0 && Network.getParam(0) < 3) {
		if (Network.getParam(1)>-1) {
		
			String line = Network.getParamString(2);
			line.replace("::","=");
			line.replace(";;",",");
			line.replace("#","\n");
			
			Serial.println(line);
			
			int id = LightProg.saveTo(Network.getParam(0), Network.getParam(1), line, true);
			if (id == -1) {
				return "-err=+PSTORE:No_space_left";
			}else {
				return "+ok=+PSTORE:"+String(id);
			}
		}
	}
	return "-err=+PSTORE";
}
//save direct lines to a program;
else if (Network.isCommand("+PREM")) {
	if (Network.getParam(0) >= 0 && Network.getParam(0) < 3) {
		if (LightProg.remove(Network.getParam(0),Network.getParam(1))) {
			return "+ok=+PREM:"+String(Network.getParam(1));
		}
	}
	return "-err=+PREM";
}

else if(Network.isCommand("+PREMALL")) {
	TFSPIFFS.removeAll("/anim");
	return "+ok=+PREMALL";
}
