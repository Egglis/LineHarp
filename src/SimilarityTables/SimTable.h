#pragma once
#include <vector>

#include <numeric>
#include "../CSV/Table.h"

enum SelectionModes {
	SINGLE,
	IMPORTANCE,
	MIDPOINT,
	MINDIST,
	HAUSDORFF,
	FRECHET
};

namespace lineweaver {

	class SimTable {
	public:
		float get(int focus, int current, float range); // row, col, range
		void setup(Table* table);

		void setActiveX(std::vector<float> x) { m_x = normalize(x); };
		void setActiveY(std::vector<float> y) { m_y = normalize(y); };
		void setActiveImportance(std::vector<float> importance) { m_importance = importance; };


		SelectionModes getMode() { return m_mode; };
		void setMode(int mode) { m_mode = static_cast<SelectionModes>(mode); };
		void setMode(SelectionModes mode) { m_mode = mode; };
	protected:
		void setupImportanceVector(Table* table);
		void iterateLines(Table* table);
		float computeEuclideanDistance(int a, int b);
		float computeMidPointDistance(int a, int b);

		float getFrechetDist(int i, int j, float d, std::vector<std::vector<float>> *matrix);

		std::vector<float> normalize(std::vector<float> list);


		std::vector<float> m_importanceTable;
		std::vector<std::vector<float>> m_midPointTable;
		std::vector<std::vector<float>> m_minDistTable;
		std::vector<std::vector<float>> m_hausdorffTable;
		std::vector<std::vector<float>> m_frechetTable;


		std::vector<float> m_x;
		std::vector<float> m_y;
		std::vector<float> m_importance;
		
		SelectionModes m_mode;
	};

};