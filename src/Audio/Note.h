
#include <string>

#include "PluckedString.h"


namespace gam {
	class Note {
	public:
		Note(float freq = 440, float amp = 0.0f);
		float pluck() { return (m_string() * (m_amp + m_minVol)); };
		void reset() { m_string.reset(); };
		void angFreq(int ang);
		void freq(float freq) { m_string.freq(freq); };
		void amp(float amp) { m_amp = amp; };
		void mute() { m_amp = 0.0 - m_minVol; }
		
		void setMinimumAmp(float amp) { m_minVol = amp; };
	private:
		PluckedString m_string{ 440 };
		std::string m_name;
		float m_freq;
		float m_amp;
		float m_minVol = 0.2;
	};
}