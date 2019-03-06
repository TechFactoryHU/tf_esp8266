/*
*	Sample configuration file for TFLight
*	Copy this file to TFLight root folder and rename to config_hw.h
*/

void DefaultLights(TFLights *light) {
	//how many lights we have
	light->begin(3);
	
	//RGB strips
	//light->setRGB((uint8_t)index, (uint8_t)redPin, (uint8_t)greenPin, (uint8_t)bluePin, (char[30])devicename)
	light->setRGB(0, D0, D1, D2,  "RGB#1");
	
	//LED strips
	//light->setLed((uint8_t)index, (uint8_t)pin, (char[30])devicename)
	light->setLed(1, D3,  "Led#1");
	
	
	//NeoPixel (WS2812b) strip
	//light->setPixel((uint8_t)index,  (uint8_t)pin, (int)pixelcount,  (char[30])name, (bool) shiftPixels) //10-pin 
	//shiftPixels=logic level shifting (3.3->5v) with "sacrificial" pixel
	//one pixel will be shifted, read more: https://hackaday.com/2017/01/20/cheating-at-5v-ws2812-control-to-use-a-3-3v-data-line/
	light->setPixel(2, D4, 100, "Pixel#1", false);
	
	//pixel types can be split to group(s)
	//how many groups
	light->beginPixelGroup(2);
	
	//light->setPixelGroup((uint8_t)grp_index, (uint8_t)mainPixelIndex, (int)startPixel, (int)endPixel, (char[30])name);
	light->setPixelGroup(0, 2, 0,  49,  "PixelGrp#1");
	light->setPixelGroup(1, 2, 50,  100,  "PixelGrp#2");
}