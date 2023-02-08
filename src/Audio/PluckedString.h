

#include "Gamma/Envelope.h"
#include "Gamma/Effects.h"
#include "Gamma/Noise.h"

namespace gam {
	
	class PluckedString {
public:

	PluckedString(double startTime = 0, float frq = 440)
		: mAmp(1), mDur(2), delay1(0.4, 0.2),
		env(0.1), fil(2), delay(1. / 27.5, 1. / frq)
	{
	}

	PluckedString& freq(float v) { delay.freq(v); return *this; }
	PluckedString& amp(float v) { mAmp = v; return *this; }


	PluckedString& pan(float v) { mPan.pos(v); return *this; }
	void reset() { env.reset(); }




	float operator() () {
		return (*this)(noise() * env());
	}

	float operator() (float in) {
		return delay(
			fil(delay() + in)
		);
	}


protected:
	float mAmp;
	float mDur;
	Pan<> mPan;
	NoiseWhite<> noise;
	Decay<> env;
	MovingAvg<> fil;
	Delay<float, ipl::Trunc> delay, delay1;
};

}
