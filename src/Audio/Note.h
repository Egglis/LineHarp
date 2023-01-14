
#include <string>

#include "PluckedString.h"


namespace gam {
	class Note {
	public:
		Note(float freq = 440, float amp = 1.0f);
		float pluck() { return (m_string() * (m_amp+0.3)); };
		void reset() { m_string.reset(); };
		void freq(float freq) { m_string.freq(freq); };
		void amp(float amp) { m_amp = amp; };

		
	private:
		PluckedString m_string{ 440 };
		std::string m_name;
		float m_freq;
		float m_amp;
	};
}