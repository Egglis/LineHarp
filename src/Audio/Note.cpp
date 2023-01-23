#include "Note.h"
#include <vector>

using namespace gam;

#include "FreqMap.h"


Note::Note(float freq, float amp) : m_freq{ freq }, m_amp{ amp }
{
    m_name = FreqMap::getNameFromFreq(freq);

}

void Note::angFreq(int ang)
{
    freq(FreqMap::mapAngleToNote(ang));
}


