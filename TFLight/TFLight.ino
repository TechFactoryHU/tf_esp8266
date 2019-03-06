/*
 *   TFLight
 *	 -------
 *	 HU:	Altalanos vilagitas vezerlo ESP8266-ra
 *	 		RGB, LED, es cimezheto NeoPixel(Ws2812b) ledszalaghoz
 *			Vezerlo appok elerhetok win, android es ios rendszerre
 *
 *	 EN:	Universal light controller for ESP8266 
 *			for RGB strips, Led strips and NeoPixel(Ws2812b) strips
 *			Controller applications are available for win, android and ios 
 *
 *   v1.0 - 2017.05.20
 *	 v1.1 - 2018.10.19
 *	 
 *   techfactory.hu
 *	 Licensed under MIT - https://en.wikipedia.org/wiki/MIT_License
 *	 Copyright (c) 2016-2019 techfactory.hu
 */

#include "helpers.h"
#include "tfheader.h"
#include "tfcustomcfg.h"
#include "tfnetwork.h"
#include "tfeq.h"
#include "tflights.h"
#include "tflightanim.h"
#include "tflightprogram.h"
#include "tfspiffs.h"
#include "tftimer.h"
#ifdef TF_DEBUG
#include "tfdebug.h"
#endif

extern "C" {
  #include "user_interface.h"
}

//Device default config
#include "config.h"

TFNetwork Network(&Config);
TFLights Lights("hwconfig.dat");
TFEq EQ(3);

TFLightAnim LightAnim[TFLIGHT_ANIM_MAX_SLOT];
RGB EQColorPalette[TFLIGHT_MAX_COLORS*TFLIGHT_EQCOLORPALETTE_SIZE];
TFLightProgram LightProg;
TFTimer Timer(&Config);

//setup hardware (pins & light types)
//TODO: Read it from config
#include "config_hw.h"

//Timer
void TimerAction(TFTIMER_ITEM item) {
	  //defined in tftimer.h
	  //TFTIMER_ITEM { 
    //  d, h, m (day, hour, minute)
    //  byte action = 0:turn off (and stop all programs), 1: turn on, 2: start program
    //  byte param1 = 
    //  int param2 =
    //  char param3[10] = 
    
    if (item.action == 0) {
       for (int i=0; i<Lights.getHwCount(false); i++) {
            LightModeChanged(i,0);
            Lights.status[i].type = 0;
            Lights.setColor(RGB{0,0,0}, i);
       }
    }else if(item.action == 1) {
       for (int i=0; i<Lights.getHwCount(false); i++) {

       }
    }
}

//Start devices with their defaults
void Start() {
	 for (int i=0; i<Lights.getHwCount(true); i++) {
		 //has default config
		 if (Config.exists("SD"+String(i))) {
			String cfg = Config.getString("SD"+String(i));
			char *tmp;
			char *s;
			TFLightState status;
			s = strtok_r(&cfg[0],",", &tmp);
			//mode
			if (s != NULL) {
				status.mode = atoi(s);
				
				uint8_t index = 0;
				while ((s = strtok_r(NULL,",",&tmp)) != NULL) {
					if (index == 0) {
						status.type = atoi(s);
					}else if(index == 1) {
						status.param1 = atoi(s);
					}else if(index == 2) {
						status.param2 = atoi(s);
					}else if(index == 3) {
						status.param3 = atoi(s);
					}else if(index == 4) {
           status.param4 = atoi(s);
          }else if(index == 5) {
						status.color.r = atoi(s);
					}else if(index == 6) {
						status.color.g = atoi(s);
					}else if(index == 7) {
						status.color.b = atoi(s);
					}
					index++;
				}
			}else {
				status.mode = 0;
			}
			
			//simple color
			if (status.mode == 1) {
				Lights.setStatus(i,status);
				Lights.setColor(status.color, i);
			//program_mode
			}else if(status.mode == 2) {
				if (status.param1 >= 0 && status.param1 < TFLIGHT_ANIM_MAX_SLOT) {
					if (LightProg.load(0, status.type, &LightAnim[status.param1])) {
						LightAnim[status.param1].addHw(i);
						Lights.setStatus(i,status);
					}
				}
			//eq (internal only)
			}else if(status.mode == 3||status.mode == 4) {
				  Lights.EQType = 1;
          Lights.setStatus(i,status);
			}
		 }else {
			 Lights.setStatus(i,0,0);
			 Lights.setColor(RGB{0,0,0},i);
		 }
	 }
}

/*
**  Setup
**  -----
*/

