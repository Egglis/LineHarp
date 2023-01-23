#include "AudioPlayer.h"

// TODO remove
#include <globjects/globjects.h>
#include <globjects/logging.h>

using namespace gam;

// Play a note based on value = volume, angle = freq
void AudioPlayer::playNote(float value, int angle) {

	mAudioOn = true;

	Note* noteX = mNotes.at(mCurrentString);

	noteX->setMinimumAmp(m_ui->Audio()->minAmp);
	noteX->amp(value);
	noteX->angFreq(angle);
	noteX->reset();

	mCurrentString += 1;
	if (mCurrentString > 5) mCurrentString = 0;
}

void AudioPlayer::addNoteToQueue(int id, float amp, int angle)
{
	QueNote q(id, amp, angle);
	mQueue.push_back(q);
}



// Ensures that the queue is played
void AudioPlayer::playQueue(float deltaTime)
{	
	if (playQue) {
		intervalTimer += deltaTime;

		// Play next note:
		if (intervalTimer > m_ui->Audio()->note_interval) {
			if (index < mQueue.size()) {
				const QueNote* qn = &mQueue.at(index);
				playNote(qn->amp, qn->angle);

				m_ui->setFocusId(qn->id);
				index += 1;
			}
			else {
				index = 0;
				playQue = false;
			}

			intervalTimer = 0.0;
		}
	}
}

void AudioPlayer::onAudio(AudioIOData& io)
{
	while (io()) {

		// toggle on/off audio
		if (!mAudioOn) continue;


		// A beat or period has passed...
		if (tmr()) {
			// Does nothing yet

		}

		float s = 0;
		for (Note* note : mNotes) {
			s += note->pluck();
		}

		// Sterio sound
		io.out(0) = io.out(1) = s * m_ui->Audio()->volume;
	}
}

void AudioPlayer::resetAllStrings()
{
	for (Note* note : mNotes) {
		note->mute();
		note->reset();
	}

}
