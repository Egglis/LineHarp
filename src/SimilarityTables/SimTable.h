#pragma once
#include "Table.h"
#include <vector>

enum SelectionModes {
	SINGLE,
	IMPORTANCE,
	SIMILARITY,
	EUCLIDEAN
};

namespace lineweaver {

	class SimTable {
	public:
		SimTable(SelectionModes type) { m_type = type; };
		float get(int focus, int current, float range) = 0; // row, col, range
		void setup(Table* table) = 0;

		void setActiveX(std::vector<float> x) { m_x = x; };
		void setActiveY(std::vector<float> y) { m_y = y; };
		void setActiveImportance(std::vector<float> importance) { m_importance = importance; };
		SelectionModes getType() { return m_type; };
	protected:

		std::vector<std::vector<float>> m_table;

		std::vector<float> m_x;
		std::vector<float> m_y;
		std::vector<float> m_importance;
		
		SelectionModes m_type;
	};

};