void setup() {
  #ifdef TF_DEBUG
    Serial.begin(115200); 
    Serial.println("Starting up...");
  #endif
  
  TFSPIFFS.begin();
  
  //Load config
  Config.begin();
  Config.load();
  
  //pwm freq
  if (Config.getInt("PWM")>0) {
      analogWriteFreq(Config.getInt("PWM"));
  }
  
  //lights setup
  Lights.load(DefaultLights);
  Lights.setup();
  
  //networking
  Network.begin();
  Network.OnMsg(Network_Incoming);
  Network.OnDeviceInfo(Network_Devinfo);
  Network.OnHTTPRequest(Network_Http);
  
  Timer.jobHandler(TimerAction);
  
  //eq color palette from config
  int colors = Config.getInt("EQC");
  if (colors>0) {
    if (colors > TFLIGHT_MAX_COLORS) { colors = TFLIGHT_MAX_COLORS; }
    RGB *tmp;
    for (int i=0; i<colors; i++) {
      tmp = Config.getRGBs( String("EQ"+String(i)), TFLIGHT_EQCOLORPALETTE_SIZE);
      for (int it=0; it<TFLIGHT_EQCOLORPALETTE_SIZE; it++) {
         EQColorPalette[i*TFLIGHT_EQCOLORPALETTE_SIZE+it] = tmp[it];
      }
    }
  }
  
  Lights.setEQColors(EQColorPalette, TFLIGHT_MAX_COLORS);
  EQ.setup();
  if (Config.exists("EQP")) {
     EQ.paramsFromString(Config.getString("EQP"));
  }
  
  //setup light anim slots
  for (int i=0;i < TFLIGHT_ANIM_MAX_SLOT; i++) {
    LightAnim[i].setTFLights(&Lights);
  }
  
  Timer.load();
  Timer.start();
}


/*
**  Main loop
**  ---------
*/

unsigned long worktime = 0;
bool EQActive = false;
bool AnimActive = false;

void loop() {
   AnimActive = false;
   EQActive   = false;
   
   #ifdef TF_DEBUG
    //watch commands from serial
    if (Serial.available()>0) {
    String rd = Serial.readStringUntil('\n');
    //remove \n
    rd.remove(rd.length()-1,1);
    String response = Network_Incoming(&rd[0],serialClient);
      if (response.length()>0) {
         Serial.println(">"+response);
      }
    }
   #endif

   if (worktime+50 < millis()) {
      worktime = millis();
      for (int i=0; i < Lights.getHwCount(true); i++) {
        //manual color
        if (Lights.status[i].mode == 1) {
          //simple color 
          if (Lights.status[i].type == 1) {
          //color, holdt, fadet, once
          }else if (Lights.status[i].type == 2) {
              AnimActive = true;
          }
        }
        //prog mode
        else if (Lights.status[i].mode == 2) {
			    AnimActive = true;
        }
        //eq mode
        else if (Lights.status[i].mode == 3||Lights.status[i].mode == 4) {
			    EQActive = true;
        }
      }
      
      if (EQActive) {
         Lights.autoEQMode(EQ.get(0));
         Lights.EQDisplay(&EQ); 
      }
      Network.CheckSta(false);
   }
   
   if (AnimActive) {
     for (int i=0; i<TFLIGHT_ANIM_MAX_SLOT;i++) {
       //valid program
       if (LightAnim[i].getType()>0) {
           LightAnim[i].update(millis());
       }
     }
   }
   
   if (EQActive) {
      EQ.update(millis());
   }
   
   Lights.update();
   TFSPIFFS.update();
   Timer.update();

   Network.UdpWorker();
   Network.TcpWorker();
}

int getAnimIndex() {
  for (int i=0;i < TFLIGHT_ANIM_MAX_SLOT; i++) {
    if (LightAnim[i].getType() == 0 ) {
        return i;
    }
  }
  return TFLIGHT_ANIM_MAX_SLOT-1;
}

//handling http requests (experimental)
bool Network_Http(uint8_t method, char* request_url, char* data, WiFiClient client) {
	#ifdef TF_DEBUG
		Serial.print("[HTTP] ");
		Serial.print(method==1?"GET":"POST");
		Serial.print(" ");
		Serial.print(request_url);
		Serial.print(" ");
		Serial.print(data);
		Serial.println();
	#endif
	
	char *tmp;
	char *seq;
	
	//On HTTP [GET|POST] /device/deviceId/command
	if (String(request_url).startsWith("/device/")) {
		//Get device status, GET /device/[deviceId]
		if (method == 1) {
			seq = strtok_r(request_url,"/",&tmp);
			seq = strtok_r(NULL,"/",&tmp);
			seq = strtok_r(NULL,"/",&tmp);
			int hw_id = -1;
			if (seq != NULL) {
				hw_id = atoi(seq);
			}
			if (hw_id >= 0 && hw_id <Lights.getHwCount(false)) {
				Network.HTTPResponse(200,String(Lights.status[hw_id].mode)+","+String(Lights.status[hw_id].type), "application/json",client);
			}else if(hw_id == -1) {
				Network.HTTPResponse(200,String(Lights.status[hw_id].mode)+","+String(Lights.status[hw_id].type), "application/json",client);
			}
			//request handled
			return true;
		//On HTTP GET /device/deviceId	-> Status
		}else if(method == 2) {
			
		}
	}
	else if (String(request_url).startsWith("/anim/")) {
		seq = strtok_r(request_url,"/",&tmp);
		seq = strtok_r(NULL,"/",&tmp);
		int hw_id = -1;
		if (seq != NULL) {
			hw_id = atoi(seq);
		}
		Network.ServeHTTPStaticFile("/anim/"+String(hw_id), "text/plain", client);
		return true;
	}
 
	//request unhandled
	return false;
};

