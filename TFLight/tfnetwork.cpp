#include <ESP8266WiFi.h> 
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>
#include <MD5Builder.h>
#include "tfheader.h"
#include "tfcustomcfg.h"
#include "tfnetwork.h"
#include "helpers.h"

TFNetwork::TFNetwork(TFCustomCfg *config) {
	_cfg = config;
	softAp 		 = false;
	staConnected = false;
	sta_last_check = millis();
	paramIndex = 0;
}

void TFNetwork::begin() {
	uint8_t  MAC_softAP[6]; 
	WiFi.softAPmacAddress(MAC_softAP);
	String _hwnr;
	for (int i = 0; i < sizeof(MAC_softAP); ++i){
	  _hwnr += String( MAC_softAP[i], HEX);
	}
	_hwnr.toCharArray(hwnr,18);
	_hwnr="";
	
	WiFi.hostname(_cfg->getString("AP"));    
	IPAddress addr;
	IPAddress gw;
	IPAddress subnet;
	IPAddress dns;
	
	WiFi.config(addr,gw,subnet);
	 
    if (_cfg->exists("NIP") && _cfg->getBool("NIP")==true) {
		#ifdef TF_DEBUG 
			Serial.println("[TFNetwork] Fixed ip mode ");
		#endif
		if (_cfg->exists("NAD") && _cfg->exists("NSM") && _cfg->exists("NGW")) {
			if (addr.fromString(_cfg->getString("NAD"))) {
				if (gw.fromString(_cfg->getString("NGW"))) {
					if (subnet.fromString(_cfg->getString("NSM"))) {
						 #ifdef TF_DEBUG 
							Serial.print("[TFNetwork] Setting up fixed ip for STAtion mode = ");
							Serial.println(_cfg->getString("NAD"));
						#endif
						//WiFi.config(addr, subnet, gw);
						if (dns.fromString(_cfg->getString("NDN"))) {
							WiFi.config(addr, dns, gw, subnet);
							#ifdef TF_DEBUG 
							Serial.print("[TFNetwork] DNS = ");
							Serial.println(_cfg->getString("NDN"));
							#endif
						}else {
							#ifdef TF_DEBUG 
							Serial.print("[TFNetwork] DNS = ");
							Serial.println(gw);
							#endif
							WiFi.config(addr, gw, gw, subnet);
						}
					}else {
						#ifdef TF_DEBUG 
							Serial.print("[TFNetwork] Cant parse ip address (subnet) = ");
							Serial.println(_cfg->getString("NSM"));
						#endif
					}
				}else {
					#ifdef TF_DEBUG 
						Serial.print("[TFNetwork] Cant parse ip address (gw) = ");
						Serial.println(_cfg->getString("NGW"));
					#endif
				}
			}else {
				#ifdef TF_DEBUG 
					Serial.print("[TFNetwork] Cant parse ip address (addr) = ");
					Serial.println(_cfg->getString("NAD"));
				#endif
			}
		}
	}else {
		#ifdef TF_DEBUG 
			Serial.println("[TFNetwork] Dynamic ip mode (dhcp)");
		#endif
	}
	
	WiFi.mode(WIFI_AP_STA);
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
	if (_cfg->exists("AP") && _cfg->exists("PWD")) {
		TFNetwork::StartAp(_cfg->getString("AP"), _cfg->getString("PWD"));
	}
}

void TFNetwork::StartAp(String ssid, String password) {
	uint8_t ch = TFNETWORK_WIFI_CH;
	if (_cfg->exists("NCH") && _cfg->getInt("NCH") > 0) {
		ch = _cfg->getInt("NCH");
	}
	if (!softAp) {
		if (WiFi.softAPConfig(TFNETWORK_WIFI_ADDR, TFNETWORK_WIFI_GATEWAY, TFNETWORK_WIFI_SUBNET)) {
			if (WiFi.softAP(ssid.c_str(), password.c_str(), ch, 0)) {
				softAp = true;
			}
		}
	}
}

void TFNetwork::StopAp() {
	softAp = false;
	WiFi.softAPdisconnect(true);
}

bool TFNetwork::ApStatus(){
	return softAp;
}

