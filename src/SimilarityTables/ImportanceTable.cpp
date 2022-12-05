#include "ImportanceTable.h"

using namespace lineweaver;

void ImportanceTable::setup(Table* table)
{
	m_importanceTable.clear();

	for (int i = 0; i < table->m_numberOfTrajectories; i++) {
		int count = static_cast<int>(table->m_numberOfTimesteps.at(i));
		float sum = 0.0f;
		for (int j = 0; j < table->m_numberOfTimesteps.at(i); j++) {
			sum += m_activeImportance.at((i * count) + j);
		}
		m_importanceTable.push_back(sum / static_cast<float>(count));
	}
}


float ImportanceTable::get(int focus, int current, float range)
{
	float selectedImportance = m_importanceTable.at(focus);
	float currentImportance = m_importanceTable.at(current);
	float diff = abs(selectedImportance - currentImportance);
	if (diff <= range) {
		return 1.0f - ((diff * 1.0f) / range);
	}
}