#ifndef TFHEADER_H
#define TFHEADER_H
#include "Arduino.h"

/*
*	Debug mode 
*/
#define TF_DEBUG 1

//tfconfig max key length
#define TFCONFIG_ITEM_KLEN 10
//tfconfig max value lenght
#define TFCONFIG_ITEM_VLEN 100

//tcp server enable/disable; 0=disabled
#define TFNETWORK_TCPSERVER 1
//track clients (last action, ip, port for both udp/tcp)
const bool TFNETWORK_TRACK_CLIENTS = true;
//search string; udp broadcast message
const String TFNETWORK_UDPBROADCAST = "TECHFACTORYESPBRCAST";
//default tcp and udp port
const unsigned int TFNETWORK_TCPPORT = 9000;
const unsigned int TFNETWORK_UDPPORT = 42910;
//max single packet size (both tcp and udp)
const int TFNETWORK_MAX_PACKET_SIZE = 255;
//parameter parser ( eg. +COMMAND=Param1,Param2,Param3 ..... )
//max parameter count
const uint8_t TFNETWORK_MAX_PARAM_COUNT = 20;
//max clients at same time
const byte TFNETWORK_MAX_CLIENTS = 3;

#endif // TFHEADER_H

