#ifndef GAMMA_AUDIO_APP_H_INC
#define GAMMA_AUDIO_APP_H_INC

#include <stdio.h>
#include "Gamma/AudioIO.h"
#include "Gamma/Domain.h"

//  A very basic audio application abstraction
namespace gam {

	class AudioApp : public AudioCallback {
	public:

		AudioApp() {
			mAudioIO.append(*this);
			initAudio(44100);
		}

		void initAudio(
			double framesPerSec, unsigned framesPerBuffer = 128,
			unsigned outChans = 1, unsigned inChans = 0
		) {
			mAudioIO.framesPerSecond(framesPerSec);
			mAudioIO.framesPerBuffer(framesPerBuffer);
			mAudioIO.channelsOut(outChans);
			mAudioIO.channelsIn(inChans);
			mAudioIO.clipOut(false);
			sampleRate(framesPerSec);
		}

		AudioIO& audioIO() { return mAudioIO; }

		void start(bool block = false) {
			mAudioIO.start();
		}

	private:
		AudioIO mAudioIO;
	};

}

#endif