void TFNetwork::StaConnect(String ssid, String password) {
	WiFi.disconnect();
    #ifdef TF_DEBUG 
        Serial.println("Station connecting to "+ssid);
    #endif
    WiFi.begin(ssid.c_str(), password.c_str());
}

void TFNetwork::StaDisconnect(){
	//WiFi.persistent(false);
	WiFi.disconnect(true);
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
                 if (_cfg->getBool("AST") == true) { TFNetwork::StopAp(); }
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
                delay(100);
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
	  String wnet;
      for (int i = 0; i < n; ++i) {
          // Print SSID and RSSI for each network found
          wnet = "{\"ssid\":\"" +String(WiFi.SSID(i))+ "\",\"rssi\":\""+String(WiFi.RSSI(i))+"\",\"encr\":" + ((WiFi.encryptionType(i) == ENC_TYPE_NONE)?"0":"1") + "}";
          if (json.length() + wnet.length() < 253) {
			  if (json.length()>1) {
				json += ",";
			  }
			  json += wnet;
          }else { break; }
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

#if defined(TFNETWORK_TCPSERVER) && TFNETWORK_TCPSERVER == 1
void TFNetwork::OnHTTPRequest(bool (*onHTTPRequest)(uint8_t, char*, char*, WiFiClient)) {
	_onHTTPRequest = onHTTPRequest;
}
#endif

void TFNetwork::OnMsg(String (*onMsgCallback)(char*, TFNETWORK_CLIENT)) {
	_onMsgCallback = onMsgCallback;
}

void TFNetwork::OnDeviceInfo(String (*onDevInfo)(TFNETWORK_CLIENT)) {
	_onDeviceInfo = onDevInfo;
}

bool TFNetwork::auth(String u, String p, TFNETWORK_CLIENT client) {
	bool admin = false;
	MD5Builder _md5;
	_md5.begin();
	String authstr;			 
			 
	if (u == "admin") {
		authstr = String(sta_last_check)+_cfg->getString("APW");	
		admin = true;		
	}else {
		authstr = String(sta_last_check)+_cfg->getString("PWD");
	}
	
	_md5.add(authstr);
	_md5.calculate();
	
	if (p == _md5.toString()) {
		int client_id;
		for (int i = 0; i < TFNETWORK_MAX_CLIENTS; i++) {
			if (clients[i].ip == client.ip && clients[i].port == client.port && clients[i].type == client.type) {
				clients[i].attrs = TFHb_set_attr(clients[i].attrs, admin ? 5 : 4, true);
				return true;
			}
		}
		return true;
	}
	
	return false;
}

int TFNetwork::getAuthToken() {
	return sta_last_check;
}

int TFNetwork::getCurrentClientId() {
	return _activeclientid;
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
        
        //lf
        if (data[len-1] == 0x0a) { data[len-1] = 0; }
        _activeclientid = _isRegistered(udpsrv.remoteIP(), udpsrv.remotePort(), 255);
        
        if (String(data) == TFNETWORK_UDPBROADCAST) {
			if (_cfg->exists("BRC") && _cfg->getBool("BRC")) {
				udpsrv.beginPacket(udpsrv.remoteIP(), udpsrv.remotePort());
				TFNETWORK_CLIENT tmpc = {};
				if (_activeclientid>-1) { tmpc = clients[_activeclientid]; }
				else {
					tmpc.ip 	= udpsrv.remoteIP();
					tmpc.port 	= udpsrv.remotePort();
				}
				udpsrv.print(_onDeviceInfo(tmpc)); 
				udpsrv.endPacket();
            }
        }else {
			if (_activeclientid == -1) {
				if (_registerClient(udpsrv.remoteIP(), udpsrv.remotePort(), 255) == -1) {
					udpsrv.print(F("-err=Out_of_slots"));
					#ifdef TF_DEBUG
					Serial.println("[Network] udp max_clients reached!");
					#endif
				}
			}else { _updateClientTimestamp(_activeclientid); }

			clients[_activeclientid].counter++;
			String response = _onMsgCallback(data, clients[_activeclientid]);
			if (response.length()>0) {
				udpsrv.beginPacket(udpsrv.remoteIP(), udpsrv.remotePort());
				udpsrv.print(response);
				udpsrv.endPacket();
			}
        }
	}
	_checkClients();
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
				if (_registerClient(tcpclients[i].remoteIP(), tcpclients[i].remotePort(), i) == -1) {
					tcpclients[i].stop();
					#ifdef TF_DEBUG
					Serial.print("[TFNetwork] Cant register "); Serial.println(tcpclients[i].remoteIP());
					#endif
				}else {
					found = true;
					tcpbuffer[i] = "";
				}
				break;
			}
		}
		
		if (!found) {
			//no free/disconnected spot so reject
			WiFiClient tcpclient = tcpsrv->available();
			tcpclient.stop();
			#ifdef TF_DEBUG
				Serial.print("[TFNetwork] Server is full ... "); Serial.println(tcpclient.remoteIP());
			#endif
		}
	}

	char *seq;
	for(int i = 0; i < TFNETWORK_MAX_CLIENTS; i++){
		char c;
		
		if (tcpclients[i] && tcpclients[i].connected()){
			_activeclientid = _isRegistered(tcpclients[i].remoteIP(), tcpclients[i].remotePort());
			
			if (_activeclientid == -1) {
				#ifdef TF_DEBUG
					Serial.print("[TFNetwork] Client not registered ... ");
					Serial.print(tcpclients[i].remoteIP());
					Serial.print(":");
					Serial.println(tcpclients[i].remotePort());
				#endif
				tcpbuffer[i] = "";
				tcpclients[i].stop(); 
				continue;
			}

			while (tcpclients[i].available()) {
				c = tcpclients[i].read();
				if (tcpbuffer[i].length() >= TFNETWORK_MAX_PACKET_SIZE) {
					#ifdef TF_DEBUG
						Serial.print("[TFNetwork] Max packet size reached ... ");
						Serial.println(tcpclients[i].remoteIP());
					#endif
					tcpbuffer[i] = "";
					if (TFHb_get_attr(clients[_activeclientid].attrs,1)) {
						HTTPResponse(400,tcpclients[i]);
					}else {
						tcpclients[i].println("-err=MAX_PACKET_SIZE");
					}
					tcpclients[i].stop();
					break;
				}else {
					if (TFHb_get_attr(clients[_activeclientid].attrs,1) && TFHb_get_attr(clients[_activeclientid].attrs, 7)) {
						if (clients[_activeclientid].content_length > 0) {
							tcpbuffer[i] += c;
							clients[_activeclientid].content_length--;
							continue;
						}
					}
	
					//not cr and lf
					if (c != 0x0d && c != 0x0a) {
						tcpbuffer[i] += c;
					//is cr or lf
					}else {
						#if defined(TFNETWORK_WEBSERVER) && TFNETWORK_WEBSERVER == 1
						//isHttp
						if(TFHb_get_attr(clients[_activeclientid].attrs, 1)) {
							if (c == 0x0a) {
								//second lf, http headers end
								if(TFHb_get_attr(clients[_activeclientid].attrs, 6)) {
									#ifdef TF_DEBUG
										Serial.println("[HTTP] Got all headers ... ");
									#endif
									if (clients[_activeclientid].request_method > 1 && clients[_activeclientid].content_length == 0) {
										HTTPResponse(400,tcpclients[i]);
										tcpclients[i].stop();
										tcpbuffer[i] = "";
										break;
									}
									
									clients[_activeclientid].attrs = TFHb_set_attr(clients[_activeclientid].attrs, 7, true);
									clients[_activeclientid].counter = 0;
									
									if (clients[_activeclientid].request_method == 2 && String(clients[_activeclientid].request_uri) == "/api") {
										clients[_activeclientid].attrs = TFHb_set_attr(clients[_activeclientid].attrs, 3, true); //set isValid flag
									}
										
									//tcpbuffer[i] = "";
									continue;
								}
								//isHttp header 
								else {
									#ifdef TF_DEBUG
										Serial.println("RQ Method: "+String(clients[_activeclientid].request_method));
										Serial.println(tcpbuffer[i]);
									#endif
									
									//watch only for content-length (if request_method is NOT GET)
									if (clients[_activeclientid].request_method > 1) {
										if (tcpbuffer[i].startsWith("Content-Length:")) {
											seq = strtok(&tcpbuffer[i][0],": ");
											seq = strtok(NULL," ");
											if (seq != NULL) {
												clients[_activeclientid].content_length = atoi(seq);
												#ifdef TF_DEBUG
													Serial.println("Content-Length: "+String(clients[_activeclientid].content_length));
												#endif
											}
										}	
									}	
									if (clients[_activeclientid].counter >= TFNETWORK_MAX_HEADER_SIZE) {
										tcpbuffer[i] = "";
										tcpclients[i].println("-err=MAX_PACKET_SIZE");
										tcpclients[i].stop();
									}else {
										clients[_activeclientid].counter+=tcpbuffer[i].length();
										clients[_activeclientid].attrs = TFHb_set_attr(clients[_activeclientid].attrs, 6, true);
									}
									tcpbuffer[i] = "";
								}
								
							}else {
								if (tcpbuffer[i].length()) {
									clients[_activeclientid].attrs = TFHb_set_attr(clients[_activeclientid].attrs, 6, false);
								}
							}
						}
						#endif
						
						if (tcpbuffer[i].length()>0) {
							//connection protocol not determined yet 
							if (!TFHb_get_attr(clients[_activeclientid].attrs,0)) {
								#ifdef TF_DEBUG
									Serial.print("[TFNetwork] Connection protocol is ");
								#endif
								
								//single command, simple "telnet" protocol
								if (String(tcpbuffer[i].charAt(0)) == "+") {
									clients[_activeclientid].attrs = TFHb_set_attr(clients[_activeclientid].attrs, 0, true); //set isTCP flag
									clients[_activeclientid].attrs = TFHb_set_attr(clients[_activeclientid].attrs, 3, true); //set isValid flag
									#ifdef TF_DEBUG
									Serial.println(" SIMPLE");
									#endif
								}
								#if defined(TFNETWORK_WEBSERVER) && TFNETWORK_WEBSERVER == 1
								//http
								else if(tcpbuffer[i].startsWith("GET") || tcpbuffer[i].startsWith("POST") || tcpbuffer[i].startsWith("PUT") ||  tcpbuffer[i].startsWith("DELETE")) {
									#ifdef TF_DEBUG
										Serial.println(" HTTP");
									#endif
									
									clients[_activeclientid].attrs = TFHb_set_attr(clients[_activeclientid].attrs, 0, true); //set isTCP flag
									clients[_activeclientid].attrs = TFHb_set_attr(clients[_activeclientid].attrs, 1, true); //set isHTTP flag
									
									//[GET|POST|PUT|DELETE] /requested_uri HTTP/1.1
									seq = strtok(&tcpbuffer[i][0]," ");
									seq = strtok(NULL," ");
									if (seq != NULL) {
										String(seq).toCharArray(clients[_activeclientid].request_uri,15); 
										clients[_activeclientid].request_method = tcpbuffer[i].startsWith("GET") ? 1 : 2;
										
										#ifdef TF_DEBUG
										Serial.print("[HTTP] ");
										Serial.print(clients[_activeclientid].request_method);
										Serial.print(" ");
										Serial.println(clients[_activeclientid].request_uri);
										#endif
									}
									tcpbuffer[i] = "";
								//unknown
								}
								#endif
								else {
									#ifdef TF_DEBUG
										Serial.print("[TFNetwork] Unknown protocol ... ");
										Serial.println(tcpclients[i].remoteIP());
									#endif
									tcpclients[i].println("-err=UNKNOWN_PROTOCOL");
									tcpclients[i].stop();
									break;
								}
							}
						}//tcpbuffer not empty
					}
				}
			}//while available
			
			
			//isValid
			if (TFHb_get_attr(clients[_activeclientid].attrs,3)) {
				if (tcpbuffer[i].length()>0) {
					String response = _onMsgCallback(&tcpbuffer[i][0],clients[_activeclientid]);
					if (response.length()>0) {
						//if http 
						if (TFHb_get_attr(clients[_activeclientid].attrs,1)) {
							HTTPResponse(200, response, "text/plain", tcpclients[i]);
							tcpclients[i].stop();
						}else {
							tcpclients[i].println(response);
						}
					}
					_updateClientTimestamp(_activeclientid);
					resetHTTPClientData(_activeclientid);
				}
			}
			#if defined(TFNETWORK_WEBSERVER) && TFNETWORK_WEBSERVER == 1
			//http request...
			else if(TFHb_get_attr(clients[_activeclientid].attrs, 1)) {
				//headers received
				if(TFHb_get_attr(clients[_activeclientid].attrs, 7)) {
					bool rp = false;
					if (_onHTTPRequest != NULL) {
						if (_onHTTPRequest(clients[_activeclientid].request_method, clients[_activeclientid].request_uri, &tcpbuffer[i][0], tcpclients[i])) {
							_updateClientTimestamp(_activeclientid);
							rp = true;
						}
					}
					//this request is unhandled by callback fn
					if (!rp) {
						//GET
						if (clients[_activeclientid].request_method == 1) {
							if (String(clients[_activeclientid].request_uri) == "/") {
								ServeHTTPStaticFile("/www/index.html", tcpclients[i]);
							}else {
								ServeHTTPStaticFile("/www"+String(clients[_activeclientid].request_uri),tcpclients[i]);
							}
							
						}else if (clients[_activeclientid].request_method == 2) {
							HTTPResponse(404,tcpclients[i]);
							tcpclients[i].stop();
						}
						else {
							HTTPResponse(404,tcpclients[i]);
							tcpclients[i].stop();
						}
					}
					
					_updateClientTimestamp(_activeclientid);
					resetHTTPClientData(_activeclientid);
				}
			}
			#endif
			tcpbuffer[i] = "";
			
		}//is connected
		else {
			if(tcpclients[i]) tcpclients[i].stop();
		}
	}
	//check clients
	#endif
};

