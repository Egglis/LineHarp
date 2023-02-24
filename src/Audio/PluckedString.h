
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

	PluckedString(double startTime = 0, float frq = 440)
		: mAmp(1), mDur(2), delay1(0.4, 0.2),
		env(0.9), fil(2), delay(1. / 27.5, 1. / frq), p(frq)
	{
	}

	PluckedString& freq(float v) { 
		delay.freq(v); 
		p.freq(v);
		return *this; 
	}
	PluckedString& amp(float v) { mAmp = v; return *this; }

	PluckedString& pan(float v) { mPan.pos(v); return *this; }
	void reset() { env.reset(); }



	// Creates some sort of feedback loop 
	float operator() () {
		return p();
		return (*this)(noise() * env());
	}

	float operator() (float in) {
		return delay(
			fil(delay() + in)
		);
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
