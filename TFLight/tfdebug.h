#ifndef TFDEBUG_H
#define TFDEBUG_H
#include "tfheader.h"

class TFDebug {
	public:
		TFDebug();
		 ~TFDebug();
		void log(String msg, uint8_t level);
		void update();
		
	private:
		
};
#endif // TFDEBUG_H

