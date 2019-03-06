//max key length = 3, max value = 50 defined in tfheader.h
TFConfigItem DefaultConfig[33] = {
  //device name
  {"DN", "TFLIGHT-DEVICE"},
  //ap ssid
  {"AP", "TFLIGHT01"},
  //User password (Used for AP & User authentication for controlling devices)
  {"PWD", "A1234B5678"},
  //Admin auth & password 
  {"ADM", "TF4DM1N"},
  {"APW", "TF4DM1N"},
  {"AST", "0"},
  {"PWM", "0"},
  {"AUT", "0"},
  {"NIP", "0"}, 			       
  {"NAD", ""}, 		  			   
  {"NSM", ""}, 	  					
  {"NGW", ""},					
  {"NDN", ""}, 				      
  {"NCH", "4"}, 			     
  {"NWV", "0"}, 			      
  {"NTS", "http://techfactory.hu/esp/time/"},
  {"TIM", "1"},
  {"BRC", "1"}, 
  //default_start_action per device
  {"SD0", ""},
  {"SD1", ""},
  {"SD2", ""},
  {"SD3", ""},
  {"EQC", "6"},
  {"EQ0", "5,50,150;0,0,255;100,0,255;255,0,100"},
  {"EQ1", "0,180,30;0,255,0;0,255,100;0,255,255"},
  {"EQ2", "255,0,50;255,50,50;255,100,50;255,0,0"},
  {"EQ3", "0,255,50;50,255,50;100,255,70;0,255,0"},
  {"EQ4", "0,50,255;50,50,255;100,70,255;0,0,255"},
  {"EQ5", "100,100,100;200,100,200;255,100,255;255,0,0"},
  {"EQP", "150,1024,30,2,10"} 
};
TFCustomCfg Config("config.dat", 33, DefaultConfig);
