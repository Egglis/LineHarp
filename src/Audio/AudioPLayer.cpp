#include "AudioPlayer.h"
#include <future>


// TODO remove
#include <globjects/globjects.h>
#include <globjects/logging.h>

using namespace gam;

// Play a note based on value = volume, angle = freq
void AudioPlayer::playNote(float value, int angle) {


	while (blockNoteChanging) {
		// Wait for note playing to finish
	};
	mAudioOn = true;



	// Then block the note playing thread
	blockThread = true;
	Note* noteX = mNotes.at(mCurrentString);

	noteX->setMinimumAmp(m_ui->Audio()->minAmp);
	noteX->amp(value);
	noteX->angFreq(angle);
	noteX->reset();
	blockThread = false;

	mCurrentString += 1;
	if (mCurrentString > 5) mCurrentString = 0;


}

void AudioPlayer::addNoteToQueue(int id, float amp, int angle)
{
	QueNote q(id, amp, angle);
	mQueue.push_back(q);
}

void AudioPlayer::addMuteNote() {
	QueNote q(-2, -1, -1);
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

				if (qn->id != -2) {
					playNote(qn->amp, qn->angle);
				};

				// Debugging 
				// m_ui->setFocusId(qn->id);
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



void AudioPlayer::addToSound(float pluck) {
	ms += pluck;
}

void AudioPlayer::onAudio(AudioIOData& io)
{
	while (io()) {

		// toggle on/off audio
		if (!mAudioOn) continue;

		// If main thread is writing to mNotes then block this thread!
		if (blockThread) {
			continue;
		}
		// Block the ability to write to mNotes when values are read
		blockNoteChanging = true;


		// The sound of all 6 Strings/Notes are summed 
		float s = 0;
		for (auto &note : mNotes) {
			s += note->pluck();
		}

		io.out(0) = io.out(1) = s * m_ui->Audio()->volume;


		// Unblock 
		blockNoteChanging = false;
	}
}

void AudioPlayer::resetAllStrings()
{
	// Block the ability to write to mNotes when values are read
	blockThread = true;
	for (auto& note : mNotes) {
		note->mute();
		note->reset();
	}
	// Unblock 
	blockThread = false;

}
