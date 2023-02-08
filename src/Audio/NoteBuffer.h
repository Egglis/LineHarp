

#include <vector>
#include <thread>
#include <shared_mutex>
#include <deque>

#include "Note.h"

namespace gam {




	class NoteBuffer {
	public:
		void addNote(float f, float a, float reScale = -1);

		// Sums all Notes
		float readBuffer();
	private:
		std::vector<Note*> mNotes;
		std::mutex mtx;
	};



}