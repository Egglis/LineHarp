#include "AudioPlayer.h"

// TODO remove
#include <globjects/globjects.h>
#include <globjects/logging.h>

using namespace gam;

void AudioPlayer::setStringFrequency(float hz)
{
	const std::string note = m_noteMap.getNote(hz);
	pluck.freq(scl::freq(note.c_str()));
	globjects::debug() << "Playing: " << note << std::endl;
}

void AudioPlayer::setStringFreqOnMetric(float value, gam::AudioMetric metric)
{	
	const float freq = m_noteMap.mapValueToFreqRange(value, metric);
	setStringFrequency(freq);
}


void AudioPlayer::onAudio(AudioIOData& io)
{
	while (io()) {

		// A beat or period has passed...
		if (tmr()) {


		}

		float s = pluck();

		// Sterio sound
		io.out(0) = io.out(1) = s * m_ui->Audio()->volume;
	}
}
