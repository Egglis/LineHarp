#include "AudioPlayer.h"

// TODO remove
#include <globjects/globjects.h>
#include <globjects/logging.h>

using namespace gam;


void AudioPlayer::playNote(float value) {
	// Set amplitude and frequency based on given value
	
	note1.amp(value);
	note1.freq(440);
	note1.reset();

}

void AudioPlayer::onAudio(AudioIOData& io)
{
	while (io()) {

		// A beat or period has passed...
		if (tmr()) {


		}

		// TODO add a queue that is combined instead of playing a single channel
		float s = note1.pluck();

		// Sterio sound
		io.out(0) = io.out(1) = s * m_ui->Audio()->volume;
	}
}
