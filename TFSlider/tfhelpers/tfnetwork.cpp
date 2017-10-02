#include <ESP8266WiFi.h> 
#include <WiFiUdp.h>
#include "../tfheader.h"
#include "tfcustomcfg.h"
#include "tfnetwork.h"
#include "helpers.h"

TFNetwork::TFNetwork(TFCustomCfg *config) {
	_cfg = config;
	
	WiFi.mode(WIFI_AP_STA);
	APLocalIp = IPAddress(192, 168, 10, 1);
	APgateway = IPAddress(192, 168, 10, 1);
	APsubnet  = IPAddress(255, 255, 255, 0);
	
	softAp 		 = false;
	staConnected = false;
	sta_last_check = 0;
	paramIndex = 0;
}

void TFNetwork::begin() {
	uint8_t  MAC_softAP[6]; 
	WiFi.softAPmacAddress(MAC_softAP);
	String _hwnr;
	for (int i = 0; i < sizeof(MAC_softAP); ++i){
	  _hwnr += String( MAC_softAP[i], HEX);
	}
	_hwnr.toCharArray(hwnr,20);
	_hwnr="";
	InitUdp();
	InitTcp();
}

void TFNetwork::SetAP(String Apssid, String APpassword, bool update) {
	if (update) {
		_cfg->add("AP",  Apssid);
		_cfg->add("PWD", APpassword);
		_cfg->save();
	}
	TFNetwork::StartAp(Apssid, APpassword);
}

void TFNetwork::StartAp() {
	if (softAp) {
		TFNetwork::StopAp();
	}
	if (_cfg->exists("AP") && _cfg->exists("PWD")) {
		TFNetwork::StartAp(_cfg->getString("AP"), _cfg->getString("PWD"));
	}
}

void TFNetwork::StartAp(String ssid, String password) {
	if (!softAp) {
		if (WiFi.softAPConfig(APLocalIp, APgateway, APsubnet)) {
			if (WiFi.softAP(ssid.c_str(), password.c_str(), 8, 0)) {
				softAp = true;
			}
		}
	}else {
		WiFi.softAP(ssid.c_str(), password.c_str(), 8, 0);
	}
}

void TFNetwork::StopAp() {
	softAp = false;
	WiFi.softAPdisconnect(true);
}

bool TFNetwork::ApStatus() {
	return softAp;
}

void TFNetwork::StaConnect(String ssid, String password) {
	WiFi.disconnect();
    #ifdef TF_DEBUG 
        Serial.println("Station connecting to "+ssid+"/"+password);
    #endif
    WiFi.begin(ssid.c_str(), password.c_str());
}

void TFNetwork::StaDisconnect(){
	WiFi.disconnect();
    WiFi.begin("","");
}

bool TFNetwork::StaStatus(){
	if (WiFi.status() != WL_CONNECTED) {
		return false;
	}
	return true;
}

String TFNetwork::StaSSID() {
	return WiFi.SSID();
}


bool TFNetwork::CheckSta(bool wait) {
    if (WiFi.SSID()) { 
        if (!wait) {
          if (sta_last_check < millis()) { 
              sta_last_check = millis()+10000;
              if (WiFi.status() != WL_CONNECTED) {
                 TFNetwork::StartAp();
              }else if(WiFi.status() == WL_CONNECTED) {
                 if (_cfg->getBool("D_APSTA") == true) { TFNetwork::StopAp(); }
              }
          }
          return true;
        }else {
            unsigned long start = millis()+10000;
            sta_last_check = millis()+20000;
            while (WiFi.status() != WL_CONNECTED) {
                if (start < millis()) {
                    return false;
                }
                delay(30);
            }
            return true;
        }
    }
    return false;
}

String TFNetwork::Scan() {
  int n = WiFi.scanNetworks();
  String json = "[";
  if (n == 0) {
      return "{}";
  }
  else {
      for (int i = 0; i < n; ++i) {
          // Print SSID and RSSI for each network found
          json += "{\"ssid\":\"" +String(WiFi.SSID(i))+ "\",\"rssi\":\""+String(WiFi.RSSI(i))+"\",\"encr\":" + ((WiFi.encryptionType(i) == ENC_TYPE_NONE)?"false":"true") + "}";
          if (i+1 < n) {
              json += ",";
          }
          delay(10);
      }
      json += "]";
  }
  return json;
}

