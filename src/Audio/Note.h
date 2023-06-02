
#include <string>

#include "PluckedString.h"


namespace gam {


	class Note {
	public:
		Note(int id, float freq = 440, float amp = 0.0f);
		~Note() {}

		const float pluck() { return mString(); };
		const float amp() { return mAmp; };
		const bool done() { return mAmp <= 0.0f; };
		const int getLineID() { return id; };

		void reScaleAmp(float factor) {
			mAmp *= factor;
			preScaled = true;
		}

		void reduce(float reduce) { mAmp = std::max(0.0f, mAmp - reduce); };

		bool preScaled = false;
	private:
		Pluck mString{ 440 };
		int id = 0; // Associated Id for the line
		float mAmp = 0.0f;
		float mFreq = 0.0f;

	};
}