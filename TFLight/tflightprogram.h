#ifndef TFLIGHTPROGRAM_H
#define TFLIGHTPROGRAM_H
#include "Arduino.h"
#include "FS.h"
#include "helpers.h"
#include "tfheader.h"
#include "tflights.h"
#include "tflightanim.h"

class TFLightProgram {

	public:
		TFLightProgram();
		bool load(uint8_t type, int program_id, TFLightAnim* a);
		bool save(TFLightAnim* a);
		int saveTo(uint8_t type, int program_id, String data, bool append);
		bool remove(uint8_t type, int program_id);
		TFLightsProgList getList(uint8_t type, int start=0);
		
	private:
		int _nextprogid(uint8_t type);
		String _getfolder(uint8_t type);
		int _file2int(String f);
};

#endif // TFLIGHTPROGRAM_H