void TFNetwork::resetHTTPClientData(int clientId) {
	clients[clientId].attrs = 0;
	clients[clientId].request_method = 0;
	String("").toCharArray(clients[clientId].request_uri,15);
};

bool TFNetwork::ServeHTTPStaticFile(String file, String type, WiFiClient client) {
	if (file.startsWith("/")) {
		File f = SPIFFS.open(file, "r");
		if (f) {
		   client.print(F("HTTP/1.1 200 OK\r\n"));
		   client.print(F("Server: TechFactory/"));
		   client.print(FW_VER);
		   client.print(F("\r\n"));
		   //force keepalive
		   client.print(F("Connection: keepalive\r\n"));
		   client.print(F("Content-Type: "));
		   client.print(type);
		   client.print(F("\r\n"));
		   client.print(F("Content-Length: "));
		   client.print(String(f.size()));
		   client.print(F("\r\n"));
		   
		   client.print(F("\r\n"));
		   while (f.available()){
				client.print((char)f.read());
           }
           f.close();
		   return true;
		}
	}
	HTTPResponse(404,client);
	return false;
}

bool TFNetwork::ServeHTTPStaticFile(String file,  WiFiClient client) {
	String type;
	if (file.endsWith(".html") || file.endsWith(".htm")) {
		type = F("Content-Type: text/html; charset=utf-8");
	}
	else if (file.endsWith(".css")) {
		type = F("Content-Type: text/css");
	}
	else if (file.endsWith(".js")) {
		type = F("Content-Type: application/javascript");
	}
	else if (file.endsWith(".png")) {
		type = F("Content-Type: image/png");
	}
	else if (file.endsWith(".jpg")) {
		type = F("Content-Type: image/jpg");
	}else {
		type = F("Content-Type: application/octet-stream");
	}
	return ServeHTTPStaticFile(file, type, client);
};

