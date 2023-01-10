#include "NoteMap.h"

using namespace gam;



std::string NoteMap::getNote(const float hz)
{
	return getNoteFromFrequency(hz);
}

Note NoteMap::getNoteFromIndex(int index)
{
	if (index >= 0 && index <= m_Notes.size()) {
		return m_Notes.at(index);
	}
	return Note(0.0f, "None");
}

float NoteMap::mapValueToFreqRange(float v, AudioMetric metric)
{
    float old_max, old_min;
    if (metric == IMPORTANCE || metric == DISTANCE) {
        old_min = 0.0f;
        old_max = 1.0f;
    }

    const float minFreq = getNoteFromIndex(min_freq_index).frequency;
    const float maxFreq = getNoteFromIndex(max_freq_index).frequency;
	const float oldRange = (old_max - old_min);
	const float newRange = (maxFreq - minFreq);
	return (((v - old_min) * newRange) / oldRange) + minFreq;
}

std::string NoteMap::getNoteFromFrequency(float frequency)
{
	float minDiff = 999999999999;
	std::string closestNote;
	for (const auto& note : m_Notes) {
		double diff = std::abs(note.frequency - frequency);
		if (diff < minDiff) {
			minDiff = diff;
			closestNote = note.name;
		}
	}
	return closestNote;
}

