
#include "Gamma/Envelope.h"
#include "Gamma/Effects.h"
#include "Gamma/Noise.h"

/*	Gamma - Generic processing library

	Example:		Filter / Plucked String
	Description:	Simulation of a plucked string with noise and a feedback
					delay-line.

	This code is based on the example in: https://github.com/LancePutnam/Gamma/blob/master/examples/synths/PluckedString.cpp
*/

namespace gam {
	


	class PluckedString {
public:

	// Creates some sort of feedback loop 
	float operator() () {
		return 0.5;
	}

protected:

	Pluck p;

	float mAmp;
	float mDur;
	Pan<> mPan;
	NoisePink<> noise;
	Decay<> env;
	MovingAvg<> fil;
	Delay<float, ipl::Trunc> delay, delay1;
};

}
