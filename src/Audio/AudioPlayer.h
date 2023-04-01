#pragma once


#include "AudioApp.h"
#include "../UiRenderer.h"
#include "NoteBuffer.h"

namespace lineweaver {
	enum AudioMode;
}

namespace gam {

	struct StagedNote {
		StagedNote(int id, float f, float a) : id{ id }, amp{a}, freq{ f } {};
		int id;
		float amp;
		float freq;

		bool operator<(const StagedNote& a) const {
			return amp > a.amp;
		};
	};

	
	class AudioPlayer : public AudioApp {
	public:

		AudioPlayer(lineweaver::UiRenderer* ui) : m_ui(ui) {};

		void playNote(int id, float value, int angle);
		void setAvailableNotes(std::vector<std::pair<int, std::pair<float, int>>> lines, int sort = 0);

		void sortStagedNotes(int sortMode);


		void onAudio(AudioIOData& io);

		void mainThread(float deltaTime, int mode);
		void mainTimerPlayback(float deltatime, bool repeat);
		void mainMetricPlayback(float metric);
		void mainFixedFrequency(float metric);

		float preComputeAmplitudeReScaling();

		std::map<int, float> peekBufferOsc() { return mNoteBuffer.getOscillation(); };

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

		float bpm = 0.0f;
		int beats = 0;
	};
}
