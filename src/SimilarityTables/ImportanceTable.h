#pragma once
#include "SimTable.h"

namespace lineweaver {

	class ImportanceTable : public SimTable {
	public:
		float get(int focus, int current, float range);
		void setup(Table* table);
	private:
		std::vector<float> m_importanceTable;

	};


};