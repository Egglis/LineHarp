#pragma once


#include "AudioApp.h"
#include "PluckedString.h"
#include "../UiRenderer.h"


namespace gam {

	class NoteMap;
	class AudioPlayer : public AudioApp {
	public:
		AudioPlayer(lineweaver::UiRenderer* ui){
			m_ui = ui;
			m_ui->setNoteMap(&m_noteMap);
		};

		// Decides how often a note is played
		Accum<> tmr{ 1.0 / 0.2 }; 
		PluckedString pluck{ scl::freq("a6") };

		// Sets the current freq of the plucked string
		void setStringFrequency(float hz);

		// Sets the freqencuy based on which metric is passed
		void setStringFreqOnMetric(float value , gam::AudioMetric metric);

		void onAudio(AudioIOData& io);
		NoteMap* noteMap() { return &m_noteMap; };
	private:
		NoteMap m_noteMap;
		lineweaver::UiRenderer* m_ui;
	};

}


// TODO
/*
- Give feedback for which audio device chosen
- Give information on channels
*/