// Add all possible notes 
gam::NoteMap::NoteMap()
{
	m_Notes.push_back(Note(16.35f, "C0"));
    m_Notes.push_back(Note(17.32f, "C+0"));
    m_Notes.push_back(Note( 18.35f, "D0" ));
    m_Notes.push_back(Note( 19.45f, "D+0" ));
    m_Notes.push_back(Note( 20.60f, "E0" ));
    m_Notes.push_back(Note( 21.83f, "F0" ));
    m_Notes.push_back(Note( 23.12f, "F+0" ));
    m_Notes.push_back(Note( 24.50f, "G0" ));
    m_Notes.push_back(Note( 25.96f, "G+0" ));
    m_Notes.push_back(Note( 27.50f, "A0" ));
    m_Notes.push_back(Note( 29.14f, "A+0" ));
    m_Notes.push_back(Note( 30.87f, "B0" ));
    m_Notes.push_back(Note( 32.70f, "C1" ));
    m_Notes.push_back(Note( 34.65f, "C+1" ));
    m_Notes.push_back(Note( 36.71f, "D1" ));
    m_Notes.push_back(Note( 38.89f, "D+1" ));
    m_Notes.push_back(Note( 41.20f, "E1" ));
    m_Notes.push_back(Note( 43.65f, "F1" ));
    m_Notes.push_back(Note( 46.25f, "F+1" ));
    m_Notes.push_back(Note( 49.00f, "G1" ));
    m_Notes.push_back(Note( 51.91f, "G+1" ));
    m_Notes.push_back(Note( 55.00f, "A1" ));
    m_Notes.push_back(Note( 58.27f, "A+1" ));
    m_Notes.push_back(Note( 61.74f, "B1" ));
    m_Notes.push_back(Note( 65.41f, "C2" ));
    m_Notes.push_back(Note( 69.30f, "C+2" ));
    m_Notes.push_back(Note( 73.42f, "D2" ));
    m_Notes.push_back(Note( 77.78f, "D+2" ));
    m_Notes.push_back(Note( 82.41f, "E2" ));
    m_Notes.push_back(Note( 87.31f, "F2" ));
    m_Notes.push_back(Note( 92.50f, "F+2" ));
    m_Notes.push_back(Note( 98.00f, "G2" ));
    m_Notes.push_back(Note( 103.83f, "G+2" ));
    m_Notes.push_back(Note( 110.00f, "A2" ));
    m_Notes.push_back(Note( 116.54f, "A+2" ));
    m_Notes.push_back(Note( 123.47f, "B2" ));
    m_Notes.push_back(Note( 130.81f, "C3" ));
    m_Notes.push_back(Note( 138.59f, "C+3" ));
    m_Notes.push_back(Note( 146.83f, "D3" ));
    m_Notes.push_back(Note( 155.56f, "D+3" ));
    m_Notes.push_back(Note( 164.81f, "E3" ));
    m_Notes.push_back(Note( 174.61f, "F3" ));
    m_Notes.push_back(Note( 185.00f, "F+3" ));
    m_Notes.push_back(Note( 196.00f, "G3" ));
    m_Notes.push_back(Note( 207.65f, "G+3" ));
    m_Notes.push_back(Note( 220.00f, "A3" ));
    m_Notes.push_back(Note( 233.08f, "A+3" ));
    m_Notes.push_back(Note( 246.94f, "B3" ));
    m_Notes.push_back(Note( 261.63f, "C4" ));
    m_Notes.push_back(Note( 277.18f, "C+4" ));
    m_Notes.push_back(Note( 293.66f, "D4" ));
    m_Notes.push_back(Note( 311.13f, "D+4" ));
    m_Notes.push_back(Note( 329.63f, "E4" ));
    m_Notes.push_back(Note( 349.23f, "F4" ));
    m_Notes.push_back(Note( 369.99f, "F+4" ));
    m_Notes.push_back(Note( 392.00f, "G4" ));
    m_Notes.push_back(Note( 415.30f, "G+4" ));
    m_Notes.push_back(Note( 440.00f, "A4" ));
    m_Notes.push_back(Note( 466.16f, "A+4" ));
    m_Notes.push_back(Note( 493.88f, "B4" ));
    m_Notes.push_back(Note( 523.25f, "C5" ));
    m_Notes.push_back(Note( 554.37f, "C+5" ));
    m_Notes.push_back(Note( 587.33f, "D5" ));
    m_Notes.push_back(Note( 622.25f, "D+5" ));
    m_Notes.push_back(Note( 659.26f, "E5" ));
    m_Notes.push_back(Note( 698.46f, "F5" ));
    m_Notes.push_back(Note( 739.99f, "F+5" ));
    m_Notes.push_back(Note( 783.99f, "G5" ));
    m_Notes.push_back(Note( 830.61f, "G+5" ));
    m_Notes.push_back(Note( 880.00f, "A5" ));
    m_Notes.push_back(Note( 932.33f, "A+5" ));
    m_Notes.push_back(Note( 987.77f, "B5" ));
    m_Notes.push_back(Note( 1046.50f, "C6" ));
    m_Notes.push_back(Note( 1108.73f, "C+6" ));
    m_Notes.push_back(Note( 1174.66f, "D6" ));
    m_Notes.push_back(Note( 1244.51f, "D+6" ));
    m_Notes.push_back(Note( 1318.51f, "E6" ));
    m_Notes.push_back(Note( 1396.91f, "F6" ));
    m_Notes.push_back(Note( 1479.98f, "F+6" ));
    m_Notes.push_back(Note( 1567.98f, "G6" ));
    m_Notes.push_back(Note( 1661.22f, "G+6" ));
    m_Notes.push_back(Note( 1760.00f, "A6" ));
    m_Notes.push_back(Note( 1864.66f, "A+6" ));
    m_Notes.push_back(Note( 1975.53f, "B6" ));
    m_Notes.push_back(Note( 2093.00f, "C7" ));
    m_Notes.push_back(Note( 2217.46f, "C+7" ));
    m_Notes.push_back(Note( 2349.32f, "D7" ));
    m_Notes.push_back(Note( 2489.02f, "D+7" ));
    m_Notes.push_back(Note( 2637.02f, "E7" ));
    m_Notes.push_back(Note( 2793.83f, "F7" ));
    m_Notes.push_back(Note( 2959.96f, "F+7" ));
    m_Notes.push_back(Note( 3135.96f, "G7" ));
    m_Notes.push_back(Note( 3322.44f, "G+7" ));
    m_Notes.push_back(Note( 3520.00f, "A7" ));
    m_Notes.push_back(Note( 3729.31f, "A+7" ));
    m_Notes.push_back(Note( 3951.07f, "B7" ));
    m_Notes.push_back(Note( 4186.01f, "C8" ));
    m_Notes.push_back(Note( 4434.92f, "C+8" ));
    m_Notes.push_back(Note( 4698.63f, "D8" ));
    m_Notes.push_back(Note( 4978.03f, "D+8" ));
    m_Notes.push_back(Note( 5274.04f, "E8" ));
    m_Notes.push_back(Note( 5587.65f, "F8" ));
    m_Notes.push_back(Note( 5919.91f, "F+8" ));
    m_Notes.push_back(Note( 6271.93f, "G8" ));
    m_Notes.push_back(Note( 6644.88f, "G+8" ));
    m_Notes.push_back(Note( 7040.00f, "A8" ));
    m_Notes.push_back(Note( 7458.62f, "A+8" ));
    m_Notes.push_back(Note( 7902.13f, "B8"  ));
}