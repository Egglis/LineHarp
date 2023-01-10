
#include <string>
#include <map>
#include <vector>

struct Note {
    Note(int frequency, std::string name) : frequency(frequency), name(name) {}
    float frequency;
    std::string name;
};


namespace gam {

    enum AudioFeedbackModes {
        ALWAYS_PLAY,
        ONLY_SELECTION,
        NEVER_PLAY
    };

    enum AudioMetric {
        IMPORTANCE,
        DISTANCE
    };


	class NoteMap{
	public:
        NoteMap();

		std::string getNote(const float hz);
        Note getNoteFromIndex(int index);
        std::vector<Note>* getNotes() { return &m_Notes; };

        // Maps a float value to frequency range
        float mapValueToFreqRange(float v, AudioMetric metric);

        int min_freq_index = 0; // C0 - 16.00hz
        int max_freq_index = 71; // B5 - 987.00hz
    private:
        std::string getNoteFromFrequency(float frequency);

        std::vector<Note> m_Notes;
	};
};