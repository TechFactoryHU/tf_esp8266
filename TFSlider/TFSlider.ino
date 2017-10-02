/*
 *   ESP8266 camera slider
 *   v1.0 - 2017.03.05
 *   techfactory.hu
 */
 
#include "tfheader.h"
#include "tfhelpers/tfcustomcfg.h"
#include "tfhelpers/tfnetwork.h"
#include "stepper_driver/DRV8825.h"

/*
 *  SETUP
 *  -----
 */
 
#define MOTOR_STEPS 200
#define MOTOR_DIR_PIN 5
#define MOTOR_STEP_PIN 4
#define DRV_MODE0 13
#define DRV_MODE1 12
#define DRV_MODE2 15
#define DRV_LEFT_STOP 14
#define DRV_RIGHT_STOP 16

const String FW_VER    = "tfslider-1.0";

//default device config
TFConfigItem DefaultConfig[6] = {
  //device name
  {"DN", "TFSLIDER-DEVICE"},
  //ap ssid
  {"AP", "TFSLIDER01"},
  //password
  {"PWD", "A1234B5678"},
  //disable AP on STA mode (wifi client mode)
  {"AST", "0"},
  //change pwm frequency globally (0=unchanged)
  {"PWM", "0"},
  //require authentication for clients (udp&tcp)
  //beta function; most of the clients doesnt support it currently
  {"AUT", "0"}
};
TFCustomCfg Config("config.dat", 6, DefaultConfig);

//Custom configuration for slider
TFConfigItem DefaultSliderConfig[6] = {
    {"LENGTH", 0}, 
    {"TIME", 0},
    {"DEF_RPM", "200"},
    {"STEPS", "1,2,4,8,16,32"},
    {"DEF_STEP", "1"},
    {"DEF_RPM", "100"}
};

TFCustomCfg SliderConfig("slidercfg.dat", 6, DefaultSliderConfig);
//networking 
TFNetwork Network(&Config);

//motor driver
DRV8825 driver(MOTOR_STEPS, MOTOR_DIR_PIN, MOTOR_STEP_PIN, DRV_MODE0, DRV_MODE1, DRV_MODE2);
uint8_t deviceMode = 1;
uint8_t deviceLeftSensor = 0;
uint8_t deviceRightSensor = 0;
uint8_t _calibrationStatus = 0;
unsigned long _calibrationTime = 0;
unsigned long laststatustime = 0;
int _calibrationMove = 0;

int slider_length = 0;
int slider_pos = 0;
unsigned long slider_time = 0;
long _lastMoveStep;

void setup() 
{  
 
  #ifdef TF_DEBUG
	Serial.begin(115200);
    Serial.println("Starting up...");
  #endif

  pinMode(DRV_LEFT_STOP, INPUT);
  pinMode(DRV_RIGHT_STOP, INPUT);
  
  //Load config
  Config.begin();
  Config.load();
 
  #ifdef TF_DEBUG
	Serial.println("Config loaded");
  #endif
   
  Network.begin();
  Network.OnMsg(Network_Incoming);
  Network.OnDeviceInfo(Network_Devinfo);
  
  #ifdef TF_DEBUG
	Serial.println("Net callbacks loaded");
  #endif
  
  SliderConfig.begin();
  SliderConfig.load();
  
  int rpm = 100;
  int dstep = 1;
  driver.begin(rpm, dstep);
  
  if (SliderConfig.exists("DEF_RPM")) {
      rpm = SliderConfig.getInt("DEF_RPM");
      if (rpm>0) { driver.setRPM(rpm);}else { driver.setRPM(100); }
  }else { driver.setRPM(100); }

  if (SliderConfig.exists("DEF_STEP")) {
      if (rpm == 1) {
        dstep = SliderConfig.getInt("DEF_STEP");
        driver.setMicrostep(dstep);
      }
  }
  slider_length = SliderConfig.getInt("LENGTH");
  slider_time = SliderConfig.getInt("TIME");

  driver.onMove(Driver_OnMove);
}

void loop() 
{
   #ifdef TF_DEBUG
    if (Serial.available()>0) {
    String rd = Serial.readStringUntil('\n');
    String response = Network_Incoming(&rd[0]);
      if (response.length()>0) {
         Serial.println(">"+response);
      }
    }
   #endif
   
   Network.UdpWorker();
   Network.TcpWorker();

   deviceLeftSensor  = digitalRead(DRV_LEFT_STOP);
   deviceRightSensor = digitalRead(DRV_RIGHT_STOP); 
  
   //calibration 
   if (deviceMode == 10) {
       Calibration(deviceLeftSensor, deviceRightSensor);
   }else {
     if (driver.getDirection() == 0) {
  	  if (deviceLeftSensor==1) {
        slider_pos = 0;
  		  driver.stop();
  	  }
     }
     else if (driver.getDirection() == 1) {
  	  if (deviceRightSensor==1) {
         slider_pos = slider_length;
  		   driver.stop();
  	  }
     }
     
     driver.moveLoop();
     if (laststatustime + 2000 < millis()){
        laststatustime = millis(); 
        Network.sendToClients("+STATUS="+GetDeviceStatus());
     }
   }
}

void Driver_OnMove(long rem_steps, long steps_count) {
   //new move start
   if (steps_count == 0) {
   }else {
     if (driver.getDirection() > 0) { slider_pos++; }
     else { slider_pos--; }
     if (slider_pos < 0) { slider_pos = 0; }
     else if(slider_pos > slider_length) { slider_pos = slider_length; }
   }
}

void Stop() {
   driver.stop();
   deviceMode = 1;
}

