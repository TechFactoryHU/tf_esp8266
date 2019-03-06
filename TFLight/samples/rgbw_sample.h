/*
*	RGB + W ledszalag minta
*	Masold at a fajlt a TFLight mappa gyokerebe config_hw.h nevvel
*/
void DefaultLights(TFLights *light) {
	//2 kulonbozo fenyeszkozunk van
	light->begin(2);
	
	//Egy RGB ledszalag
	//light->setRGB((uint8_t)index, (uint8_t)redPin, (uint8_t)greenPin, (uint8_t)bluePin, (char[30])devicename)
	light->setRGB(0, D0, D1, D2,  "RGB#1");
	
	//Plusz egy sima ledszalag (W)
	//light->setLed((uint8_t)index, (uint8_t)Pin, (char[30])devicename)
	light->setLed(1, D3, "Led#1");
}