void TFNetwork::InitUdp() {
	udpsrv.begin(TFNETWORK_UDPPORT);
	 #ifdef TF_DEBUG
    Serial.print("[TFNetwork] Udp listening on ");
    Serial.println(TFNETWORK_UDPPORT);
	#endif
}

void TFNetwork::InitTcp() {
	#if defined(TFNETWORK_TCPSERVER) && TFNETWORK_TCPSERVER == 1
	tcpsrv = new WiFiServer(TFNETWORK_TCPPORT);
	tcpsrv->begin();
	tcpsrv->setNoDelay(true);
	#ifdef TF_DEBUG
    Serial.print("[TFNetwork] TCP server listening on ");
    Serial.println(TFNETWORK_TCPPORT);
	#endif
	#endif
}

void TFNetwork::OnMsg(String (*onMsgCallback)(char*)) {
	_onMsgCallback = onMsgCallback;
}

void TFNetwork::OnDeviceInfo(String (*onDevInfo)(void)) {
	_onDeviceInfo = onDevInfo;
}

void TFNetwork::UdpWorker() {
	char data[255];
	int size = udpsrv.parsePacket();
	if (size){
		int len = udpsrv.read(data, TFNETWORK_MAX_PACKET_SIZE);
		if (len > 0 && len < TFNETWORK_MAX_PACKET_SIZE) {
            data[len] = 0;
        }else if (len >= TFNETWORK_MAX_PACKET_SIZE) {
           return;
        }
        
        if (String(data) == TFNETWORK_UDPBROADCAST) {
			udpsrv.beginPacket(udpsrv.remoteIP(), udpsrv.remotePort());
            udpsrv.print(_onDeviceInfo()); 
            udpsrv.endPacket();
        }else {
			//need auth?
			if (_cfg->getBool("AUTH") == true) {
				int clientid = _isRegistered(udpsrv.remoteIP(), udpsrv.remotePort());
				//if not authenticated
				if (clientid == -1) {
					udpsrv.beginPacket(udpsrv.remoteIP(), udpsrv.remotePort());
					//is auth packet?
					if (strstr(data, "+AUTH=") != NULL) {
						 char *seq = strtok(data,"=");
						 seq = strtok(NULL,"=");
						 if (seq != NULL && String(seq) == _cfg->getString("PWD")) {
							 _registerClient(udpsrv.remoteIP(), udpsrv.remotePort(), 255);
							 udpsrv.print("+ok=+AUTH");
							 udpsrv.endPacket();
							 return;
						 }
					}
					
					udpsrv.print("-err=+AUTH");
					udpsrv.endPacket();
					return;
				}else {
				   TFNetwork::_updateClientTimestamp(clientid);
				}
			}else {
				if (TFNETWORK_TRACK_CLIENTS) {
					int clientid = _isRegistered(udpsrv.remoteIP(), udpsrv.remotePort());
					if (clientid == -1) { _registerClient(udpsrv.remoteIP(), udpsrv.remotePort(), 255); }
					else {
						TFNetwork::_updateClientTimestamp(clientid);
					}
				}
			}

			String response = _onMsgCallback(data);
			
			if (response.length()>0) {
				udpsrv.beginPacket(udpsrv.remoteIP(), udpsrv.remotePort());
				udpsrv.print(response);
				udpsrv.endPacket();
			}
        }
        TFNetwork::_checkClients();
	}
};

