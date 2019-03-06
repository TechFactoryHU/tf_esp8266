#ifndef TFNETWORK_H
#define TFNETWORK_H
#include "Arduino.h"
#include <ESP8266WiFi.h> 
#include <WiFiUdp.h>
#include "tfheader.h"
#include "tfcustomcfg.h"
#include "helpers.h"

struct TFNETWORK_CLIENT {
	uint8_t type;
	IPAddress ip;
	uint16_t port;
	
	//flags
	unsigned char attrs;
	//0->isTcp
	//1->isHttp
	//2->isUdp
	//3->isValid
	//4->isAuthenticated (user)
	//5->isAdmin
	//6->http_header_lf
	//7->http_header_end
	
	unsigned long lastaction; 		
	unsigned int counter;			
	unsigned int session_id; 		//for http auth	//2byte
	unsigned int content_length; 	//for http _ max 65,535 bytes
	char request_uri[15];			
	uint8_t request_method;			
		
};

//default client for serial commands (admin)
const TFNETWORK_CLIENT serialClient = {
  254, IPAddress(0,0,0,0), 0, 32
};

struct TFNETWORK_HTTP_REQUEST {
	uint8_t method; //1=GET,2=POST
	char request_uri[10];
};

typedef std::function<String(char*)> TFRequestHandler;
		
class TFNetwork {
	private:
		bool softAp;
		bool staConnected;
		unsigned long sta_last_check;
		TFCustomCfg *_cfg;
		char hwnr[18];
		
		int _activeclientid;
		WiFiUDP udpsrv;
		#if defined(TFNETWORK_TCPSERVER) && TFNETWORK_TCPSERVER == 1
		WiFiServer *tcpsrv;
		WiFiClient tcpclients[TFNETWORK_MAX_CLIENTS];
		String tcpbuffer[TFNETWORK_MAX_CLIENTS];
		bool (*_onHTTPRequest)(uint8_t, char*, char*, WiFiClient);
		#endif
		
		String (*_onMsgCallback)(char*, TFNETWORK_CLIENT);
		String (*_onDeviceInfo)(TFNETWORK_CLIENT);
		
		TFNETWORK_CLIENT clients[TFNETWORK_MAX_CLIENTS];
		void resetHTTPClientData(int clientId);
		int _isRegistered(IPAddress remoteIp, uint16_t port, uint8_t type = 0);
		int _registerClient(IPAddress remoteIp, uint16_t port, byte type);
		void _updateClientTimestamp(int clientId);
		void _removeClient(IPAddress remoteIp, uint16_t port);
		void _removeClient(int clientId);
		void _removeClients();
		void _checkClients();
		
		uint8_t paramIndex;
		String command;
		uint8_t paramCount;
		String params[TFNETWORK_MAX_PARAM_COUNT];
		
		
	public:
		TFNetwork(TFCustomCfg *config);
		void begin(void);
		inline char* getHWNR(void) { return hwnr; }
		
		void SetAP(String APssid, String APpassword, bool update);
		void StartAp(void);
		void StartAp(String ssid, String password);
		void StopAp();
		void StaConnect(String ssid, String password);
		void StaDisconnect(void);
		String StaSSID(void);
		bool StaStatus(void);
		
		bool CheckSta(bool wait);
		bool ApStatus(void);
		String Scan();
		
		void InitUdp(void);
		void InitTcp(void);
		
		void OnMsg(String (*onMsgCallback)(char*,TFNETWORK_CLIENT));
		void OnDeviceInfo(String (*onDevInfo)(TFNETWORK_CLIENT));
		#if defined(TFNETWORK_TCPSERVER) && TFNETWORK_TCPSERVER == 1
		void OnHTTPRequest(bool (*onHTTPRequest)(uint8_t, char*, char*, WiFiClient));
		#endif
		
		void UdpWorker(void);
		void TcpWorker(void);
		
		bool auth(String user, String pwd, TFNETWORK_CLIENT client);
		int getAuthToken(void);
		int getCurrentClientId(void);
		
		void sendToClients(String data);
		
		int parsePacket(char* packet);
		char* getCommand();
		bool isCommand(const char* cmd);
		int getParam(void);
		int getParam(uint8_t index);
		
		RGB getParamRGB(void);
		RGB getParamRGB(uint8_t index);
		String getParamString(void);
		String getParamString(uint8_t index);
		uint8_t checkRGB(int c);
		
		bool ServeHTTPStaticFile(String file, WiFiClient client);
		bool ServeHTTPStaticFile(String file, String type, WiFiClient client);
		void HTTPResponse(int code, WiFiClient client);
		void HTTPResponse(int code, String msg, String content_type, WiFiClient client);
		
};

#endif // TFNETWORK_H