#pragma once


#include "AudioApp.h"
#include "../UiRenderer.h"
#include "NoteBuffer.h"

namespace lineweaver {
	enum AudioMode;
}

namespace gam {

	struct StagedNote {
		StagedNote(float f, float a) : amp{a}, freq{ f } {};
		float amp;
		float freq;

		bool operator<(const StagedNote& a) const {
			return amp > a.amp;
		};
	};

	
	class AudioPlayer : public AudioApp {
	public:

		AudioPlayer(lineweaver::UiRenderer* ui) : m_ui(ui) {};

		void playNote(float value, int angle);
		void setAvailableNotes(std::vector<std::pair<float, int>> lines, int sort = 0);

		void sortStagedNotes(int sortMode);


		void onAudio(AudioIOData& io);

		void mainThread(float deltaTime, int mode);
		void mainTimerPlayback(float deltatime, bool repeat);
		void mainMetricPlayback(float metric);
		void mainFixedFrequency(float metric);

		float preComputeAmplitudeReScaling();



	private:
		lineweaver::UiRenderer* m_ui;
		NoteBuffer mNoteBuffer;
		std::vector<std::unique_ptr<StagedNote>> mStagedNotes;

		// Timer Playback
		int mIndex = 0;
		float mInternalTimer = 0.0f;
		float mStagedNotesScaling = -1.0f;

		// Metric Playback
		int prevIndex = 0;

		// Fixed Playbakc
		float prevMetric = 0.0f;

		bool mAudioOn = false;
		bool mStopQue = false;
	};
}
// void sortQueue(int metric) {  };

/*
// The Main Game Thread, Scary!
void NoteBuffer::popQueIfReady(float deltaTime, float target){
	internalBufferTimer += deltaTime;
	if (internalBufferTimer > target) {

		auto sNote = std::move(mNoteQue.front());

		std::unique_lock<std::mutex> lock(mtx);

		mNotes.push_back(new Note(sNote->freq, sNote->amp));

		lock.unlock();

		mNoteQue.pop_front();

		// Reset Timer
		internalBufferTimer = 0.0f;
	}
}

*/