void LightModeChanged(uint8_t hwid, uint8_t mode) {
	if (Lights.status[hwid].mode == 2) {
		if (Lights.status[hwid].param1 >= 0 && TFLIGHT_ANIM_MAX_SLOT > Lights.status[hwid].param1) {
			LightAnim[Lights.status[hwid].param1].remHw(hwid);
			if (LightAnim[Lights.status[hwid].param1].hwCount() == 0) {
				LightAnim[Lights.status[hwid].param1].reset();
			}
		}
	}
 
  if (mode == 0) {
     Lights.setColor(RGB{0,0,0}, hwid);
  }
  
	Lights.status[hwid].mode = mode;
};

String getLightStatus() {
	String json = "[";
	for (int i=0; i<Lights.getHwCount(true); i++) {
		if (i!=0) { json += ","; } 
		json += "{\"i\":"+String(i)+",\"m\":"+String(Lights.status[i].mode)+",\"t\":"+String(Lights.status[i].type)+",\"p1\":"+String(Lights.status[i].param1);
   /* json += ",\"p2\":"+String(Lights.status[i].param2);
    json += ",\"p3\":"+String(Lights.status[i].param3);
    json += ",\"p4\":"+String(Lights.status[i].param4);*/
    json += "}";
	}
	json += "]";
	return json;
};

//command handler
#include "cmd_network_defaults.h"
String Network_Incoming(char* packet, TFNETWORK_CLIENT client) {
  int pcount		  = -1;
  bool trusted		= false;
  bool neutral		= false;
  
  #include "cmd_auth.h"
  pcount = Network.parsePacket(packet);
  if (pcount > -1) {
    if (Network.isCommand("+INFO")) {
      return Network_Devinfo(client);
    }
	  else if (Network.isCommand("+STOP")) {
          int hw_id = -1;
          if (pcount > 0) { hw_id = Network.getParam(0); }
          for (int i=0; i<Lights.getHwCount(false); i++) {
            if (hw_id == -1 || hw_id == i) {
				        LightModeChanged(i,0);
				        Lights.status[i].type = 0;
                Lights.setColor(RGB{0,0,0}, i);
            }
         }
         return "+ok=+STOP";
      }
      else if (Network.isCommand("+MODE")) {
		int hw_id = Network.getParam(0);
		for (int i=0; i<Lights.getHwCount(true); i++) {
			if (hw_id == -1 || hw_id == i) {
				LightModeChanged(i,Network.getParam(1));
				Lights.status[i].type   = Network.getParam(2);
				Lights.status[i].param1 = Network.getParam(3);
				Lights.status[i].param2 = Network.getParam(4);
				Lights.status[i].param3 = Network.getParam(5);
        Lights.status[i].param4 = Network.getParam(6);
			}
		}
		return "+ok=+MODE:"+String(hw_id);
     }
	 else if (Network.isCommand("+STATUS")) {
		return "+ok=+STATUS:"+getLightStatus();
     }
    
     #include "cmd_light.h"
     #include "cmd_prog.h"
	 #include "cmd_eq.h"
	
     return Network_Default_Cmds(pcount);
  }
  return "-err=UKNOWN_CMD:"+String(Network.getCommand());
}

//Send device info packet
String Network_Devinfo(TFNETWORK_CLIENT c) {
	#ifdef TF_DEBUG
		Serial.print("Devinfo request= ");
		Serial.println(c.ip);
	#endif
	
    int hwc = Lights.getHwCount(false);
    String json = "{";
    json += "\"fw\":\""+String(FW_VER)+"\",\"nr\":\""+String(Network.getHWNR())+"\",\"dc\":"+String(hwc);
    if (Config.getBool("AUT")==true) {
		  json += ",\"ato\":"+String(Network.getAuthToken());
    }
    if (TFHb_get_attr(c.attrs, 5)) 		{ json += ",\"ast\":1"; }
    else if (TFHb_get_attr(c.attrs, 4)) { json += ",\"ast\":2"; }
    
    #ifdef TFNETWORK_TCPSERVER && TFNETWORK_TCPSERVER == 1
      json += ",\"tcp\":"+String(TFNETWORK_TCPPORT)+"";
		  #ifdef TFNETWORK_WEBSERVER && TFNETWORK_WEBSERVER == 1
          json += ",\"web\":1";
		  #endif
    #else
      json += ",\"tcp\":\"0\",\"web\":0";
    #endif
    
    //EQ-DATA support
    json += ",\"eq\":"+String(EQ.bars());
   
    json += ",\"name\":\"";
    json += Config.getString("DN");
    json += "\"";
    json += "}";
  
    return json;
}
