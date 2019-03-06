#ifndef TFHEADER_H
#define TFHEADER_H
#include "Arduino.h"
#include <ESP8266WiFi.h> //for ipAddress

//firmware type and version
const String FW_VER    = "tflight-1.1";

//debug mode
#define TF_DEBUG 1

//TfConfig
//tfconfig max key length
#define TFCONFIG_ITEM_KLEN 5
//tfconfig max value lenght
#define TFCONFIG_ITEM_VLEN 50

//TFNetwork
//tcp server enable/disable; 0=disabled
#define TFNETWORK_TCPSERVER 1
//tcp webserver (depends on TFNETWORK_TCPSERVER)
#define TFNETWORK_WEBSERVER 1

//search string; udp broadcast message
const String TFNETWORK_UDPBROADCAST = "TECHFACTORYESPBRCAST";
//default tcp and udp port
const unsigned int TFNETWORK_TCPPORT = 80;
const unsigned int TFNETWORK_UDPPORT = 42910;
//max single packet size (for udp)
const int TFNETWORK_MAX_PACKET_SIZE = 255;
//for webserver (tcp)
const int TFNETWORK_MAX_HEADER_SIZE = 1000;
const int TFNETWORK_MAX_BODY_SIZE 	 = 255;

//parameter parser ( eg. +COMMAND=Param1,Param2,Param3 ..... )
//max parameter count
const uint8_t TFNETWORK_MAX_PARAM_COUNT = 20;
//max clients at same time
const byte TFNETWORK_MAX_CLIENTS 		= 3;
//default ip address and channel for AP mode
const uint8_t TFNETWORK_WIFI_CH 		= 10;
const IPAddress TFNETWORK_WIFI_ADDR    = IPAddress(192, 168, 99, 1);
const IPAddress TFNETWORK_WIFI_GATEWAY = IPAddress(192, 168, 99, 1);
const IPAddress TFNETWORK_WIFI_SUBNET  = IPAddress(255, 255, 255, 0);
//STA mode settings (dhcp or fixed ip) moved to DefaultConfig (TFConfig)

//TFLightAnim
//anim frame time in ms
#define TFLIGHT_FRAME_TIME 30
//max anim slots
#define TFLIGHT_ANIM_MAX_SLOT 3
//max color palette count
#define TFLIGHT_MAX_COLORS 6
//color palette size
#define TFLIGHT_EQCOLORPALETTE_SIZE 4
//max programs can be saved on SPIFFS
#define TFLIGHTANIM_SAVED_PROG_COUNT 30
#define TFLIGHTANIM_MAX_PIXEL_COUNT 50

//internal spectrum analyzer support (with msgeq7)
//STROBE_PIN AND RESET_PIN
#define TFEQ_MSGEQ7_STROBE 4
#define TFEQ_MSGEQ7_RESET 5

#define TFNETWORK_NEUTRAL_COMMANDS 30
static const char* PROGMEM Network_neutral_commands[TFNETWORK_NEUTRAL_COMMANDS] = {
	"+INFO", "+AUTH", "+STATUS", "+DEV-INFO", "+NET-INFO", "+ANIM-INFO", 
	//light control
	"+RGB", "+RGBA", "+BRIGHT", "+PIXGRP", "+PIXMULTI", "+PIX", 
	//mode
	"+START", "+STOP", "+MODE", "+WIFI-SCAN", 
	//eq
	"+EQ-INFO", "+EQAUTO", "+EQ", "+EQCOLORS-GET", "+EQCOLORS-SET", "+EQTYPE", "+EQRANGE"
	//
	"+ANIM-STATUS", "+ANIM-LOAD", "+ANIM-STOP", 
	//programs
	"+PLIST", "+PSTORE", "+PREM", "+PSTORE-START"
};

#define TFTIMER_TIMERS 10
#define TFTIMER_UPD_INTERVAL 2160000

#endif // TFHEADER_H