void TFNetwork::TcpWorker() {
	#if defined(TFNETWORK_TCPSERVER) && TFNETWORK_TCPSERVER == 1
	if (tcpsrv->hasClient()){
		bool found = false;
		for(int i = 0; i < TFNETWORK_MAX_CLIENTS; i++){
			if (!tcpclients[i] || !tcpclients[i].connected()){
				if(tcpclients[i]) tcpclients[i].stop();
				tcpclients[i] = tcpsrv->available();
				#ifdef TF_DEBUG
					Serial.print("[TFNetwork] New tcp client: "); Serial.println(i);
				#endif
			  found = true;
			  break;
			}
		}
		if (!found) {
			//no free/disconnected spot so reject
			WiFiClient tcpclient = tcpsrv->available();
			tcpclient.println("-err=MAX_CONN");
			tcpclient.stop();
		}
	}
	
	for(int i = 0; i < TFNETWORK_MAX_CLIENTS; i++){
		if (tcpclients[i] && tcpclients[i].connected()){
			int size = tcpclients[i].available();
			if(size>0){
				while (size>0) {
					char c = tcpclients[i].read();
					if (tcpbuffer[i].length() >= TFNETWORK_MAX_PACKET_SIZE) {
						tcpbuffer[i] = "";
						size = 0;
					}else {
						//cr or lf
						if (c != 0x0a && c != 0x0d) {
							tcpbuffer[i] += c;
						}else {
							if (tcpbuffer[i].length()>0) {
								//need auth?
								if (_cfg->getBool("AUTH") == true) {
									int clientid = _isRegistered(tcpclients[i].remoteIP(), tcpclients[i].remotePort());
									if (clientid == -1) {
										//is auth packet?
										if (strstr(&tcpbuffer[i][0], "+AUTH=") != NULL) {
											 char *seq = strtok(&tcpbuffer[i][0],"=");
											 seq = strtok(NULL,"=");
											 if (seq != NULL && String(seq) == _cfg->getString("PWD")) {
												 _registerClient(tcpclients[i].remoteIP(), tcpclients[i].remotePort(), i);
												 tcpclients[i].println("+ok=+AUTH");
												 tcpbuffer[i] = "";
												 break;
											 }
										}
										tcpclients[i].println("-err=AUTH");
										tcpbuffer[i] = "";
										break;
									}else {
										TFNetwork::_updateClientTimestamp(clientid);
									}
								}else {
									if (TFNETWORK_TRACK_CLIENTS) {
										int clientid = _isRegistered(tcpclients[i].remoteIP(), tcpclients[i].remotePort());
										if (clientid == -1) {  _registerClient(tcpclients[i].remoteIP(), tcpclients[i].remotePort(), i); }
										else {
											TFNetwork::_updateClientTimestamp(clientid);
										}
									}
								}
								
								String response = _onMsgCallback(&tcpbuffer[i][0]);
								if (response.length()>0) {
									tcpclients[i].println(response);
								}
							}
						}
					}
					size--;
				}
			}
		}else {
			if(tcpclients[i]) tcpclients[i].stop();
		}
	}
	#endif
};

int TFNetwork::_isRegistered(IPAddress remoteIp, uint16_t port) {
	for (int i = 0; i < TFNETWORK_MAX_CLIENTS; i++) {
		if (clients[i].ip == remoteIp && clients[i].port == port)  {
			return i;
		}
	}
	return -1;
};

void TFNetwork::_updateClientTimestamp(int clientid) {
	if (clientid > -1 && clientid < TFNETWORK_MAX_CLIENTS) {
		clients[clientid].lastaction = millis();
	}
};

void TFNetwork::sendToClients(String data) {
	for (int i = 0; i < TFNETWORK_MAX_CLIENTS; i++) {
		if (clients[i].lastaction > millis()-15000) {
			if (clients[i].type == 255) {
				udpsrv.beginPacket(clients[i].ip,clients[i].port);
				udpsrv.print(data); 
				udpsrv.endPacket();
			}
		}else {
			//TODO: TCP
		}
	}
};