void setMode(int mode, long data = 0) {
	#ifdef TF_DEBUG
    Serial.print("[setMode]");
    Serial.print(mode);
    Serial.print(" = ");
    Serial.println(data);
    #endif
    
    if (deviceMode == 10 && mode != 1) {
        return;
    }
    //off
    if (mode == 0) {
       driver.disable();
       deviceMode = 0;
    }
    //ready & hold motor in position
    else if (mode == 1) {
       driver.enable();
       deviceMode = 1;
    }
    //move
    else if (mode == 2) {
         if (driver.status()==false) { driver.enable(); }
         deviceMode = 2;
         driver.move(data);
    //calibration
    }else if(mode == 10) {
        if (driver.status()==false) { driver.enable(); }
        driver.setRPM(100);
        driver.setMicrostep(1);
         _calibrationStatus = 0;
        deviceMode = 10;
    }
}

void Calibration(int leftsens, int rightsens) {
    if (_calibrationStatus == 0) {
        if (!leftsens) { 
            if (!driver.isMoving()) {
              driver.move(-10); 
            }
        }
        else {
            driver.stop();
            _calibrationStatus = 1;
            _calibrationMove = 0;
            _calibrationTime = millis();
        }
    }else if(_calibrationStatus == 1) {
       if (!rightsens) {
          if (!driver.isMoving()) {
             _calibrationMove += 10;
             driver.move(10); 
          }
       }else {
          driver.stop();
          _calibrationMove -= (int)_lastMoveStep;
          #ifdef TF_DEBUG
          Serial.print("[Calibration] Step 1) Length is :");
          Serial.println(_calibrationMove);
          #endif
           
          slider_time   =  millis() - _calibrationTime;
          #ifdef TF_DEBUG
          Serial.print("[Calibration] Step 1) Time is :");
          Serial.println(slider_time);
          #endif
          
          SliderConfig.add("LENGTH", String(_calibrationMove));
          SliderConfig.add("TIME", String( slider_time));
          slider_length = _calibrationMove;
          slider_time   =  millis() - _calibrationTime;
          _calibrationStatus = 2;
       }
    }
    else if(_calibrationStatus == 2) {
        if (!leftsens) { 
            if (!driver.isMoving()) {
                _calibrationMove -= 10;
               driver.move(-10); 
            }
        }else {
			driver.stop();
			_calibrationMove += (int)_lastMoveStep;
			#ifdef TF_DEBUG
			Serial.print("[Calibration] Step 2) done, difference is :");
			Serial.println(_calibrationMove);
			#endif
			SliderConfig.save();
			setMode(1,0);
        }
    }
    driver.moveLoop();
}

String GetDeviceStatus() {
  return String(deviceMode)+","+String(slider_pos)+","+String(slider_length);
}

//commands handler
#include "network_defaults.h"
String Network_Incoming(char* packet){
  int pcount = Network.parsePacket(packet);
  if (pcount > -1) {
     if (Network.isCommand("+POS")) {
        return String(slider_pos)+","+String(slider_length);
     }
     else if (Network.isCommand("+STATUS")) {
        return "+ok=+STATUS:"+GetDeviceStatus(); 
     }
     else if (Network.isCommand("+STOP")) {
			    driver.stop();
          return "+ok="+String(Network.getCommand());
     }
     //mode
     else if (Network.isCommand("+MODE")) {
        setMode(Network.getParam(0),(long)Network.getParam(1));
        return "+ok="+String(Network.getCommand());
     }
	 //move=[int] negative/positive numbers 
     else if (Network.isCommand("+MOVE")) {
        setMode(2,(long)Network.getParam(0));
        return "+ok="+String(Network.getCommand());
     }
     //moveTo=[position]
     else if (Network.isCommand("+MOVETO")) {
         if (slider_length > 0){
              int pos = Network.getParam(0);
              if (pos >= 0 && pos <= slider_length) {
                   setMode(2,  pos - slider_pos);
                   return "+ok="+String(Network.getCommand());
              }
              return "-err=+MOVETO:WRONG_PARAMETER";
         }else {
            return "-err=+MOVETO:NOT_CALIBRATED";
         }
     }
     //rotate
     else if (Network.isCommand("+ROTATE")) {
         driver.rotate(Network.getParam(0));
         return "+ok="+String(Network.getCommand());
     }

     //set-rpm
     else if (Network.isCommand("+AT-RPM")) {
        driver.setRPM(Network.getParam(0));
        return "+ok";
     }
     //set-microstep
     else if (Network.isCommand("+AT-MST")) {
		    driver.setMicrostep(Network.getParam(0));
        return "+ok";
     }
     else if (Network.isCommand("+AT-CALIBRATION")) {
         setMode(10,0);
         return "+ok=+AT-CALIBRATION";
     }
     else {
         return Network_Default_Cmds(pcount);
     }
     return "-err="+String(Network.getCommand());
  }
  return "-err=UKNOWN_CMD";
}

//Device info for clients
String Network_Devinfo() {
    String json = "{";
    json += "\"fw\":\""+String(FW_VER)+"\",\"hwnr\":\""+Config.getString("HWNR")+"\",\"auth\":"+(Config.getBool("AUTH")?"1":"0")+",\"devs\":[]";
    #ifdef TFNETWORK_TCPSERVER && TFNETWORK_TCPSERVER == 1
    json += ",\"tcp\":\""+String(TFNETWORK_TCPPORT)+"\"";
    #else
    json += ",\"tcp\":\"0\"";
    #endif
    json += ",\"name\":\""+Config.getString("DEVNAME")+"\"";
    if (Network.ApStatus()) { json += ",\"ap\":\""+Config.getString("AP") +"\""; }
    else {
      json += ",\"ap\":\"0\"";
    }
    if (WiFi.status() == WL_CONNECTED) { json += ",\"sta\":\""+String(WiFi.SSID())+"\""; }
    json += "}";
    return json;
}
