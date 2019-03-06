//auth request
if (String(packet).startsWith("+AUTH")) {
	pcount = Network.parsePacket(packet);
	if (pcount == 2) {
		if (Network.auth(Network.getParamString(0),Network.getParamString(1), client)) {
			if (Network.getParamString(0) == "admin") {
				return "+ok=+AUTH:1";
			}
			else {
				return "+ok=+AUTH:2";
			}
		}else {
			return "-err=+AUTH";
		}
	}else {
		if (TFHb_get_attr(client.attrs, 4)) {
			return "+ok=+AUTH:2";
		}else if(TFHb_get_attr(client.attrs, 5)) {
			return "+ok=+AUTH:1";
		}
		return "+AUTH="+String(Network.getAuthToken());
	}
}

for (int i=0; i<TFNETWORK_NEUTRAL_COMMANDS; i++) {
	if (String(packet).startsWith(Network_neutral_commands[i])) {
	//if (strncmp(Network_neutral_commands[i], packet, strlen(Network_neutral_commands[i])) == 0) {
		neutral = true;
		break;
	}
}

//neutral command
if (neutral) {
	//devinfo||info: no auth required
	if (String(packet).startsWith("+DEV-INFO")||String(packet).startsWith("+INFO")) {
		trusted = true;
	}else {
		//require auth for control
		if (Config.getBool("AUT")){
			//user authenticated
			if (!TFHb_get_attr(client.attrs, 4) && !TFHb_get_attr(client.attrs, 5)) {
				//is auth packet
				if (String(packet).startsWith("+AUTH=")) {
					trusted = true;
				}else {
					return "-err="+String(Network.getCommand())+":AUTH,2,"+String(Network.getAuthToken());
				}
			}else {
				trusted = true;
			}
		}else {
			trusted = true;
		}
	}
//admin command
}else {
	//require auth for admin commands
	if (Config.getBool("ADM")) {
		//admin authenticated
		if (TFHb_get_attr(client.attrs, 5)) {
			trusted = true;
		}else {
			return "-err="+String(Network.getCommand())+":AUTH,1,"+String(Network.getAuthToken());
		}
	}else {
		trusted = true;
	}
}
