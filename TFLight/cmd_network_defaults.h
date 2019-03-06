/*
*	Basic commands
*	(Networking, Device mgmt, ...)
*/

String Network_Default_Cmds(int pcount) {
	 //devinfo request
	 if(Network.isCommand("+DEV-INFO")) {
		if (pcount==0) { return "+ok=+DEV-INFO:c:"+String(Lights.getHwCount(false)); }
		else {
			int hwc = Lights.getHwCount(false);
			int hwid = Network.getParam(0);
			String json;
			
			if (hwid >= 0 && hwid < hwc) {
				TFLightConfig hw = Lights.getHw(hwid);
				json = "{\"id\":"+String(hwid)+",\"t\":"+String(hw.type)+",\"pin1\":"+String(hw.pin1);
				json += ",\"pin2\":"+String(hw.pin2);
				json += ",\"pin3\":"+String(hw.pin3);
				json += ",\"p1\":"+String(hw.param1);
				json += ",\"n\":\""+String(hw.devname)+"\"";
				
				if (hw.type == 3) {
					if (Lights.hasPixelGroup(hwid)) {
						String custom;
						TFLightPixelGroup tmp;
						for (int g=0; g<Lights.getPixelGroupCount(); g++) {
							tmp = Lights.getPixelGroup(g);
							if (tmp.hwid == hwid) {
								if (custom.length()>0) { custom += ";"; }
								custom += String(tmp.gid)+","+String(tmp.hwid)+","+String(tmp.start)+","+String(tmp.end)+","+String(tmp.pxname);
							}
						}
						json += ",\"c\":\""+custom+"\"";
					}
				}
				
				json += "}";
				return "+ok=+DEV-INFO:"+json;
			}else {
				return "-err=+DEV-INFO:Out_of_range";
			}
		}
		
		String json = "+ok=+DEV-INFO:{";
		json += "\"pc\":"+String(TFLIGHT_EQCOLORPALETTE_SIZE)+",\"cc\":"+String(TFLIGHT_MAX_COLORS);
			#ifdef TFEQ_MSGEQ7_STROBE && TFEQ_MSGEQ7_RESET
				json +=",\"i\":1",
			#endif
		json += "}";
		
		
		return json;
     }
	 //Restart device
	 else if (Network.isCommand("+DEV-REST")) {
         ESP.restart();
         return "";
     //reset main configuration
     } else if (Network.isCommand("+DEV-CFGRST")) {
          Config.remove();
          ESP.restart();
     }
     //scan wifi networks
     else if (Network.isCommand("+WIFI-SCAN")) {
        return "+ok=+WIFI-SCAN:"+Network.Scan();
     }
     
     else if(Network.isCommand("+NET-INFO")) {
		 String jinfo = "{\"sta\":\"";
		 if (Network.StaSSID()) { jinfo += Network.StaSSID(); }
		 jinfo += "\",\"sts\":";
		 if (Network.StaStatus()) { jinfo += "1"; }else{ jinfo += "0"; }
		 jinfo += ",\"ap\":\""+Config.getString("AP")+ "\",\"aps\":";
		 if (Network.ApStatus()) { jinfo += "1"; }else{ jinfo += "0"; }
		 jinfo += ",\"ast\":"+Config.getString("AST");
		 jinfo += ",\"apc\":";
		 if (Config.getInt("NCH")>0) {
		 jinfo += Config.getString("NCH");
		 }else { jinfo += String(TFNETWORK_WIFI_CH); }
		 jinfo += ",\"at\":"+Config.getString("NIP");
		 jinfo += ",\"ip\":\""+WiFi.localIP().toString()+"\"";
		 jinfo += ",\"gw\":\""+WiFi.gatewayIP().toString()+"\"";
		 jinfo += ",\"sn\":\""+WiFi.subnetMask().toString()+"\"";
		 jinfo += ",\"dn\":\""+WiFi.dnsIP(0).toString()+"\"";
		 jinfo += ",\"dhn\":\""+WiFi.hostname()+"\"}";
		 return "+ok="+String(Network.getCommand())+":"+jinfo;
     }
     
     else if (Network.isCommand("+NET-IP")) {
		 return "+ok=+NET-IP:"+String(WiFi.localIP().toString());
     }
     
     else if (Network.isCommand("+NET-IPCFG")) {
		if (pcount >= 3) {
			IPAddress addr;
			if (addr.fromString(Network.getParamString(0))) {
				if (addr.fromString(Network.getParamString(1))) {
					if (addr.fromString(Network.getParamString(2))) {
						Config.add("NIP",1);
						Config.add("NAD",Network.getParamString(0));
						Config.add("NSM",Network.getParamString(1));
						Config.add("NGW",Network.getParamString(2));
						if (addr.fromString(Network.getParamString(3))) {
							Config.add("NDN",Network.getParamString(2));
						}
						Config.save();
						return "+ok=+NET-IPCFG";
					}
				}
			}
			return "-err=+NET-IPCFG";
		}else if (pcount == 1) {
			if (Network.getParam(0) == -1) {
				Config.add("NIP",0);
				Config.save();
			}
			return "+ok="+String(Network.getCommand());
		}else if (pcount == 0) {
			return "+ok=+NET-IPCFG:"+Config.getString("NAD")+","+Config.getString("NSM")+","+Config.getString("NGW");
		}
     }
     
     //set station (client) mode
     else if (Network.isCommand("+NET-STA")) {
        if (pcount>0) {
          String STAssid = Network.getParamString(0);
          if (Network.getParam(0) == -1) { 
              Network.StaDisconnect();
              ESP.restart();
          }else {
              if (Network.getParamString(2) != "") {
                  int stamode = Network.getParam(2);
                  if (stamode == 1) { Config.add("AST","1");  }
                  else {
                    Config.add("AST","0"); 
                  }
                  Config.save();
              }
              Network.StaConnect(Network.getParamString(0),Network.getParamString(1));
              if (Network.CheckSta(true)) {
                   return "+ok=NET-STA:"+String(Network.StaSSID());
                  //ESP.restart();
              }else {
                  return "-err=+NET-STA:"+String(STAssid);
              }
          }
        }else {
            if (Network.StaSSID()) {
              if (Network.StaStatus()) {
                 return "+ok=NET-STA:"+String(Network.StaSSID());
              }else {
                 return "-err=+NET-STA:"+String(Network.StaSSID());
              }
            }else {
              return "-err=+NET-STA:0";
            }
        }
     }
     //Enable/Disable AP/STA mode
	else if(Network.isCommand("+NET-APSTA")) {
		if (pcount==1) {
			if (Network.getParam(0) == 1) {
				Config.add("AST","1");
			}else {
				Config.add("AST","0");
			}
			Config.save();
		}
		return "+ok=+NET-APSTA:"+( Config.getBool("AST") ? String("1") : String("0"));
	}
	//AP config (SSID,PASSWD)
    else if(Network.isCommand("+NET-AP")) {
		if (pcount>0) {
			Config.add("AP",Network.getParamString(0));
			if (Network.getParamString(1) != "") {
				Config.add("PWD",Network.getParamString(1));
			}
			Config.save();
			Network.StartAp();
			
			return "+ok=+NET-AP";
		}
    }      
    //DHCP
    else if(Network.isCommand("+NET-DHCP")) {
		if (pcount > 0) {
			Config.add("NIP", Network.getParam(0));
		}
		return "+ok=+NET-DHCP:"+Config.getString("NIP");
    } 
    //WiFICH
    else if(Network.isCommand("+NET-WIFICH")) {
		if (pcount > 0) {
			Config.add("NCH", Network.getParam(0));
		}
		return "+ok=+NET-WIFICH:"+Config.getString("NCH");
    } 
	//set device name
    else if(Network.isCommand("+DEV-NAME")) {
		if (Network.getParamString(0) != "") { 
			Config.add("DN",Network.getParamString(0)); 
			Config.save(); 
			return "+ok=+DEV-NAME:"+Network.getParamString(0);
		}else {
			return "+ok=+DEV-NAME:"+Config.getString("DN");
		}
    }
    //set ap password  
    else if(Network.isCommand("+DEV-PWD")) {
		if (Network.getParamString(0) != "") { 
			Config.add("PWD", Network.getParamString(0)); 
			Config.save(); 
			return "+ok=+DEV-PWD";
		}
    }
    //set ap password  
    else if(Network.isCommand("+DEV-ADMIN-PWD")) {
		if (Network.getParamString(0) != "") { 
			Config.add("APW", Network.getParamString(0)); 
			Config.save(); 
			return "+ok=+DEV-ADMIN-PWD";
		}
    }
    //set device password
    else if(Network.isCommand("+DEV-AUTH")) {
		if (Network.getParam(0) == 1) {
			if (Config.exists("APW")) {
				Config.add("AUT","1");
				Config.save(); 
			}
		}else { Config.add("AUT","0"); Config.save();  }
		return "+ok=+DEV-AUTH";
    }
    //change PWM frequency (0=esp default)
    else if(Network.isCommand("+DEV-PWMFREQ")) {
		analogWriteFreq(Network.getParam(0));	
		Config.add("PWM",Network.getParamString(0)); 
		Config.save(); 
		return "+ok=+DEV-PWMFREQ:"+Network.getParamString(0);
    }
    //webadmin update from url
    else if (Network.isCommand("+DEV-WEBUP")) {
		if (TFSPIFFS.downloadAll(Network.getParamString(0), "/www", true)) {
			return "+ok=+DEV-WEBUP";
		}
		return "-err=+DEV-WEBUP";
    }
    else if(Network.isCommand("+DEV-DIR")) {
		auto listcallback = [](String f, String p) { Serial.println(p); };
		TFSPIFFS.list(Network.getParamString(0), listcallback);
    }
    //fw update from url
    else if (Network.isCommand("+DEV-FWUP")) {
		
    }
    //download config file from remote http server
    else if (Network.isCommand("+DEV-DLCFG")) {
		
    }
    //get current time from remote http server
    else if (Network.isCommand("+DEV-UPDTIM")) {
		Timer.updateTime(true);
		return "+ok=+DEV-UPDTIM";
    }
    //enable/disable timer service
    else if(Network.isCommand("+DEV-TIMER")) {
		Config.add("TIM", Network.getParam(0)); 
		if (Network.getParamString(1) != "") {
			Config.add("NTS", Network.getParamString(1)); 
		}
		if (Network.getParam(0)==1) { Timer.start(); }
		else { Timer.stop(); }
		
		return "+ok=+DEV-TIMER:"+String(Network.getParam(0));
    }else{
		return "-err=UNKNOWN_CMD:"+String(Network.getCommand()); 
    }
    
    return "-err="+String(Network.getCommand()); 
};