#pragma once


#include "AudioApp.h"
#include "../UiRenderer.h"
#include "Note.h"
#include <deque>

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
		AudioPlayer(lineweaver::UiRenderer* ui){
			m_ui = ui;
			//m_ui->setNoteMap(&m_noteMap);
		};

		// Decides how often a note is played
		Accum<> tmr{ 1.0 / 0.2 }; 


		Note note0 = Note();
		Note note1 = Note();
		Note note2 = Note();
		Note note3 = Note();
		Note note4 = Note();
		Note note5 = Note();

		std::vector<Note*> mNotes{
			&note0,
			&note1,
			&note2,
			&note3,
			&note4,
			&note5
		};

		void playNote(float value, int angle);


		void addNoteToQueue(int id, float amp, int angle);
		void playQueue(float deltaTime);
		void stopQueue()  { playQue = false; };
		void startQueue() { playQue = true; };
		void resetQueue() { mQueue.clear(); };
		bool isQuePlaying() { return playQue; };
		void sortQueue(int metric) { std::sort(mQueue.begin(), mQueue.end()); };

		void onAudio(AudioIOData& io);
		void resetAllStrings();

	private:
		lineweaver::UiRenderer* m_ui;
		std::vector<QueNote> mQueue;

		int mCurrentString = 0;
		bool mAudioOn = false;
		bool playQue = false;


		float intervalTimer = 0.0f;
		int index = 0;
	};
}


// TODO
/*
- Give feedback for which audio device chosen
- Give information on channels
*/
