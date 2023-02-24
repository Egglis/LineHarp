#include "Note.h"
#include <vector>

using namespace gam;



Note::Note(int id, float freq, float amp) : id{ id }, mFreq { freq }, mAmp{ amp }
{
	mString.freq(freq);
}