void TFNetwork::HTTPResponse(int code, WiFiClient client) {
	HTTPResponse(code,"","", client);
};

void TFNetwork::HTTPResponse(int code, String msg, String content_type, WiFiClient client) {
	#ifdef TF_DEBUG
	Serial.print("[TFNetwork] HTTP Response ");
	Serial.println(code);
	#endif
	
	if (code==200) { 
		client.print(F("HTTP/1.1 200 OK\r\n"));
	}
	else if (code==301) { 
		client.print(F("HTTP/1.1 301 Moved permanelty\r\n"));
	}
	else if (code==400) { 
		client.print(F("HTTP/1.1 400 Bad Request!\r\n"));
	}
	else if (code==404) {
		client.print(F("HTTP/1.1 404 Not Found\r\n"));
		msg = F("404 - Not Found");
		content_type = F("plain/text; charset=utf-8");
	}
	else  { 
		client.print(F("HTTP/1.1 500 Internal server error? Just kidding. Its your fault...\r\n"));
	}
	
	client.print(F("Server: TechFactory/"));
	client.print(String(FW_VER));
	client.print(F("\r\n"));
	
	if (msg != NULL && msg.length()) {
		client.print(F("Content-Length: "));
		client.print(String(msg.length()));
		client.print(F("\r\n"));
		client.print(F("Content-Type: "));
		client.print(content_type);
		client.print(F("\r\n"));
	}
	
	client.print(F("Connection: close\r\n"));
	client.print(F("\r\n"));
	
	if (msg != NULL && msg.length()) {
		client.print(msg);
	}
};

