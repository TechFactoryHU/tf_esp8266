/*
*	Basic commands
*	(Networking, Device mgmt, ...)
*/

String Network_Default_Cmds(int pcount) {
	  //restart ESP module
	 if (Network.isCommand("+AT-REST")) {
         ESP.restart();
         return "";
         
     //scan wifi networks
     else if (Network.isCommand("+AT-SCAN")) {
        return "+ok=+AT-SCAN:"+Network.Scan();
     }
     //set station (client) mode
     else if (Network.isCommand("+AT-STA")) {
        if (pcount>=2) {
          String STAssid = Network.getParamString(0);
          if (STAssid == "-1") { 
              Network.StaDisconnect();
              return "";
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
                  ESP.restart();
              }else {
                  return "-err=+AT-STA:"+String(STAssid);
              }
          }
        }else {
            if (Network.StaSSID()) {
              if (Network.StaStatus()) {
                 return "+ok=AT-STA:"+String(Network.StaSSID());
              }else {
                 return "-err=AT-STA:"+String(Network.StaSSID());
              }
            }else {
              return "-err=+AT-STA:0";
            }
        }
     }
     //Enable/Disable AP/STA mode
	else if(Network.isCommand("+AT-APSTA")) {
		if (pcount==1) {
			if (Network.getParam(0)) {
				Config.add("AST","0");
			}else {
				Config.add("AST","1");
			}
			Config.save();
		}
		return "+ok=+AT-APSTA:"+( Config.getBool("AST") ? String("1") : String("0"));
	}
	//AP config (SSID,PASSWD)
    else if(Network.isCommand("+AT-AP")) {
		if (Network.getParamString(0) != "") {
			Config.add("AP",Network.getParamString(0));
			if (Network.getParamString(1) != "") {
				Config.add("PWD",Network.getParamString(1));
			}
			Config.save();
			Network.StartAp();
			return "+ok=+AT-AP";
		}
    }            
	//set device name
    else if(Network.isCommand("+AT-NAME")) {
		if (Network.getParamString(0) != "") { 
			Config.add("DN",Network.getParamString(0)); 
			Config.save(); 
			return "+ok=+AT-NAME:"+Network.getParamString(0);
		}
    }
    //set ap password  
    else if(Network.isCommand("+AT-PWD")) {
		if (Network.getParamString(0) != "") { 
			Config.add("PWD",Network.getParamString(0)); 
			Config.save(); 
			return "+ok=+AT-PWD";
		}
    }
    //change PWM frequency (0=esp default)
    else if(Network.isCommand("+AT-PWMFREQ")) {
		if (Network.getParam(0)) { 
			Config.add("PWM",Network.getParamString(0)); 
			Config.save(); 
		}
    }else {
		return "-err=UNKNOWN_CMD"; 
    }
    
    return "-err="+String(Network.getCommand()); 
};