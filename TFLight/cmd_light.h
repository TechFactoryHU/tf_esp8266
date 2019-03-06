/*
*	Light control related commands
*	- Set/Get color
*	- Set/Get/Start color animation
*	....
*/

if(Network.isCommand("+BRIGHT")) {
   int hw_id = Network.getParam(0);
   if (hw_id >= 0 && hw_id < Lights.getHwCount(true)) {
	  //manual color
	  //Lights.status[hw_id].mode = 1;
	  if (Network.getParam(1) == 0) {
			LightModeChanged(hw_id,0);
	  }else {
			LightModeChanged(hw_id,1);
	  }
	  Lights.setBrightness(Network.getParam(1), hw_id); 
	  return "+ok="+String(Network.getCommand());
   }
}
else if(Network.isCommand("+RGB")) {
   int hw_id = Network.getParam(0);
   RGB color = Network.getParamRGB(1);
   for (int i=0; i<Lights.getHwCount(false); i++) {
	  if (hw_id == -1 || hw_id == i) {
		  if (color.r == 0 && color.g == 0 && color.b == 0) {
			LightModeChanged(i,0);
		  }else {
			//manual color
			LightModeChanged(i,1);
			//Lights.status[i].mode = 1;
			Lights.status[i].type = 1;
		  }
		  Lights.setColor(color, i); 
	  }
   }
   return "+ok="+String(Network.getCommand());
}
//+C=SR,SG,SB,ER,EG,EB,FADETIME,HOLDTIME,ONCE,STRIP,TURN_OFF_OTHERS
else if (Network.isCommand("+RGBA")) {
   int hw_id = Network.getParam(0);
   int anim = getAnimIndex();
   
   LightAnim[anim].reset();
   LightAnim[anim].setType(1,1);
   LightAnim[anim].setColors(2);
   LightAnim[anim].resetOnEnd(true);
   LightAnim[anim].addColor(0, Network.getParamRGB());
   LightAnim[anim].addColor(1, Network.getParamRGB());

   TFLightAnimRGB tmp;
   tmp.start_color = 0;
   tmp.end_color = 1;
   tmp.fade = Network.getParam();
   tmp.hold = Network.getParam();
   tmp.hold_p = Network.getParam();
   tmp.random = Network.getParam();
   tmp.repeat = Network.getParam();
   
   if (!LightAnim[anim].addRGBProgram(0, tmp)) {
		return "-err=+RGBA";
   }
   
   for (int i=0; i<Lights.getHwCount(false); i++) {
	  if (hw_id == -1 || hw_id == i) {
		  LightAnim[anim].addHw(i);
		  //manual color
		  Lights.status[i].mode = 1;
		  Lights.status[i].type = 2;
	  }
   }
}
else if(Network.isCommand("+PIXGRP")) {
   int grp_id = Network.getParam(0);
   //manual color
   Lights.setPixelsGroupColor(Network.getParamRGB(1), grp_id); 
   return "+ok="+String(Network.getCommand());
}
else if(Network.isCommand("+PIXMULTI")) {
   int hw_id = Network.getParam(0);
   RGB color = Network.getParamRGB(1);
   Lights.setPixelsColor(color, hw_id, Network.getParamString(4), ";");
   return "+ok=+PIXMULTI"; 
}
else if(Network.isCommand("+PIX")) {
   int hw_id = Network.getParam(0);
   int px_id = Network.getParam(1);
   if (hw_id >= 0 && hw_id < Lights.getHwCount(false)) {
	 //manual color
	 Lights.status[hw_id].mode = 1;
	 if (px_id == -1) {
		Lights.setPixelsColor(Network.getParamRGB(2), hw_id); 
	 }else {
		Lights.setPixelColor(Network.getParamRGB(2), px_id, hw_id); 
	 }
	 if (Network.getParam(5) == 1) { return "+ok=+PIX"; }
   }
   return "";
}