int TFNetwork::_isRegistered(IPAddress remoteIp, uint16_t port, uint8_t type) {
	for (int i = 0; i < TFNETWORK_MAX_CLIENTS; i++) {
		//udp connection tracking
		if (type == 255 && clients[i].ip == remoteIp && clients[i].port == port)  {
			return i;
		//tcp connection tracking locked to ip only
		}else if(type < 255 && clients[i].ip == remoteIp) {
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
		if (clients[i].lastaction+15000 >= millis()) {
			if (clients[i].type == 255 || TFHb_get_attr(clients[i].attrs, 2)) {
				udpsrv.beginPacket(clients[i].ip,clients[i].port);
				udpsrv.print(data); 
				udpsrv.endPacket();
			}
		}else {
			if (clients[i].type>=0&&clients[i].type<=TFNETWORK_MAX_CLIENTS) {
				//if not http
				if (!TFHb_get_attr(clients[i].attrs, 1)) {
					if (tcpclients[clients[i].type] && tcpclients[clients[i].type].connected()) {
						tcpclients[clients[i].type].print(data);
					}
				}
			}
		}
	}
};

int TFNetwork::_registerClient(IPAddress remoteIp, uint16_t port, uint8_t type) {
	int clientid = -1; 
	for (int i = 0; i < TFNETWORK_MAX_CLIENTS; i++) {
		if (clients[i].ip == remoteIp && clients[i].port == port) {
			 clients[i].type = type;
			 clients[i].lastaction =  millis();
			 _activeclientid = i;
			 return i;
		}
	}
	
	for (int i = 0; i < TFNETWORK_MAX_CLIENTS; i++) {
		if (clients[i].type == 0) {
			clientid = i;
			break;
		}else {
			if (clients[i].lastaction + 30000 < millis()) {
				clientid = i;
				break;
			}
		}
	}
	
	if (clientid > -1) {
		clients[clientid].type = type;
		clients[clientid].ip = remoteIp;
		clients[clientid].port = port;
		clients[clientid].attrs = 0;
		clients[clientid].lastaction = millis();
		clients[clientid].counter = 0;
		_activeclientid = clientid;
		return clientid;
	}
	
	return -1;
}

void TFNetwork::_checkClients() {
	unsigned long oldclients = millis();
	for (int i = 0; i < TFNETWORK_MAX_CLIENTS; i++) {
		//isHttp
		if (TFHb_get_attr(clients[i].attrs, 1)) {
			//keepalive timeout
			if (clients[i].lastaction + 5000 < oldclients) {
				if (clients[i].type < TFNETWORK_MAX_CLIENTS && tcpclients[clients[i].type]) {
					if (tcpclients[clients[i].type]){
						#ifdef TF_DEBUG
						Serial.println("[TFNetwork] Keepalive: Timeout, closing connection.");
						#endif
						tcpclients[clients[i].type].stop();
					}
				}
			}
		}
		
		//udp & tcp clients session timeout
		if (clients[i].lastaction + 30000 < oldclients) {
			//tcp
			if (TFHb_get_attr(clients[i].attrs, 0)) {
				if (clients[i].type >= 0 && clients[i].type < TFNETWORK_MAX_CLIENTS) {
					if (tcpclients[clients[i].type]){
						tcpclients[clients[i].type].stop();
					}
				}
			}
			
			clients[i].type = 0;
			clients[i].ip = {0,0,0,0};
			clients[i].port = 0;
			clients[i].lastaction = 0;
			clients[i].attrs = 0;
			clients[i].content_length = 0;
			clients[i].request_method = 0;
			clients[i].session_id = 0;
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
			int l = strlen(&command[0]);
			if (command[l-1] == 0x0d) {
				command = command.substring(0,	l-1);
			}
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
