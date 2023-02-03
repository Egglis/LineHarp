#pragma once


#include <Gamma/Delay.h>
#include <Gamma/Envelope.h>
#include <Gamma/Filter.h>
#include <Gamma/Noise.h>
#include <Gamma/Oscillator.h>


namespace gam {
	class PluckedString {
	public:
		PluckedString(float frq = 440)
			: env(0.1), fil(3), delay(1./27.5, 1./frq) {};

		
		float operator()() {
			return (*this)(noise() * env());
		};

		float operator()(float in) {
			return delay(
				fil(delay() + in)
			);
		};

		void reset() { env.reset(); };
		void freq(float v) { delay.freq(v); };
		bool done() { return fil.reachedEnd(); };

		NoiseWhite<> noise;
		Decay<> env;
		MovingAvg<> fil;
		Delay<float, ipl::Trunc> delay;
	};


}