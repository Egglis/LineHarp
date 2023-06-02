#include "NoteBuffer.h"

// TODO remove
#include <globjects/globjects.h>
#include <globjects/logging.h>

#include <cmath>

using namespace gam;


void NoteBuffer::addNote(int i, float f, float a, float reScale)
{

	std::unique_lock<std::mutex> lock(mtx);

	// Add new Note
	Note* note = new Note(i, f, a);
	
	mNotes.push_back(note);

	lock.unlock();
}

std::map<int, float> NoteBuffer::getOscillation() {
	std::unique_lock<std::mutex> lock(mtx);

	std::map<int, float> copy = mOsc;

	lock.unlock();
	return copy;
}


float NoteBuffer::readBuffer()
{

	std::unique_lock<std::mutex> lock(mtx);

	float sound = 0.0f;
	float reScale = 1.0f;

	float sum = 0.0f;
	for (const auto& note : mNotes) {
		sum += std::abs(note->amp());
	}

	float max = 0.6f;
	reScale = std::min(max / sum, max);
	
	mOsc.clear();
	std::vector<Note*> to_delete;
	for (auto it = mNotes.begin(); it != mNotes.end(); ++it) {
		auto note = *it;

		float am = (note->amp() * reScale);
		sound += note->pluck() * am;

		if (note->getLineID() > -1) mOsc[note->getLineID()] = sound;

		// TODO make the falloff an adv option --> 
		 note->reduce(0.000001 * mNotes.size()); 

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
