/*
*	Vegtelen tukros hangszoro minta konfiguracio
*	Masold at a fajlt a TFLight mappa gyokerebe config_hw.h nevvel
*/
void DefaultLights(TFLights *light) {
	//3 kulonbozo fenyeszkozunk van
	light->begin(3);
	
	//Ebbol 2 RGB szalag (hangszoronal oldalankent 1)
	//light->setRGB((uint8_t)index, (uint8_t)redPin, (uint8_t)greenPin, (uint8_t)bluePin, (char[30])devicename)
	light->setRGB(0, 16, 15, 13,  "RGB#1");
	light->setRGB(1, 14, 12, 2,  "RGB#2");
	
	//NeoPixel (WS2812b) cimezheto ledszalag (ez fut a hangszorok korul)
	//light->setPixel((uint8_t)index,  (uint8_t)pin, (int)pixelcount,  (char[30])name, (bool) shiftPixels) //10-pin 
	//shiftPixels=Lehetoseg van egy aldozati pixellel megoldani a 3.3V-5V logikai szintemelest
	//ebben az esetben az elso pixelt nem hasznaljuk. Bovebb leiras a linken:
	//https://hackaday.com/2017/01/20/cheating-at-5v-ws2812-control-to-use-a-3-3v-data-line/
	light->setPixel(2, 10, 145, "Pixel#1", false);
	
	//a pixel tipusu ledszalagokat csoportokra lehet osztani
	//ez kesobb hasznos lehet, mert a csoportot lehet kulon vezerelni
	//itt mi most osszesen 4 csoportra osztjuk (hangfalankent 2 csoportra)
	light->beginPixelGroup(2);
	
	//light->setPixelGroup((uint8_t)grp_index, (uint8_t)mainPixelIndex, (int)startPixel, (int)endPixel, (char[30])name);
	//hangfal 1, a mely hangszoro koruli resz
	light->setPixelGroup(0, 2, 0,  6,  "Bass#1");
	//hangfal 1, a kozep/magas koruli resz
	light->setPixelGroup(1, 3, 7, 13,  "Mid/High#1");
	//hangfal 2, a mely hangszoro koruli resz
	light->setPixelGroup(2, 2, 0,  6,  "Bass#2");
	//hangfal 2, a kozep/magas koruli resz
	light->setPixelGroup(3, 3, 7, 13,  "Mid/High#2");
}