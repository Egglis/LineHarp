#pragma once


#include "AudioApp.h"
#include "../UiRenderer.h"
#include "Note.h"
#include <deque>

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
		Note note1 = Note(440, 1.0);
		Note note2 = Note(440, 1.0);

		std::deque<Note> notes;


		void playNote(float value);

		void onAudio(AudioIOData& io);

	private:
		lineweaver::UiRenderer* m_ui;
	};

}


// TODO
/*
- Give feedback for which audio device chosen
- Give information on channels
*/
