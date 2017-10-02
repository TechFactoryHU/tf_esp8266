#ifndef TFNETWORK_H
#define TFNETWORK_H
#include "../tfheader.h"
#include "Arduino.h"
#include <ESP8266WiFi.h> 
#include <WiFiUdp.h>
#include "tfcustomcfg.h"
#include "helpers.h"


struct TFNETWORK_CLIENT {
	uint8_t type;
	IPAddress ip;
	uint16_t port;
	unsigned long lastaction;
};

class TFNetwork {
	private:
		IPAddress APLocalIp;
		IPAddress APgateway;
		IPAddress APsubnet;
		bool softAp;
		bool staConnected;
		unsigned long sta_last_check;
		TFCustomCfg *_cfg;
		char hwnr[20];
		
		bool hasUdp;
		WiFiUDP udpsrv;
		
		bool hasTcp;
		#if defined(TFNETWORK_TCPSERVER) && TFNETWORK_TCPSERVER == 1
		WiFiServer *tcpsrv;
		WiFiClient tcpclients[TFNETWORK_MAX_CLIENTS];
		String tcpbuffer[TFNETWORK_MAX_CLIENTS];
		#endif
		
		String (*_onMsgCallback)(char*);
		String (*_onDeviceInfo)(void);
		
		TFNETWORK_CLIENT clients[TFNETWORK_MAX_CLIENTS];
		int _isRegistered(IPAddress remoteIp, uint16_t port);
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
		
		void OnMsg(String (*onMsgCallback)(char*));
		void OnDeviceInfo(String (*onDevInfo)(void));
		
		void UdpWorker(void);
		void TcpWorker(void);
		
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
		
};

#endif // TFNETWORK_H