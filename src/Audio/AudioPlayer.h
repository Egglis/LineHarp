#pragma once


#include "AudioApp.h"
#include "../UiRenderer.h"
#include "Note.h"
#include <deque>
#include <atomic>

struct QueNote {
	int id;
	float amp;
	int angle;
	QueNote(int id, float amp, int angle) : id{ id }, amp{amp}, angle{ angle } {};

	bool operator<(const QueNote& a) const {
		return amp > a.amp; 
	};
};

namespace gam {

	class NoteMap;
	class AudioPlayer : public AudioApp {
	public:
		const int NR_UNIQUE_NOTES = 25;


		AudioPlayer(lineweaver::UiRenderer* ui){
			m_ui = ui;
			//m_ui->setNoteMap(&m_noteMap);

			
			for (int i = 0; i < NR_UNIQUE_NOTES; i++) {
				Note *tNote = new Note();
				mNotes.push_back(tNote);
			}

		};

		// Decides how often a note is played
		Accum<> tmr{ 1.0 / 0.2 }; 

		std::vector<Note*> mNotes;

		void playNote(float value, int angle);


		void addNoteToQueue(int id, float amp, int angle);
		void addMuteNote();
		void playQueue(float deltaTime);
		void playFoldQueue(float foldDelta, float foldTimer, int indexOverride = -1);

		void playInternalQueue(float deltaTime, int index = -1);

		void stopQueue()  { playQue = false; };
		void startQueue() { playQue = true; };
		int queSize() { return mQueue.size(); };
		void resetQueue() { 
			mQueue.clear();
		};
		bool isQuePlaying() { return playQue; };
		void sortQueue(int metric) { std::sort(mQueue.begin(), mQueue.end()); };

		void onAudio(AudioIOData& io);
		void resetAllStrings();

		void addToSound(float pluck);

		int m_index = -1;
	private:
		lineweaver::UiRenderer* m_ui;
		std::vector<QueNote> mQueue;

		int mCurrentString = 0;
		bool mAudioOn = false;
		bool playQue = false;

		bool blockThread = false;
		bool blockNoteChanging = false;

		float intervalTimer = 0.0f;


		float ms = 0;
	};
}


// TODO Audio Feedback
/*
- Give feedback for which audio device chosen
- Give information on channels
*/
