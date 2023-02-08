#include "NoteBuffer.h"

// TODO remove
#include <globjects/globjects.h>
#include <globjects/logging.h>

#include <cmath>

using namespace gam;



void NoteBuffer::addNote(float f, float a, float reScale)
{

	std::unique_lock<std::mutex> lock(mtx);

	// Add new Note
	Note* note = new Note(f, a);
	if(reScale > 0) note->reScaleAmp(reScale);
	
	mNotes.push_back(note);

	lock.unlock();
}


float NoteBuffer::readBuffer()
{

	std::unique_lock<std::mutex> lock(mtx);

	float sound = 0.0f;
	float reScale = 1.0f;

	float sum = 0.0f;
	for (const auto& note : mNotes) {
		sum += note->amp();
	}
	float max = 0.6f;
	reScale = std::min(max / sum, max);
	

	std::vector<Note*> to_delete;
	for (auto it = mNotes.begin(); it != mNotes.end(); ++it) {
		auto note = *it;

		float am = (note->amp() * reScale);
		sound += note->pluck() * am;
		note->reduce(0.0000001 * mNotes.size()); // TODO make the falloff an adv option --> 

		// Mark notes which amplitude < 0 for deletion
		if (note->done()) {
			to_delete.push_back(note);
		}

	}

	// Finally delete all notes that are complete
	for (const auto& note : to_delete) {
		mNotes.erase(std::remove(mNotes.begin(), mNotes.end(), note), mNotes.end());
		delete note;
	}


	lock.unlock();

	return sound;
}
