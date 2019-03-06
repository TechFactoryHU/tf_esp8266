/*
*	EQ (Spectrum analyzer) mode & controls
*	with MSGEQ7 & External spectrum analyzer (pc software, tfcontrol)
*/

//TFLIGHT_ANIM_MAX_SLOT
if(Network.isCommand("+EQ-INFO")) {
	String json = "+ok=+EQ-INFO:{";
	json += "\"b\":"+String(EQ.bars())+",\"pc\":"+String(TFLIGHT_EQCOLORPALETTE_SIZE)+",\"cc\":"+String(TFLIGHT_MAX_COLORS);
	#ifdef TFEQ_MSGEQ7_STROBE && TFEQ_MSGEQ7_RESET
		json +=",\"i\":1",
	#endif
	json += ",\"t\":"+String(Lights.EQType)+"}";
	return json;
}
else if (Network.isCommand("+EQAUTO")) {
	if (pcount > 0) {
		Lights.setAutoEQMode(Network.getParam(0),Network.getParam(1));
		if (pcount == 3) {
			Lights.setEqAutoTreshold(Network.getParam(2),0);
		}
	}
	return "+ok=+EQAUTO:"+String(Lights.getAutoEQMode());
}
//External EQ data
else if (Network.isCommand("+EQ")) {
	if (Lights.EQType == 0) {
		 int barcount = Network.getParam(0);
		 int value = 0;
		 uint8_t bindex = 0; 
		 while (barcount > bindex) {
			 value = Network.getParam(bindex+1);
			 if (value >= 0 && value <= 255) {
				EQ.data(bindex, value);
			 }else {
				EQ.data(bindex, 0);
			 }
			 bindex++;
		 }
	}
	return "";
}
//0=External, 1=Internal EQ data source   
else if (Network.isCommand("+EQTYPE")) {
	if (pcount > 0) {
	  Lights.EQType = Network.getParam(0) == 1 ? 1 : 0;
	}
	return "+ok=+EQTYPE:"+String(Lights.EQType);
}

else if (Network.isCommand("+EQCOLORS-SET")) {
	int cid = Network.getParam(0);
	if (cid >= 0 && cid < TFLIGHT_MAX_COLORS) {
		for (int i=0; i< TFLIGHT_EQCOLORPALETTE_SIZE; i++) {
			  EQColorPalette[cid + i] = Network.getParamRGB();
		}
	return "+ok=+EQCOLORS-SET";
	}
}
else if (Network.isCommand("+EQCOLORS-GET")) {
	int cid = Network.getParam(0);
	if (cid >= 0 && cid < TFLIGHT_MAX_COLORS) {
		String colors;
		for (int i=0; i< TFLIGHT_EQCOLORPALETTE_SIZE; i++) {
			  colors += (colors != NULL ? ";" : "") +String(EQColorPalette[cid*TFLIGHT_EQCOLORPALETTE_SIZE + i].r)+","+String(EQColorPalette[cid*TFLIGHT_EQCOLORPALETTE_SIZE + i].g)+","+String(EQColorPalette[cid*TFLIGHT_EQCOLORPALETTE_SIZE + i].b);
		}
		return "+ok=+EQCOLORS-GET:"+String(cid)+";"+colors;
	}
}
else if (Network.isCommand("+EQRANGE")) {
	if (pcount == 2) {
		EQ.setAnalogMap(Network.getParam(0),Network.getParam(1));
		return "+ok="+String(Network.getCommand());
	}
	return "-err="+String(Network.getCommand());
}

else if (Network.isCommand("+EQPARAMS")) {
	if (pcount == 3) {
		//time (default 30ms), max = 2, level = 10
		EQ.setDrop(Network.getParam(0),Network.getParam(1),Network.getParam(2));
		return "+ok="+String(Network.getCommand());
	}
	return "-err="+String(Network.getCommand());
}

else if (Network.isCommand("+EQTRES")) {
	Lights.setEqAutoTreshold(Network.getParam(0),Network.getParam(1));
	return "+ok="+String(Network.getCommand());
}

//save current values
else if (Network.isCommand("+EQPSAVE")) {
	Config.add("EQP", EQ.paramsToString());
	Config.save();
	return "+ok="+String(Network.getCommand());
}