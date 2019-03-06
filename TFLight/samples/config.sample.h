/*
*	Default TFLight config
*	Copy this file to TFLight root folder and rename to config.h
*/

//max key length = 3, max value = 50 defined in tfheader.h
TFConfigItem DefaultConfig[33] = {
  //device name
  {"DN", "TFLIGHT-DEVICE"},
  //ap ssid
  {"AP", "TFLIGHT"},
  //User password (Used for AP & User authentication for controlling devices)
  {"PWD", "A1234B5678"},
  //require user auth for controlling lights
  {"AUT", "0"},
  //require admin auth for configuration (dev name, wifi config, ip config, etc...)
  {"ADM", "0"},
  //Admin password 
  {"APW", "TF4DM1N"},
  //disable AP on STA mode (wifi client mode)
  {"AST", "0"},
  //change pwm frequency globally (0=unchanged)
  {"PWM", "0"},
  {"NIP", "0"}, 			      //0=dhcp, 1=fixip
  {"NAD", "192.168.10.50"}, 	  //network address
  {"NSM", "255.255.255.0"}, 	  //network subnet mask
  {"NGW", "192.168.10.1"}, 		  //network gateway
  {"NDN", "192.168.10.1"}, 		  //network dns-server 
  {"NCH", "4"}, 			      //ap wifi channel
  {"NWV", "0"}, 			      //webadmin version
  {"NTS", "http://techfactory.hu/esp/time/"}, //time server
  {"TIM", "1"},								  //timer enabled
  {"BRC", "1"}, 							  //discoverable by udp broadcast
  
  //default_start_action per device
  //devices defined in hardware_config.h
  {"SD0", ""},
  {"SD1", ""},
  {"SD2", ""},
  {"SD3", ""},
  
  //eq color palette & colors
  {"EQC", "6"},
  {"EQ0", "5,50,150;0,0,255;100,0,255;255,0,100"},
  {"EQ1", "0,180,30;0,255,0;0,255,100;0,255,255"},
  {"EQ2", "255,0,50;255,50,50;255,100,50;255,0,0"},
  {"EQ3", "0,255,50;50,255,50;100,255,70;0,255,0"},
  {"EQ4", "0,50,255;50,50,255;100,70,255;0,0,255"},
  {"EQ5", "100,100,100;200,100,200;255,100,255;255,0,0"},
  
  //EQ Parameters (analog in range, drop time/level, defined in tfeq.h)
  {"EQP", "150,1024,30,2,10"} 
};
TFCustomCfg Config("config.dat", 33, DefaultConfig);
