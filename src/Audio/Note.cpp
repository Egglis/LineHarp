#include "Note.h"
#include <vector>

using namespace gam;



Note::Note(float freq, float amp) : mFreq{ freq }, mAmp{ amp }
{
	mString.freq(freq);
}




