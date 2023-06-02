

#include <vector>
#include <thread>
#include <shared_mutex>
#include <deque>
#include <map>
#include "Note.h"

namespace gam {




	class NoteBuffer {
	public:
		void addNote(int i, float f, float a, float reScale = -1);

		// Sums all Notes
		float readBuffer();

		// Mainly used for rendering the "wiggle" effect, eventually passed to the shaders stages: Returns the ocelating sound
		std::map<int, float> getOscillation();
	private:
		std::vector<Note*> mNotes;
		std::map<int, float> mOsc;
		std::mutex mtx;

	};



}