int TFNetwork::_registerClient(IPAddress remoteIp, uint16_t port, uint8_t type) {
	int clientid = -1; 
	unsigned long lastaction = millis();
	
	for (int i = 0; i < TFNETWORK_MAX_CLIENTS; i++) {
		if (clients[i].ip == remoteIp && clients[i].port == port) {
			 clients[i].type = type;
			 clients[i].lastaction = lastaction;
			 return i;
		}
	}
	
	for (int i = 0; i < TFNETWORK_MAX_CLIENTS; i++) {
		if (clients[i].type == 0) {
			clientid = i;
			break;
		}else {
			if (clients[i].lastaction + 15000 < lastaction) {
				clientid = i;
				break;
			}
		}
	}
	
	if (clientid > -1) {
		clients[clientid].type = type;
		clients[clientid].ip = remoteIp;
		clients[clientid].port = port;
		clients[clientid].lastaction = lastaction;
		return clientid;
	}
	
	return -1;
}



void TFNetwork::_checkClients() {
	unsigned long oldclients = millis() - 60000;
	for (int i = 0; i < TFNETWORK_MAX_CLIENTS; i++) {
		if (clients[i].type != 0 && clients[i].lastaction < oldclients) {
			clients[i].type = 0;
			clients[i].ip = {0,0,0,0};
			clients[i].port = 0;
			clients[i].lastaction = 0;
		}
	}
};

void TFNetwork::_removeClient(IPAddress remoteIp, uint16_t port) {
	for (int i = 0; i < TFNETWORK_MAX_CLIENTS; i++) {
		if (clients[i].ip == remoteIp && clients[i].port == port) {
			clients[i].type = 0; clients[i].ip = {0,0,0,0}; clients[i].lastaction = 0;
		}
	}
};
void TFNetwork::_removeClient(int clientid) {
	if (clientid > -1 && clientid < TFNETWORK_MAX_CLIENTS) {
		clients[clientid].type = 0; clients[clientid].ip = {0,0,0,0}; clients[clientid].lastaction = 0;
	}
};
void TFNetwork::_removeClients() {
	for (int i = 0; i < TFNETWORK_MAX_CLIENTS; i++) {
		clients[i].type = 0; clients[i].ip = {0,0,0,0}; clients[i].lastaction = 0;
	}
};

int TFNetwork::parsePacket(char* packet) {
	paramCount = 0;
	command = "";
	paramIndex = 0;

	char* seq = strtok(packet,"=");
	for (int i = 0; i < TFNETWORK_MAX_PARAM_COUNT; i++) {
		params[i] = "";
	}
	
	if (seq != NULL) {
		command = String(seq);
		seq = strtok(NULL,"=");
		if (seq != NULL) {
			seq = strtok(seq,",");
			while (seq != NULL && paramCount < TFNETWORK_MAX_PARAM_COUNT) {
				params[paramCount] = String(seq);
				paramCount++;
				seq = strtok(NULL,",");
			}
		}else {
			command = command.substring(0,strlen(&command[0])-1);
		}
		return paramCount;
	}else {
		return -1;
	}
};

bool TFNetwork::isCommand(const char* cmd) {
	if (strcmp(&command[0],cmd)==0) {  return true; }
	return false;
};

int TFNetwork::getParam(void) {
	return getParam(paramIndex);
};

int TFNetwork::getParam(uint8_t index) {
	if (index >= 0 && index < TFNETWORK_MAX_PARAM_COUNT) {
		paramIndex = index+1;
		return atoi(&params[index][0]);
	}
	return -1;
};

RGB TFNetwork::getParamRGB() {
	return getParamRGB(paramIndex);
};

RGB TFNetwork::getParamRGB(uint8_t index) {
	RGB c = {0,0,0};
	c.r = checkRGB(getParam(index));
	c.g = checkRGB(getParam(index+1));
	c.b = checkRGB(getParam(index+2));
	paramIndex = index+3;
	return c;
};

uint8_t TFNetwork::checkRGB(int c) {
	if (c>=0&&c<=255) { return c; }
	return 0;
};

String TFNetwork::getParamString() {
	return getParamString(paramIndex);
};

String TFNetwork::getParamString(uint8_t index) {
	paramIndex = index+1;
	if (index >= 0 && index < TFNETWORK_MAX_PARAM_COUNT) {
		return String(params[index]);
	}
	return "";
};

char* TFNetwork::getCommand() {
	return &command[0];
};
