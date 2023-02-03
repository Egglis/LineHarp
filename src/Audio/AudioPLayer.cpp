#include "AudioPlayer.h"
#include <future>


// TODO remove
#include <globjects/globjects.h>
#include <globjects/logging.h>

using namespace gam;

// Play a note based on value = volume, angle = freq
void AudioPlayer::playNote(float value, int angle) {


	while (blockNoteChanging) { /* Wait for audio thread to finish */ };
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
	if (mCurrentString > NR_UNIQUE_NOTES-1) mCurrentString = 0;

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
			if (m_index < mQueue.size()) {

				// Honestly should not matter: TODO
				while (blockNoteChanging) { /* Wait for audio thread to finish */ };

				const QueNote* qn = &mQueue.at(m_index);
				const int t_id = qn->id;
				const float t_amp = qn->amp;
				const int t_ang = qn->angle;

				if (t_id != -2) {
					playNote(t_amp, t_ang);
				};

				// Debugging: Visually shows which notes that is played, will break pull Audio
				m_ui->setFocusId(t_id);

				m_index += 1;
			}
			else {
				m_index = 0;
				playQue = false;
			}

			intervalTimer = 0.0;
		}
	}
}

void AudioPlayer::playFoldQueue(float foldDelta, float foldTimer, int indexOverride) {
	if (playQue) {
		intervalTimer += foldDelta;


		const float fixedInterval = 0.1;
		if (intervalTimer > fixedInterval) {
			int index = int(round(foldTimer * 10) - 1);
			
			if (indexOverride != -1) {
				index = indexOverride;
			}

			globjects::debug() << index << std::endl;

			if (index >= 0 && index < mQueue.size()) {

				while(blockNoteChanging) { /* Wait for audio thread to finish */ };


				const QueNote* qn = &mQueue.at(index);
				const int t_id = qn->id;
				const float t_amp = qn->amp;
				const int t_ang = qn->angle;

				// Id == -2, Note is Mute
				if (t_id != -2) {
					playNote(t_amp, t_ang);
				};
				// Debugging: Visually shows which notes that is played, will break pull Audio
				// m_ui->setFocusId(t_id);

			}
			intervalTimer = 0.0;
		}



	}

}

void AudioPlayer::playInternalQueue(float deltaTime, int index) {
	if (!playQue) return;

	intervalTimer += deltaTime;
	if (intervalTimer > m_ui->Audio()->note_interval) {
		
		// Use internal index
		if (index == -1) {
			m_index += 1;
		 	index = m_index;
		}

		if (index < 0 || index >= mQueue.size()) return;

		const QueNote* qn = &mQueue.at(index);
		if (qn->id == -2) return;

		playNote(qn->amp, qn->angle);


		// Debugging
		if (m_ui->Audio()->enableVisualGuide) {
			m_ui->setFocusId(qn->id);
		}


		if (m_index > mQueue.size()) {
			m_index = -1;
			playQue = false;
		} 

		intervalTimer = 0.0;
	} 

}



void AudioPlayer::addToSound(float pluck) {
	ms += pluck;
}

void AudioPlayer::onAudio(AudioIOData& io)
{
	
	if (m_ui->Audio()->defaultDevice == "None") {
		m_ui->Audio()->defaultDevice = gam::AudioDevice::defaultOutput().name();
	}

	while (io()) {

		// toggle on/off audio
		if (!mAudioOn) continue;

		// If main thread is writing to mNotes then block this thread!
		if (blockThread) {
			continue;
		}
		// Block the ability to write to mNotes when values are read
		blockNoteChanging = true;



		float s = 0;
		for (auto &note : mNotes) {
			s += note->pluck();
			
		}

		io.out(0) = s * m_ui->Audio()->volume;


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
