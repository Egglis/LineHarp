#include "AudioPlayer.h"
#include "FreqMap.h"


// TODO remove
#include <globjects/globjects.h>
#include <globjects/logging.h>

using namespace gam;

// Main Audio Thread
void AudioPlayer::onAudio(AudioIOData& io)
{

	// Display the current Audio Device in the GUI
	if (m_ui->Audio()->defaultDevice == "None") {
		m_ui->Audio()->defaultDevice = gam::AudioDevice::defaultOutput().name();
	}

	while (io()) {

		// toggle on/off audio
		if (!mAudioOn) continue;


		float s = 0;
		s = mNoteBuffer.readBuffer();
				
		io.out(0) = s;
	}
}

// Called every frame
void AudioPlayer::mainThread(float deltaTime, int mode) {
	if (mStopQue) return;

	if (mode == 4) mainTimerPlayback(deltaTime, false); // Global Audio Playback
	else if (mode == 2) mainTimerPlayback(deltaTime, true);
	else if (mode == 3) mainMetricPlayback(m_ui->Lens()->lensDepthValue);
	else if (mode == 5) mainFixedFrequency(m_ui->Lens()->lensRadius);
}

void AudioPlayer::mainTimerPlayback(float deltaTime, bool repeat) {

	mInternalTimer += deltaTime;
	bpm += deltaTime;
	if (mInternalTimer > m_ui->Audio()->note_interval && !mStagedNotes.empty()) {

		// Retrive and play the next note
		if (mIndex >= 0 || mIndex <= mStagedNotes.size()) {
			auto sNote = std::move(mStagedNotes.at(mIndex));

			if(m_ui->Selection()->enableVisualAudioGuide) m_ui->Selection()->audioLineId = sNote.get()->id;


			mNoteBuffer.addNote(sNote.get()->id, sNote.get()->freq, sNote.get()->amp, -1);
			beats += 1;

			// Increment index and reset timer for next Staged Note
			mIndex += 1;
			mInternalTimer = 0.0f;
		}

	}

	// If playback reaches the end of the staged notes
	if (mIndex >= mStagedNotes.size()) {
		mIndex = 0;
		mStopQue = !repeat;
	}

	if (bpm > 60.0) {
		globjects::debug() << "Current BPM: " << bpm << std::endl;
		beats = 0;
		bpm = 0.0f;
	}
}

void AudioPlayer::mainMetricPlayback(float metric) {
	int index = (mStagedNotes.size() - 1) - (metric * (mStagedNotes.size() - 1));

	if (index != prevIndex && !mStagedNotes.empty()) {
		auto& sNote = std::move(mStagedNotes.at(index));

		if (m_ui->Selection()->enableVisualAudioGuide) m_ui->Selection()->audioLineId = sNote.get()->id;

		mNoteBuffer.addNote(sNote.get()->id, sNote.get()->freq, sNote.get()->amp);

	}

	prevIndex = index;
}

void AudioPlayer::mainFixedFrequency(float metric) {
	int index = (FreqMap::m_Notes.size() - 1) - (metric * (FreqMap::m_Notes.size() - 1));

	if (index != prevIndex) {
		mNoteBuffer.addNote(-1, FreqMap::m_Notes.at(index).freq, 0.5);

	}

	prevIndex = index;
}



// TODO Remove
float AudioPlayer::preComputeAmplitudeReScaling() {
	float sum = 0.0f;
	for (const auto& note : mStagedNotes) {
		sum += pow(note.get()->amp, 2);
	}

	mStagedNotesScaling = std::min(1.0f / sum, 1.0f);

	return mStagedNotesScaling;
}



void AudioPlayer::playNote(int id, float value, int angle) {
	mAudioOn = true;

	float f = FreqMap::mapAngleToNote(angle);
	float a = pow(value, 2);

	if (m_ui->Selection()->enableVisualAudioGuide) m_ui->Selection()->audioLineId = id;

	mNoteBuffer.addNote(id, f, a);
}


void AudioPlayer::setAvailableNotes(std::vector<std::pair<int, std::pair<float, int>>> lines, int sort) {
	mStagedNotes.clear();
	mIndex = 0;
	mStopQue = false;
	mAudioOn = true;


	// TODO Maybe to Sorting Here
	for (auto line : lines) {
		const float f = FreqMap::mapAngleToNote(line.second.second);
		const float a = line.second.first;
		const int i = line.first;
		mStagedNotes.push_back(std::make_unique<StagedNote>(i, f, a));
	}
	
	sortStagedNotes(sort);

}



bool compareByFrequency(const std::unique_ptr<StagedNote>& a, const std::unique_ptr<StagedNote>& b) {
	return a->freq < b->freq;
}
bool compareByAmplitude(const std::unique_ptr<StagedNote>& a, const std::unique_ptr<StagedNote>& b) {
	return a->amp > b->amp;
}

void AudioPlayer::sortStagedNotes(int sortMode) {
	if (sortMode == 1) std::sort(mStagedNotes.begin(), mStagedNotes.end(), compareByAmplitude); // Amplitude sorted
	else if (sortMode == 2) std::sort(mStagedNotes.begin(), mStagedNotes.end(), compareByFrequency); // Amplitude sorted
}