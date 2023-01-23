
#include <vector>
#include <string>


const struct FreqPair {
    FreqPair(float freq, std::string name) : freq{ freq }, name{ name } {};
    const float freq;
    const std::string name;
};



class FreqMap {
public:
    // Converts a frequency into a named note
    static std::string getNameFromFreq(float freq) {
        float minDiff = 999999999999;
        std::string closestNote;
        for (const auto& note : m_Notes) {
            double diff = std::abs(note.freq - freq);
            if (diff < minDiff) {
                minDiff = diff;
                closestNote = note.name;
            }
        }
        return closestNote;
    };

    static float mapAngleToNote(int angle) {
        float oldRange = float(180 - 0);

        float newMin = 16.36f;
        float newRange = (987.77f - newMin);

        return ((float(angle) * newRange) / oldRange) + newMin;
    };

    static const std::vector<FreqPair> m_Notes;

};


const std::vector<FreqPair> FreqMap::m_Notes({
    FreqPair(16.35f, "C0"),
        FreqPair(17.32f, "C+0"),
        FreqPair(18.35f, "D0"),
        FreqPair(19.45f, "D+0"),
        FreqPair(20.60f, "E0"),
        FreqPair(21.83f, "F0"),
        FreqPair(23.12f, "F+0"),
        FreqPair(24.50f, "G0"),
        FreqPair(25.96f, "G+0"),
        FreqPair(27.50f, "A0"),
        FreqPair(29.14f, "A+0"),
        FreqPair(30.87f, "B0"),
        FreqPair(32.70f, "C1"),
        FreqPair(34.65f, "C+1"),
        FreqPair(36.71f, "D1"),
        FreqPair(38.89f, "D+1"),
        FreqPair(41.20f, "E1"),
        FreqPair(43.65f, "F1"),
        FreqPair(46.25f, "F+1"),
        FreqPair(49.00f, "G1"),
        FreqPair(51.91f, "G+1"),
        FreqPair(55.00f, "A1"),
        FreqPair(58.27f, "A+1"),
        FreqPair(61.74f, "B1"),
        FreqPair(65.41f, "C2"),
        FreqPair(69.30f, "C+2"),
        FreqPair(73.42f, "D2"),
        FreqPair(77.78f, "D+2"),
        FreqPair(82.41f, "E2"),
        FreqPair(87.31f, "F2"),
        FreqPair(92.50f, "F+2"),
        FreqPair(98.00f, "G2"),
        FreqPair(103.83f, "G+2"),
        FreqPair(110.00f, "A2"),
        FreqPair(116.54f, "A+2"),
        FreqPair(123.47f, "B2"),
        FreqPair(130.81f, "C3"),
        FreqPair(138.59f, "C+3"),
        FreqPair(146.83f, "D3"),
        FreqPair(155.56f, "D+3"),
        FreqPair(164.81f, "E3"),
        FreqPair(174.61f, "F3"),
        FreqPair(185.00f, "F+3"),
        FreqPair(196.00f, "G3"),
        FreqPair(207.65f, "G+3"),
        FreqPair(220.00f, "A3"),
        FreqPair(233.08f, "A+3"),
        FreqPair(246.94f, "B3"),
        FreqPair(261.63f, "C4"),
        FreqPair(277.18f, "C+4"),
        FreqPair(293.66f, "D4"),
        FreqPair(311.13f, "D+4"),
        FreqPair(329.63f, "E4"),
        FreqPair(349.23f, "F4"),
        FreqPair(369.99f, "F+4"),
        FreqPair(392.00f, "G4"),
        FreqPair(415.30f, "G+4"),
        FreqPair(440.00f, "A4"),
        FreqPair(466.16f, "A+4"),
        FreqPair(493.88f, "B4"),
        FreqPair(523.25f, "C5"),
        FreqPair(554.37f, "C+5"),
        FreqPair(587.33f, "D5"),
        FreqPair(622.25f, "D+5"),
        FreqPair(659.26f, "E5"),
        FreqPair(698.46f, "F5"),
        FreqPair(739.99f, "F+5"),
        FreqPair(783.99f, "G5"),
        FreqPair(830.61f, "G+5"),
        FreqPair(880.00f, "A5"),
        FreqPair(932.33f, "A+5"),
        FreqPair(987.77f, "B5"),
        FreqPair(1046.50f, "C6"),
        FreqPair(1108.73f, "C+6"),
        FreqPair(1174.66f, "D6"),
        FreqPair(1244.51f, "D+6"),
        FreqPair(1318.51f, "E6"),
        FreqPair(1396.91f, "F6"),
        FreqPair(1479.98f, "F+6"),
        FreqPair(1567.98f, "G6"),
        FreqPair(1661.22f, "G+6"),
        FreqPair(1760.00f, "A6"),
        FreqPair(1864.66f, "A+6"),
        FreqPair(1975.53f, "B6"),
        FreqPair(2093.00f, "C7"),
        FreqPair(2217.46f, "C+7"),
        FreqPair(2349.32f, "D7"),
        FreqPair(2489.02f, "D+7"),
        FreqPair(2637.02f, "E7"),
        FreqPair(2793.83f, "F7"),
        FreqPair(2959.96f, "F+7"),
        FreqPair(3135.96f, "G7"),
        FreqPair(3322.44f, "G+7"),
        FreqPair(3520.00f, "A7"),
        FreqPair(3729.31f, "A+7"),
        FreqPair(3951.07f, "B7"),
        FreqPair(4186.01f, "C8"),
        FreqPair(4434.92f, "C+8"),
        FreqPair(4698.63f, "D8"),
        FreqPair(4978.03f, "D+8"),
        FreqPair(5274.04f, "E8"),
        FreqPair(5587.65f, "F8"),
        FreqPair(5919.91f, "F+8"),
        FreqPair(6271.93f, "G8"),
        FreqPair(6644.88f, "G+8"),
        FreqPair(7040.00f, "A8"),
        FreqPair(7458.62f, "A+8"),
        FreqPair(7902.13f, "B8"),

    });